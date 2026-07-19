// eval.cc
#include "eval.h"

#include <cstddef>
#include "board.h"

namespace lightknight::eval {
    int EvaluateMaterial(const lightknight::Board& board) {
        int eval = 0;

        for (size_t piece = 0; piece < lightknight::kNumPieces - 1; ++piece) {
            eval += kPieceValues[piece] * SetBitsCount(board.piece_bitboards[piece]);
        }

        return eval;
    }

    int EvaluatePieceSquare(const lightknight::Board& board) {
        int evaluation = 0;

        for (size_t piece = 0; piece < lightknight::kNumPieces - 1; ++piece) {
            const lightknight::Color color = (piece < 6) ? lightknight::Color::kWhite : lightknight::Color::kBlack;
            const size_t piece_idx = (color == lightknight::Color::kWhite) ? piece : piece - 6;

            uint64_t piece_bb = board.piece_bitboards[piece];

            while (piece_bb) {
                const uint64_t square_bb = LSB(piece_bb);
                const lightknight::Square square = BitboardToSquare(square_bb);
                const lightknight::Square table_square = (color == lightknight::Color::kWhite) ? square : MirrorVertically(square);

                const int square_value = kPieceSquareTables[piece_idx][table_square];

                evaluation += color == lightknight::Color::kWhite ? square_value : -square_value;
                piece_bb &= ~square_bb;
            }
        }

        return evaluation;
    }

    int Evaluate(const lightknight::Board& board) {
        const int white_relative_score = 
            EvaluateMaterial(board) + 
            EvaluatePieceSquare(board);
        
        return board.turn == lightknight::Color::kWhite
            ? white_relative_score
            : -white_relative_score;
    }
} // namespace lightknight::eval