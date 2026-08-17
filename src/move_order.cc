// move_order.cc
#include "move_order.h"

namespace lightknight::search {
    // Returns the score of a single move.
    inline int ScoreMove(
        const Board& board,
        const Move& move,
        const TTEntry* tt_entry,
        const History& history
    ) {
        // TT Entry move first.
        if (tt_entry && move == tt_entry->move)
            return kTTMoveScore;

        const bool is_capture = board.IsCapture(move);

        // Queen promotions, sorted by captured piece.
        if (move.IsQueenPromotion()) {
            int score = kQueenPromotionBase;

            if (is_capture) {
                const int victim_idx = kPieceIdx[static_cast<int>(board.GetCapturedPiece(move))];
                score += victim_idx;
            }

            return score;
        }

        if (is_capture) {
            const int victim_idx = kPieceIdx[static_cast<int>(board.GetCapturedPiece(move))];
            const int attacker_idx = kPieceIdx[static_cast<int>(board.GetMovedPiece(move))];
            
            assert(victim_idx >= 0 && victim_idx < 5); // No king. 
            assert(attacker_idx >= 0 && attacker_idx < 6); 

            // Winning captures (king captures as well) sorted MVV-LVA.
            if (attacker_idx == 5 || attacker_idx < victim_idx) {
                return kWinningCaptureBase + MvvLvaScore[victim_idx][attacker_idx];
            }
            
            // Equal captures, sorted MVV-LVA.
            if (attacker_idx == victim_idx) {
                return kEqualCaptureBase + MvvLvaScore[victim_idx][attacker_idx];
            }

            // losing capture, sorted MVV-LVA.
            return kLosingCaptureBase + MvvLvaScore[victim_idx][attacker_idx];
        }

        // Underpromotions that are not captures, sorted by promotion piece type.
        if (move.IsUnderpromotion()) {
            const int prom_idx = static_cast<int>(move.GetPromotedPiece(Color::kWhite));
            return kUnderpromotionBase + prom_idx;
        }

        // Quiet move, ordered by history heuristic
        return kQuietMoveScore + history.Get(move, board.turn);
    }

    inline int ScoreQSMove(
        const Board& board,
        const Move& move,
        const TTEntry* tt_entry
    ) {
        // TT Entry move first.
        if (tt_entry && move == tt_entry->move)
            return kTTMoveScore;

        const bool is_capture = board.IsCapture(move);

        // Queen promotions, sorted by captured piece.
        if (move.IsQueenPromotion()) {
            int score = kQueenPromotionBase;

            if (is_capture) {
                const int victim_idx = kPieceIdx[static_cast<int>(board.GetCapturedPiece(move))];
                score += victim_idx;
            }

            return score;
        }

        // Captures
        if (is_capture) {
            const int victim_idx = kPieceIdx[static_cast<int>(board.GetCapturedPiece(move))];
            const int attacker_idx = kPieceIdx[static_cast<int>(board.GetMovedPiece(move))];
            
            assert(victim_idx >= 0 && victim_idx < 5); // No king. 
            assert(attacker_idx >= 0 && attacker_idx < 6); 

            // Winning captures (king captures as well) sorted MVV-LVA.
            if (attacker_idx == 5 || attacker_idx < victim_idx) {
                return kWinningCaptureBase + MvvLvaScore[victim_idx][attacker_idx];
            }
            
            // Equal captures, sorted MVV-LVA.
            if (attacker_idx == victim_idx) {
                return kEqualCaptureBase + MvvLvaScore[victim_idx][attacker_idx];
            }

            // losing capture, sorted MVV-LVA.
            return kLosingCaptureBase + MvvLvaScore[victim_idx][attacker_idx];
        }

        // Underpromotions that are not captures, sorted by promotion piece type.
        if (move.IsUnderpromotion()) {
            const int prom_idx = static_cast<int>(move.GetPromotedPiece(Color::kWhite));
            return kUnderpromotionBase + prom_idx;
        }

        // If there are quiet moves, no history here.
        return kQuietMoveScore;
    }

    // Compute the score of the moves.
    void ScoreMoves(
        const std::vector<Move>& moves,
        std::vector<int>& scores,
        size_t num_moves,
        const Board& board,
        const TTEntry* tt_entry,
        const History& history
    ) {
        for (size_t idx = 0; idx < num_moves; ++idx) {
            scores[idx] = ScoreMove(board, moves[idx], tt_entry, history);
        }
    }

    void ScoreQSMoves(
        const std::vector<Move>& moves,
        std::vector<int>& scores,
        size_t num_moves,
        const Board& board,
        const TTEntry* tt_entry
    ) {
        for (size_t idx = 0; idx < num_moves; ++idx) {
            scores[idx] = ScoreQSMove(board, moves[idx], tt_entry);
        }
    }

    // Returns a copy of the best move, swaps it in the vector to the last position and decrements
    // num_moves (effectively removing the move)
    Move PickMove(
        std::vector<Move>& moves,
        std::vector<int>& scores,
        size_t& num_moves
    ) {
        size_t best_idx = 0;

        for (size_t idx = 1; idx < num_moves; ++idx)
            if (scores[idx] > scores[best_idx])
                best_idx = idx;

        // Move the best move to last pos and decrement size (effectively removing it from
        // subsequent best move searches)
        --num_moves;
        
        std::swap(moves[best_idx], moves[num_moves]);
        std::swap(scores[best_idx], scores[num_moves]);

        return moves[num_moves];
    }
} // namespace lightknight::search
