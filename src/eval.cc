// eval.cc
#include "eval.h"

#include <cstddef>
#include "board.h"

namespace lightknight::eval {
    int EvaluateMaterial(lightknight::Board& board) {
        uint64_t eval = 0;

        for (size_t piece = 0; piece < lightknight::kNumPieces - 1; ++piece) {
            eval += kPieceValues[piece] * SetBitsCount(board.piece_bitboards[piece]);
        }

        return eval;
    }

    int Evaluate(lightknight::Board& board) {
        return EvaluateMaterial(board);    
    }
} // namespace lightknight::eval