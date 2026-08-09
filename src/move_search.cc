// move_search.cc
#include "move_search.h"

#include <iostream>
#include <chrono>
#include <algorithm>
#include <cstddef>
#include <vector>

#include "board.h"
#include "types.h"
#include "eval.h"
#include "movegen.h"
#include "transposition_table.h"
#include "move_order.h"

namespace lightknight::search {
    int ScoreToTT(int score, int ply) {
        if (score >= kMateScore - kMaxDepth)
            return score + ply;

        if (score <= -kMateScore + kMaxDepth)
            return score - ply;

        return score;
    }

    int ScoreFromTT(int score, int ply) {
        if (score >= kMateScore - kMaxDepth)
            return score - ply;

        if (score <= -kMateScore + kMaxDepth)
            return score + ply;

        return score;
    }

    bool ShouldStopSearch(TimeControlStruct& time_control) {
        // Clock was stopped before.
        if (time_control.stopped)
            return true;

        // Avoid doing the costly .now() call every time. Only do it once
        // lightknight::search::kCallsPerClockCheck elapsed.
        if (--time_control.calls_until_clock_check > 0)
            return false;

        // Reset clock check counter and check the clock.
        time_control.calls_until_clock_check = kCallsPerClockCheck;
        if (std::chrono::steady_clock::now() >= time_control.deadline) {
            time_control.stopped = true;
            return true;
        }

        return false;
    }

    std::vector<Move> ExtractPV(
        Board board,
        const TranspositionTable& tt
    ) {
        std::vector<Move> pv;
        pv.reserve(kMaxDepth);

        std::vector<Move> legal_moves;
        legal_moves.reserve(256);

        for (int ply = 0; ply < kMaxDepth; ++ply) {
            // See if the position is hashed.
            const TTEntry* entry = tt.Probe(board.zobrist_hash);
            if (entry == nullptr || entry->move.IsNull())
                break;

            // Check if the move is legal.
            legal_moves.clear();
            movegen::GenerateMoves<movegen::MoveGenType::kAll>(board, legal_moves);

            const auto itt = std::find(legal_moves.begin(), legal_moves.end(), entry->move);
            if (itt == legal_moves.end())
                break;

            // Check if it's draw by three fold.
            if (board.IsRepetition())
                break;

            // Add to pv.
            const Move move = entry->move;
            pv.push_back(move);

            // Make the move.
            UndoMoveInfo undo{};
            board.MakeMove(move, undo);
        }

        return pv;
    }

    SearchInfo BuildSearchInfo(
        const Board& board,
        const TranspositionTable& tt,
        const SearchStats& stats,
        int score
    ) {
        return SearchInfo{
            .nodes = stats.search_nodes + stats.q_nodes,
            .time_ms = stats.time_ms,
            .depth = stats.depth,
            .selective_depth = stats.selective_depth,
            .score = score,
            .pv = ExtractPV(board, tt)
        };
    }
    
    int IterativeDeepening(
        Board& board,
        int max_depth,
        const params::EngineParameters& params,
        TranspositionTable& tt, 
        TimeControlStruct& time_control,
        SearchStats& stats,
        const SearchInfoCallback& info_callback // Prints search info with UCI.
    ) {
        // Timing for stats.
        const auto start_time = std::chrono::steady_clock::now();

        // Clamp the depth so that it no bigger than the depth limit.
        const int target_max_depth = std::clamp(max_depth, 1, kMaxDepth);

        // Preallocate reusable vector for moves.
        std::vector<std::vector<lightknight::Move>> move_lists(kMaxDepth);
        for (std::vector<lightknight::Move>& moves : move_lists)
            moves.reserve(256);

        // Preallocate reusable vector for move scores.
        std::vector<std::vector<int>> score_lists(kMaxDepth);
        for (std::vector<int>& scores : score_lists)
            scores.reserve(256);
        
        // TT aging mechanism.
        // Increment the current generation of the tt entries.
        tt.IncrementGeneration();

        int result;
        for (int to_depth = 1; to_depth <= target_max_depth; ++to_depth) {
            stats.selective_depth = 0;

            const int score = PrincipalVariationSearch<search::SearchNodeType::kPVNode>(board, -kInfinity, kInfinity, to_depth, 0, move_lists, score_lists, params, tt, time_control, stats);
    
            if (time_control.stopped)
                break;

            result = score;

            // Stats.
            stats.depth = to_depth;
            stats.time_ms = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count());
        
            // Print stats to output UCI.
            if (info_callback) {
                const SearchInfo info = BuildSearchInfo(board, tt, stats, score);
                info_callback(info);
            }

            // Stop if a mate was found.
            if (IsMateScore(score))
                break;
        }

