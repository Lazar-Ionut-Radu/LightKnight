#ifndef LIGHTKNIGHT_MOVE_ORDER_H
#define LIGHTKNIGHT_MOVE_ORDER_H

#include <cstddef>
#include <vector>

#include "board.h"
#include "transposition_table.h"
#include "move_search.h"

namespace lightknight::search {
    constexpr int kTTMoveScore = 1'000'000;
    constexpr int kQueenPromotionBase = 500'000;
    constexpr int kWinningCaptureBase = 400'000;
    constexpr int kEqualCaptureBase = 300'000;
    constexpr int kUnderpromotionBase = 200'000;
    constexpr int kKillerMoveBase = 100'000;
    constexpr int kQuietMoveBase = 0;
    constexpr int kLosingCaptureBase = -100'000;

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
        const History& history,
        const Killers& killers,
        const int depth
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
        const History& history,
        const Killers& Killers,
        const int depth
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