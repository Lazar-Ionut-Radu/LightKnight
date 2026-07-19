// search.cc
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

namespace lightknight::search {
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

    // Alpha-Beta pruning algorithm, written in a negamax framework.
    // Alpha = proven score lower-bound: This node has a move scoring at least alpha
    // Beta = Cutoff inherited from the parent. A move scoring more than beta will not be reached
    //        because the opponent's best move is on another branch. Node scores > beta are not
    //        exact, because work is cut short when encountered.
    // Keeps track of the principal variation PV.
    template<bool collect_stats>
    int AlphaBeta(
        lightknight::Board& board,
        std::vector<std::vector<lightknight::Move>>& move_lists, // Preallocated, clear is faster than allocating mem.
        int alpha,
        int beta,
        int max_depth, 
        int depth,
        PrincipalVariation& pv,
        TimeControlStruct& time_control,
        SearchStats& stats
    ) {
        if (ShouldStopSearch(time_control))
            return 0;

        // Protect access to move_lists (should not happen but who knows)
        // If depth limit was reached return the evaluation.
        if (depth > kMaxDepth || depth > move_lists.size()) {
            if constexpr (collect_stats)
                ++stats.leaf_nodes;

            return lightknight::eval::Evaluate(board);
        }
        
        // No pv known yet from this node.
        pv.length[depth] = 0;
        
        // Generate moves
        std::vector<lightknight::Move>& moves = move_lists[depth];
        moves.clear();
        const size_t move_count = lightknight::movegen::GenerateMoves<lightknight::movegen::MoveGenType::kAll>(board, moves);

        // Base case, no moves in position. Either checkmate or stalemate.
        if (move_count == 0) {
            if constexpr (collect_stats)
                ++stats.leaf_nodes;
            
            return board.IsInCheck(board.turn)
                ? -lightknight::search::kMateScore + depth // Mate found.
                : 0;                                       // Stalemate.
        }

        // Base case, max depth reached.
        if (depth >= max_depth || depth >= lightknight::search::kMaxDepth - 1) {
            if constexpr (collect_stats)
                ++stats.leaf_nodes;
            
            return lightknight::eval::Evaluate(board);
        }

        // This node will have at least one child.
        if constexpr (collect_stats)
            ++stats.inner_nodes;

        // Search recursively.
        int best_score = -lightknight::search::kInfinity;
        for (std::size_t idx = 0; idx < move_count; ++idx) {
            // Prepare for make/unmake move.
            const lightknight::Move move = moves[idx];
            lightknight::UndoMoveInfo undo{};

            // Recurse
            board.MakeMove(move, undo);
            int score = -AlphaBeta<collect_stats>(
                board,
                move_lists, 
                -beta, 
                -alpha, 
                max_depth, 
                depth + 1, 
                pv,
                time_control,
                stats
            );
            board.UnmakeMove(move, undo);
            
            // Check if the child node stopped. Important to do after restoring the board.
            // A dummy score is returned, one that does not results (results that should 
            // be discarded regardless)
            if (time_control.stopped)
                return 0; 

            // Update best score found.
            best_score = std::max(best_score, score);

            // Found an improvement on that lower bound best score.
            if (score > alpha) {
                alpha = score;

                // Move is now the first move of the best line from the current node.
                pv.table[depth][0] = move;
                
                // Copy best continuation found by child node.
                for (size_t i = 0; i < pv.length[depth + 1]; ++i) {
                    pv.table[depth][i + 1] = pv.table[depth + 1][i];
                }
                pv.length[depth] = pv.length[depth + 1] + 1;
            }
            
            // Cutoff, fail-high.
            // Alpha is a lower bound on this node's true value. We have proven that this branch
            // will not be taken, so its futile to compute the true value.
            // Returning best_score makes this fail-soft.
            if (alpha >= beta) {
                if constexpr (collect_stats)
                    ++stats.beta_cutoffs;

                return best_score;
            }
        }

        // If the result is inside the original (alpha, beta) window, then this is an exact score.
        // If every result stayed below the original alpha then this is a fail-low and 
        // best_score is only an upper bound on the real score.
        return best_score;
    }

    template<bool collect_stats>
    SearchResult IterativeDeepening(
        Board& board,
        int max_depth,
        int time_limit_ms
    ) {
        // Clamp the depth so that it no bigger than the depth limit.
        const int target_max_depth = std::clamp(max_depth, 1, kMaxDepth);
    
        // Avoid invalid negative durations.
        time_limit_ms = std::max(time_limit_ms, 0);

        // Time control struct.
        TimeControlStruct time_control{
            .deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(time_limit_ms),
            .calls_until_clock_check = 1,
            .stopped = false
        };

        // Reusable vector for lists (so that its allocated only once)
        std::vector<std::vector<lightknight::Move>> move_lists(kMaxDepth);
        for (std::vector<lightknight::Move>& moves : move_lists)
            moves.reserve(256);
        
        // I should add a fallback if no AlphaBeta could be finished.
        SearchResult result{};
        
        // The succesive searches
        for (int to_depth = 1; to_depth <= target_max_depth; ++to_depth) {
            PrincipalVariation pv{};
            SearchStats stats{};
            
            // Declaration for stats. 
            std::chrono::steady_clock::time_point iter_start, iter_end;

            if constexpr (collect_stats)
                iter_start = std::chrono::steady_clock::now();
            
            const int score = AlphaBeta<collect_stats>(
                board,
                move_lists,
                -kInfinity,
                kInfinity,
                to_depth,
                0,
                pv,
                time_control,
                stats
            );
            
            if constexpr (collect_stats)
                iter_end = std::chrono::steady_clock::now();
            
            // If time elapsed, don't save the invalid result and don't start other searches.
            if (time_control.stopped) {
                std::cout << "Search to depth " << to_depth << " was interrupted.\n";
                break;
            }
            
            // Update the result with the newest search.
            result.evaluation = score;
            result.pv_length = pv.length[0];
            for (int i = 0; i < result.pv_length; ++i)
                result.pv[i] = pv.table[0][i];

            if constexpr (collect_stats)
                stats.elapsed_ms = std::chrono::duration<double, std::milli>(iter_end-iter_start).count();
                result.stats = stats;
        }
    
        return result;
    }

    // Explicitly generate both versions in this translation unit.
    template SearchResult IterativeDeepening<true>(
        lightknight::Board&,
        int,
        int
    );

    template SearchResult IterativeDeepening<false>(
        lightknight::Board&,
        int,
        int
    );
} // namespace lightknight::search