// eval.cc
#include "eval.h"

#include <cstddef>
#include "board.h"

namespace lightknight::eval {    
    int ComputeGamePhase(const lightknight::Board& board) {
        int phase = 0;

        for (size_t piece = 0; piece < lightknight::kNumPieces - 1; ++piece)
            phase += kGamePhaseWeights[piece] * SetBitsCount(board.piece_bitboards[piece]);

        // Rescaled in the range [0, 1024].
        // Starting position -> 1024 and can only decrease
        phase = 1024 * phase / kTotalGamePhase; 
        return phase;
    }

    int ComputeWeightedEval(int phase, int mg_eval, int eg_eval) {
        return ((mg_eval * phase) + (eg_eval * (1024 - phase))) / 1024;
    }

    int EvaluateMaterial(const lightknight::Board& board, GamePhase game_phase) {
        int eval = 0;

        for (size_t piece = 0; piece < lightknight::kNumPieces - 1; ++piece) {
            eval += kPieceValues[game_phase][piece] * SetBitsCount(board.piece_bitboards[piece]);
        }

        return eval;
    }

    int EvaluateMaterial(const lightknight::Board& board, int phase_weight) {
        return ComputeWeightedEval(
            phase_weight,
            EvaluateMaterial(board, GamePhase::kMG),
            EvaluateMaterial(board, GamePhase::kEG)
        );
    }

    int EvaluatePieceSquare(const lightknight::Board& board, GamePhase game_phase) {
        int evaluation = 0;

        for (size_t piece = 0; piece < lightknight::kNumPieces - 1; ++piece) {
            const lightknight::Color color = (piece < 6) ? lightknight::Color::kWhite : lightknight::Color::kBlack;
            const size_t piece_idx = (color == lightknight::Color::kWhite) ? piece : piece - 6;

            uint64_t piece_bb = board.piece_bitboards[piece];

            while (piece_bb) {
                const uint64_t square_bb = LSB(piece_bb);
                const lightknight::Square square = BitboardToSquare(square_bb);
                const lightknight::Square table_square = (color == lightknight::Color::kWhite) ? square : MirrorVertically(square);

                const int square_value = kPieceSquareTables[game_phase][piece_idx][table_square];

                evaluation += color == lightknight::Color::kWhite ? square_value : -square_value;
                piece_bb &= ~square_bb;
            }
        }

        return evaluation;
    }

    int EvaluatePieceSquare(const lightknight::Board& board, int phase_weight) {
        return ComputeWeightedEval(
            phase_weight,
            EvaluatePieceSquare(board, GamePhase::kMG),
            EvaluatePieceSquare(board, GamePhase::kEG)
        );
    }

    int Evaluate(const lightknight::Board& board, GamePhase game_phase) {
        const int white_relative_score = 
            EvaluateMaterial(board, game_phase) + 
            EvaluatePieceSquare(board, game_phase);
        
        return board.turn == lightknight::Color::kWhite
            ? white_relative_score
            : -white_relative_score;
    }

    int Evaluate(const lightknight::Board& board) {
        const int phase_weight = ComputeGamePhase(board);
        const int white_relative_score = 
            EvaluateMaterial(board, phase_weight) + 
            EvaluatePieceSquare(board, phase_weight);
        
        return board.turn == lightknight::Color::kWhite
            ? white_relative_score
            : -white_relative_score;
    }
} // namespace lightknight::eval