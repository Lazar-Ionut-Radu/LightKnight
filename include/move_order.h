#ifndef LIGHTKNIGHT_MOVE_ORDER_H
#define LIGHTKNIGHT_MOVE_ORDER_H

#include <cstddef>
#include <vector>

#include "board.h"
#include "transposition_table.h"

namespace lightknight::search {
    constexpr int kTTMoveScore = 100'000;
    constexpr int kQueenPromotionBase = 400'000;
    constexpr int kWinningCaptureBase = 300'000;
    constexpr int kEqualCaptureBase = 200'000;
    constexpr int kUnderpromotionBase = 100'000;
    constexpr int kQuietMoveScore = 0;
    constexpr int kLosingCaptureBase = -100'000;

    struct History{
        int scores[kNumColors][kNumSquares][kNumSquares]{};

        inline void Update(const Move& move, Color color, int value) {
            this->scores[color][move.GetOriginSquare()][move.GetDestinationSquare()] += value;
        }
        inline int Get(const Move& move, Color color) const {
            return this->scores[color][move.GetOriginSquare()][move.GetDestinationSquare()];
        }
    };

    // Scores for capture moves according to MVV-LVA.
    // MvvLvaScore[Victim][Attacker]
    constexpr int MvvLvaScore[5][6] =
    {   //        P   N   B   R   Q   K
        /* P */ { 5,  4,  3,  2,  1,  0},
        /* N */ {11, 10,  9,  8,  7,  6},
        /* B */ {17, 16, 15, 14, 13, 12},
        /* R */ {23, 22, 21, 20, 19, 18},
        /* Q */ {29, 28, 27, 26, 25, 24},
    };
    //                                     P  N  B  R  Q  K  P  N  B  R  Q  K  Empty
    constexpr int kPieceIdx[kNumPieces] = {0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, -1};

    // Returns the score of a single move.
    inline int ScoreMove(
        const Board& board,
        const Move& move,
        const TTEntry* tt_entry,
        const History& history
    );

    // Score moves for Quiescence search.
    inline int ScoreQSMove(
        const Board& board,
        const Move& move,
        const TTEntry* tt_entry
    );

    // Compute the score of the moves.
    void ScoreMoves(
        const std::vector<Move>& moves,
        std::vector<int>& scores,
        size_t num_moves,
        const Board& board,
        const TTEntry* tt_entry,
        const History& history
    );

    // Compute the score of moves for Quiescence search.
    void ScoreQSMoves(
        const std::vector<Move>& moves,
        std::vector<int>& scores,
        size_t num_moves,
        const Board& board,
        const TTEntry* tt_entry
    );

    // Returns a copy of the best move, swaps it in the vector to the last position and decrements
    // num_moves (effectively removing the move)
    Move PickMove(
        std::vector<Move>& moves,
        std::vector<int>& scores,
        size_t& num_moves
    );
} // namespace lightknight::search

#endif // LIGHTKNIGHT_MOVE_ORDER_H