        return result;
    }

    template<SearchNodeType node_type>
    int PrincipalVariationSearch(
        Board& board,
        int alpha, // The best (highest) score this player can guarantee so far.
        int beta, // The best (lowest) score the opponent can guarantee so far.
        int max_depth, // Max search depth.
        int depth, // Current depth.
        std::vector<std::vector<Move>>& move_lists, // Preallocated vectors.
        std::vector<std::vector<int>>& score_lists, // Preallocated vectors for move scores.
        const params::EngineParameters& params,
        TranspositionTable& tt, // Memoization of positions.
        TimeControlStruct& time_control,
        SearchStats& stats
    ) {
        // Check if the allocated time elapsed.
        // Also increment the variable that counts after how many function calls to inspect the
        // clock.
        if (ShouldStopSearch(time_control))
            return 0;

        // Stats
        ++stats.search_nodes;
        stats.selective_depth = std::max(stats.selective_depth, depth);

        // Basecase: 
        // Draw by 3-fold repetition
        // Moreover, if this position is repeated (even only twice) within the search tree we
        // return a draw result to avoid searches of duplicate subtrees.
        // Check before TT probing as it is path-dependent. Don't save to TT for the same
        // reason.
        if (depth > 0 && board.IsRepetition(depth))
            return 0;
        
        // If we have reached the hard limit on depth that can be searched (unlikely) return
        // immediately with a provisional evaluation
        if (depth >= search::kMaxDepth || depth >= move_lists.size()) {
            return eval::Evaluate(board, params);
        }

        // Save for telling fail-low nodes (all nodes) from pv nodes later.
        const int original_alpha = alpha;

        // For TT probing and TT stores.
        const uint8_t remaining_depth = max_depth - depth;
        const uint64_t zobrist_hash = board.zobrist_hash; 
        TTEntry* tt_entry = nullptr;
        
        // Check the transposition table to see if the position is already evaluated.
        // If 50 move rule applies then this is a leaf node (either a checkmate, stalemate
        // or 50-move rule draw, which will be caught right after) -> Don't probe TT.
        const bool fifty_move_rule = board.halfmoves >= 100;

        if (!fifty_move_rule) {    
            tt_entry = tt.Probe(zobrist_hash);

            if (tt_entry) {
                // We consider only entries that are searched at a deeper depth.
                if (tt_entry->depth >= remaining_depth) {
                    const int tt_score = ScoreFromTT(tt_entry->score, depth);
                    
                    // A score whose value is exactly known -> return.
                    if (tt_entry->bound == TTBound::Exact) {
                        return tt_score;
                    }

                    // Beta cutoff, fail-high.
                    if (tt_entry->bound == TTBound::Lower && tt_score >= beta) {
                        return tt_score;
                    }

                    // Fail low.
                    if (tt_entry->bound == TTBound::Upper && tt_score <= alpha) {
                        return tt_score;
                    }
                }
            }
        }

        // Basecase:
        // Reaching the max depth of this search.
        if (depth >= max_depth) {
            return QuiescenceSearch<node_type>(board, alpha, beta, depth, move_lists, score_lists, params, tt, time_control, stats);
        }

        // Generate moves.
        // Use the move_lists to avoid expensive allocations at each function call.
        std::vector<Move>& moves = move_lists[depth];
        moves.clear();
        size_t num_moves = movegen::GenerateMoves<movegen::MoveGenType::kAll>(board, moves);
        
        // Basecase:
        // There are no legal moves in the position. Checkmate or Stalemate.
        const bool in_check = board.IsInCheck(board.turn);

        if (num_moves == 0) {
            const int score = in_check ? -search::kMateScore + depth : 0;
            return score;
        }
        
        // Basecase:
        // Draw by 50-move rule (while in check but not checkmated)
        if (fifty_move_rule)
            return 0;

        // Compute the move scores.
        std::vector<int>& scores = score_lists[depth];
        ScoreMoves(moves, scores, num_moves, board, tt_entry);

        // Keep track of the best move of the children nodes.
        int best_score = -search::kInfinity;
        Move best_move{};

        // First move is always searched with a full window.
        bool first_move = true;

        // Recursively seach the children nodes.
        while (num_moves > 0) {
            // Pick the best remaining move.
            const Move move = PickMove(moves, scores, num_moves);

            // Prepare the needed stuff for {Make | Unamake}Move()
            int score;
            UndoMoveInfo undo{};

            // The first move of a PVS is fully searched, with a full window.
            // Under the assumption of a good move ordering, the first move searched should be
            // one of the better moves, for which reason the subsequent moves will be initially
            // searched with a zero-window (alpha, alpha + 1), to test if they provide an
            // improvement. If they do, they are re-searched with a full-window.
            // Re-searches should not be numerous given a good move ordering, the benefits of the
            // faster zero-window searches will outweight the re-searches.
            board.MakeMove(move, undo);
            if (first_move) {
                // Full search
                score = -PrincipalVariationSearch<node_type>(board, -beta, -alpha, max_depth, depth + 1, move_lists, score_lists, params, tt, time_control, stats);
                first_move = false;
            } else {
                // Zero-window search.
                score = -PrincipalVariationSearch<SearchNodeType::kNonPVNode>(board, -alpha - 1, -alpha, max_depth, depth + 1, move_lists, score_lists, params, tt, time_control, stats);
                
                // If the move was proven to give an improvement, re-search it will a full window.
                // The node time constraint makes sure that we don't re-evaluate nodes that already
                // have a null-window (that can be further down a subtree)
                if (score > alpha && node_type == SearchNodeType::kPVNode)
                    score = -PrincipalVariationSearch<SearchNodeType::kPVNode>(board, -beta, -alpha, max_depth, depth + 1, move_lists, score_lists, params, tt, time_control, stats);
            }
            board.UnmakeMove(move, undo);
            
            // Check if the allocated time elapsed. Important to do after restoring the board and
            // before modifying the score, as a dummy score is returned here.
            if (time_control.stopped)
                return 0;

            // Update the best score
            if (score > best_score) {
                best_score = score;
                best_move = move;
            }

            // Update the alpha lower bound when finding the improvement.
            if (score > alpha) {
                alpha = score;
            }

            // Beta cutoff, fail-high.
            // We found a move that's "too good", the opponent will not take this branch, they have
            // a better move elsewhere in the tree. Return as to not waste time needlesly.
            // The returned value is a lower-bound for the nodes true value.
            if (alpha >= beta) {
                // Store the result in the transposition table.
                tt.Store(zobrist_hash, ScoreToTT(best_score, depth), remaining_depth, best_move, TTBound::Lower);
                
                return best_score; // Fail-soft.
            }
        }

        TTBound bound;
        // Fail-low. The move did not improve the position. It's result is an upper bound on its
        // true score because it has a cut node child, a node that failed high.
        if (best_score <= original_alpha) {
            bound = TTBound::Upper;
        } else {
            bound = TTBound::Exact;
        }
        
        // Store the result in the transposition table.
        tt.Store(zobrist_hash, ScoreToTT(best_score, depth), remaining_depth, best_move, bound);

        return best_score;
    }

    template<SearchNodeType node_type>
    int QuiescenceSearch(
        Board& board,
        int alpha, // The best (highest) score this player can guarantee so far.
        int beta, // The best (lowest) score the opponent can guarantee so far.
        int depth,
        std::vector<std::vector<Move>>& move_lists, // Preallocated vectors.
        std::vector<std::vector<int>>& score_lists, // Preallocated vectors for move scores.
        const params::EngineParameters& params,
        TranspositionTable& tt,
        TimeControlStruct& time_control,
        SearchStats& stats
    ) {
        // Check if the allocated time elapsed.
        // Also increment the variable that counts after how many function calls to inspect the
        // clock.
        if (ShouldStopSearch(time_control))
            return 0;

        // Stats.
        ++stats.q_nodes;
        stats.selective_depth = std::max(stats.selective_depth, depth);

        // Basecase:
        // Draw by 3-fold repetition
        // Moreover, if this position is repeated (even only twice) within the search tree we
        // return a draw result to avoid searches of duplicate subtrees.
        // Check before TT probing as it is path-dependent. Don't save to TT for the same
        // reason.
        if (depth > 0 && board.IsRepetition(depth))
            return 0;

        const bool in_check = board.IsInCheck(board.turn);
        const bool fifty_move_draw = board.halfmoves >= 100;

        // Basecase:
        // Draw by 50-move rule (while not in check)
        if (fifty_move_draw && !in_check)
            return 0;

        // If we have reached the hard limit on depth that can be searched (unlikely) return
        // immediately with a provisional evaluation
        if (depth >= search::kMaxDepth || depth >= move_lists.size()) {
            return eval::Evaluate(board, params);
        }

        // Save for telling fail-low nodes (all nodes) from pv nodes later.
        const int original_alpha = alpha;

        // For TT probing and TT stores.
        const uint64_t zobrist_hash = board.zobrist_hash;
        TTEntry* tt_entry = nullptr;

        // Check the transposition table to see if the position is already evaluated.
        // If 50 move rule applies then this is a leaf node (either a checkmate, stalemate
        // or 50-move rule draw, which will be caught right after) -> Don't probe TT.
        if (!fifty_move_draw) {
            tt_entry = tt.Probe(zobrist_hash);
            
            if (tt_entry) {
                const int tt_score = ScoreFromTT(tt_entry->score, depth);

                // We don't need to test the depth, nodes from Q search are considered depth 0 as far
                // as the transposition table is concerned.
                // Exact match, return
                if (tt_entry->bound == TTBound::Exact) {
                    return tt_score;
                }

                // Fail high, beta cutoff.
                if (tt_entry->bound == TTBound::Lower && tt_score >= beta) {
                    return tt_score;
                }

                // Fail low.
                if (tt_entry->bound == TTBound::Upper && tt_score <= alpha) {
                    return tt_score;
                }
            }
        }

        // Generate moves.
        std::vector<Move>& moves = move_lists[depth];
        moves.clear();
        size_t num_moves;

        // To decide what moves to generate, either all moves (to get out of check) or just
        // captures and promotions.
        if (in_check) {
            num_moves = movegen::GenerateMoves<movegen::MoveGenType::kAll>(board, moves);
        } else {
            num_moves = movegen::GenerateMoves<movegen::MoveGenType::kTactical>(board, moves);
        }

        // Basecase.
        // If there are no moves it's either checkmate, stalemate, or just a quiet position.
        if (num_moves == 0) {
            // Basecase: Checkmate
            if (in_check) {
                const int score = -kMateScore + depth;

                tt.Store(zobrist_hash, ScoreToTT(score, depth), 0, Move{}, TTBound::Exact);
                return score;
            }

            // Basecase: Stalemate
            moves.clear();
            if (movegen::GenerateMoves<movegen::MoveGenType::kQuiet>(board, moves) == 0) {
                tt.Store(zobrist_hash, 0, 0, Move{}, TTBound::Exact);
                return 0;
            }

            // Basecase: Quiet position.
            const int score = eval::Evaluate(board, params);

            tt.Store(zobrist_hash, score, 0, Move{}, TTBound::Exact);
            return score;
        }

        // Basecase: Draw by 50-move rule. (when in check)
        if (fifty_move_draw)
            return 0;

        // Keep track of the best move of the children nodes.
        int best_score = -search::kInfinity;
        Move best_move{};

        // Standing pat
        // Allow the search to stop if subsequent captures lead to worse positions.
        if (!in_check) {
            const int stand_pat = eval::Evaluate(board, params);
            best_score = stand_pat;

            // Return if doing nothing (no capture I mean) is better.
            // Beta cutoff, fail-high.
            if (stand_pat >= beta) {
                tt.Store(zobrist_hash, stand_pat, 0, Move{}, TTBound::Lower);
                return stand_pat;
            }

            if (stand_pat > alpha) {
                alpha = stand_pat;
            }
        } else {
            best_score = -search::kInfinity;
        }

        // The first move is always searched with a full window.
        bool first_move = true;

        // Move Ordering.
        // Compute the move scores.
        std::vector<int>& scores = score_lists[depth];
        ScoreMoves(moves, scores, num_moves, board, tt_entry);

        // Recursively search the children nodes.
        while (num_moves > 0) {
            // Pick the best remaining move.
            const Move move = PickMove(moves, scores, num_moves);

            // Prepare the needed stuff for {Make | Unamake}Move()
            int score;
            UndoMoveInfo undo{};

            // The first move of a PVS is fully searched, with a full window.
            // Under the assumption of a good move ordering, the first move searched should be
            // one of the better moves, for which reason the subsequent moves will be initially
            // searched with a zero-window (alpha, alpha + 1), to test if they provide an
            // improvement. If they do, they are re-searched with a full-window.
            // Re-searches should not be numerous given a good move ordering, the benefits of the
            // faster zero-window searches will outweight the re-searches.
            board.MakeMove(move, undo);
            if (first_move) {
                // Full search
                score = -QuiescenceSearch<node_type>(board, -beta, -alpha, depth + 1, move_lists, score_lists, params, tt, time_control, stats);
                first_move = false;
            } else {
                // Zero-window search.
                score = -QuiescenceSearch<SearchNodeType::kNonPVNode>(board, -alpha - 1, -alpha, depth + 1, move_lists, score_lists, params, tt, time_control, stats);
                
                // If the move was proven to give an improvement, re-search it will a full window.
                // The node time constraint makes sure that we don't re-evaluate nodes that already
                // have a null-window (that can be further down a subtree)
                if (score > alpha && node_type == SearchNodeType::kPVNode)
                    score = -QuiescenceSearch<SearchNodeType::kPVNode>(board, -beta, -alpha, depth + 1, move_lists, score_lists, params, tt, time_control, stats);
            }
            board.UnmakeMove(move, undo);

            // Check if the allocated time elapsed. Important to do after restoring the board and
            // before modifying the score, as a dummy score is returned here.
            if (time_control.stopped)
                return 0;

            // Update the best score
            if (score > best_score) {
                best_score = score;
                best_move = move;
            }

            // Update the alpha lower bound when finding the improvement.
            if (score > alpha) {
                alpha = score;
            }

            // Beta cutoff, fail-high.
            // We found a move that's "too good", the opponent will not take this branch, they have
            // a better move elsewhere in the tree. Return as to not waste time needlesly.
            // The returned value is a lower-bound for the nodes true value.
            if (alpha >= beta) {
                // Store the result in the transposition table.
                tt.Store(zobrist_hash, ScoreToTT(best_score, depth), 0, best_move, TTBound::Lower);
                
                return best_score; // Fail-soft.
            }
        }

        TTBound bound;
        // Fail-low. The move did not improve the position. It's result is an upper bound on its
        // true score because it has a cut node child, a node that failed high.
        if (best_score <= original_alpha) {
            bound = TTBound::Upper;
        } else {
            bound = TTBound::Exact;
        }
        
        // Store the result in the transposition table.
        tt.Store(zobrist_hash, ScoreToTT(best_score, depth), 0, best_move, bound);

        return best_score;
    }   
    

    bool IsMateScore(int score) {
        return std::abs(score) > (kMateScore - kMaxDepth);
    }

    int GetMateMoves(int score) {
        if (score > 0) {
            const int plies_to_mate = kMateScore - score;
            return (plies_to_mate + 1) / 2;
        }

        const int plies_to_mate = kMateScore + score;
        return -plies_to_mate / 2;
    }
    
    template int PrincipalVariationSearch<SearchNodeType::kPVNode>(
        Board&,
        int,
        int,
        int,
        int,
        std::vector<std::vector<Move>>&,
        std::vector<std::vector<int>>&,
        const params::EngineParameters&,
        TranspositionTable&,
        TimeControlStruct&,
        SearchStats&
    );

    template int PrincipalVariationSearch<SearchNodeType::kNonPVNode>(
        Board&,
        int,
        int,
        int,
        int,
        std::vector<std::vector<Move>>&,
        std::vector<std::vector<int>>&,
        const params::EngineParameters&,
        TranspositionTable&,
        TimeControlStruct&,
        SearchStats&
    );
} // namespace lightknight::search