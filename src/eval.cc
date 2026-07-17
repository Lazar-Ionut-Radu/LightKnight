// eval.cc
#include "eval.h"

#include <cstddef>
#include "board.h"

namespace lightknight::eval {
    int EvaluateMaterial(lightknight::Board& board) {
        int eval = 0;

        for (size_t piece = 0; piece < lightknight::kNumPieces - 1; ++piece) {
            eval += kPieceValues[piece] * SetBitsCount(board.piece_bitboards[piece]);
        }

        return eval;
    }

    int Evaluate(lightknight::Board& board) {
        const int white_relative_score = EvaluateMaterial(board);
        
        return board.turn == lightknight::Color::kWhite
            ? white_relative_score
            : -white_relative_score;
    }
} // namespace lightknight::eval