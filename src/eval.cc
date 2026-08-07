// eval.cc
#include "eval.h"

#include <cstddef>
#include "board.h"
#include "movegen.h"

namespace lightknight::eval {    
    int ComputeGamePhase(const Board& board) {
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

    template <Piece piece_type>
    int GetPieceMobility(const Board& board, Square piece_sq) {
        uint64_t friendly_bb;

        if constexpr (piece_type < kBlackPawn)
            friendly_bb = board.color_bitboards[Color::kWhite];
        else
            friendly_bb = board.color_bitboards[Color::kBlack];

        uint64_t attack_bb;
        
        if constexpr (piece_type == Piece::kWhiteKnight || piece_type == Piece::kBlackKnight) {
            attack_bb = movegen::kKnightAttacks[piece_sq];
        }
        else if constexpr (piece_type == Piece::kWhiteBishop || piece_type == Piece::kBlackBishop) {
            uint64_t blockers = board.color_bitboards[Color::kWhite] | board.color_bitboards[Color::kBlack];
            attack_bb = movegen::BishopAttackBitboard(piece_sq, blockers);
        }
        else if constexpr (piece_type == Piece::kWhiteRook || piece_type == Piece::kBlackRook) {
            uint64_t blockers = board.color_bitboards[Color::kWhite] | board.color_bitboards[Color::kBlack];
            attack_bb = movegen::RookAttackBitboard(piece_sq, blockers);
        }
        else if constexpr (piece_type == Piece::kWhiteQueen || piece_type == Piece::kBlackQueen) {
            uint64_t blockers = board.color_bitboards[Color::kWhite] | board.color_bitboards[Color::kBlack];
            attack_bb = movegen::QueenAttackBitboard(piece_sq, blockers);
        }
        
        attack_bb &= ~friendly_bb;
        return SetBitsCount(attack_bb);
    }

    // Explicitly instantiate template funcs.
    template int GetPieceMobility<Piece::kWhiteKnight>(const Board&, Square);
    template int GetPieceMobility<Piece::kBlackKnight>(const Board&, Square);
    template int GetPieceMobility<Piece::kWhiteBishop>(const Board&, Square);
    template int GetPieceMobility<Piece::kBlackBishop>(const Board&, Square);
    template int GetPieceMobility<Piece::kWhiteRook>(const Board&, Square);
    template int GetPieceMobility<Piece::kBlackRook>(const Board&, Square);
    template int GetPieceMobility<Piece::kWhiteQueen>(const Board&, Square);
    template int GetPieceMobility<Piece::kBlackQueen>(const Board&, Square);

    int EvaluateMaterial(const Board& board, GamePhase game_phase) {
        int eval = 0;

        for (size_t piece = 0; piece < lightknight::kNumPieces - 1; ++piece) {
            eval += kPieceValues[game_phase][piece] * SetBitsCount(board.piece_bitboards[piece]);
        }

        return eval;
    }

    int EvaluatePieceSquare(const Board& board, GamePhase game_phase) {
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

    int EvaluateMobility(const Board& board, GamePhase game_phase) {
        int evaluation = 0;
        
        // Knights
        for (uint64_t piece_bb = board.piece_bitboards[Piece::kWhiteKnight]; piece_bb != 0; piece_bb &= ~LSB(piece_bb))
            evaluation += kMobilityBonuses[game_phase][1][GetPieceMobility<Piece::kWhiteKnight>(board, LSBSquare(piece_bb))];
        for (uint64_t piece_bb = board.piece_bitboards[Piece::kBlackKnight]; piece_bb != 0; piece_bb &= ~LSB(piece_bb))
            evaluation -= kMobilityBonuses[game_phase][1][GetPieceMobility<Piece::kBlackKnight>(board, LSBSquare(piece_bb))];
        
        // Bishops
        for (uint64_t piece_bb = board.piece_bitboards[Piece::kWhiteBishop]; piece_bb != 0; piece_bb &= ~LSB(piece_bb))
            evaluation += kMobilityBonuses[game_phase][2][GetPieceMobility<Piece::kWhiteBishop>(board, LSBSquare(piece_bb))];
        for (uint64_t piece_bb = board.piece_bitboards[Piece::kBlackBishop]; piece_bb != 0; piece_bb &= ~LSB(piece_bb))
            evaluation -= kMobilityBonuses[game_phase][2][GetPieceMobility<Piece::kBlackBishop>(board, LSBSquare(piece_bb))];
        
        // Rooks
        for (uint64_t piece_bb = board.piece_bitboards[Piece::kWhiteRook]; piece_bb != 0; piece_bb &= ~LSB(piece_bb))
            evaluation += kMobilityBonuses[game_phase][3][GetPieceMobility<Piece::kWhiteRook>(board, LSBSquare(piece_bb))];
        for (uint64_t piece_bb = board.piece_bitboards[Piece::kBlackRook]; piece_bb != 0; piece_bb &= ~LSB(piece_bb))
            evaluation -= kMobilityBonuses[game_phase][3][GetPieceMobility<Piece::kBlackRook>(board, LSBSquare(piece_bb))];
        
        // Queens
        for (uint64_t piece_bb = board.piece_bitboards[Piece::kWhiteQueen]; piece_bb != 0; piece_bb &= ~LSB(piece_bb))
            evaluation += kMobilityBonuses[game_phase][4][GetPieceMobility<Piece::kWhiteQueen>(board, LSBSquare(piece_bb))];
        for (uint64_t piece_bb = board.piece_bitboards[Piece::kBlackQueen]; piece_bb != 0; piece_bb &= ~LSB(piece_bb))
            evaluation -= kMobilityBonuses[game_phase][4][GetPieceMobility<Piece::kBlackQueen>(board, LSBSquare(piece_bb))];
    
        return evaluation;
    }

    int EvaluateSmallBonuses(const Board& board, GamePhase game_phase) {
        int evaluation = 0;
        
        // Tempo.
        evaluation += board.turn == Color::kWhite ? kTempoBonus[game_phase] : -kTempoBonus[game_phase];

        // Bishop Pair.
        // Technically this is wrong because you may have only 2 bishops of the same color but
        // should not happen much, bishop promotions shouldn't happen much lol.
        if (SetBitsCount(board.piece_bitboards[Piece::kWhiteBishop]) > 1)
            evaluation += kBishopPairBonus[game_phase];
        if (SetBitsCount(board.piece_bitboards[Piece::kBlackBishop]) > 1)
            evaluation -= kBishopPairBonus[game_phase];
        
        return evaluation; 
    }

    int EvaluateMaterial(const Board& board, int phase_weight) {
        return ComputeWeightedEval(
            phase_weight,
            EvaluateMaterial(board, GamePhase::kMG),
            EvaluateMaterial(board, GamePhase::kEG)
        );
    }

    int EvaluatePieceSquare(const Board& board, int phase_weight) {
        return ComputeWeightedEval(
            phase_weight,
            EvaluatePieceSquare(board, GamePhase::kMG),
            EvaluatePieceSquare(board, GamePhase::kEG)
        );
    }

    int EvaluateMobility(const Board& board, int phase_weight) {
        return ComputeWeightedEval(
            phase_weight,
            EvaluateMobility(board, GamePhase::kMG),
            EvaluateMobility(board, GamePhase::kEG)
        );
    }

    int EvaluateSmallBonuses(const Board& board, int phase_weight) {
        return ComputeWeightedEval(
            phase_weight,
            EvaluateSmallBonuses(board, GamePhase::kMG),
            EvaluateSmallBonuses(board, GamePhase::kEG)
        );
    };

    int Evaluate(const Board& board, GamePhase game_phase) {
        const int white_relative_score = 
            EvaluateMaterial(board, game_phase) + 
            EvaluatePieceSquare(board, game_phase) +
            EvaluateMobility(board, game_phase) +
            EvaluateSmallBonuses(board, game_phase);
        
        return board.turn == Color::kWhite
            ? white_relative_score
            : -white_relative_score;
    }

    int Evaluate(const Board& board) {
        const int phase_weight = ComputeGamePhase(board);
        const int white_relative_score = 
            EvaluateMaterial(board, phase_weight) + 
            EvaluatePieceSquare(board, phase_weight) +
            EvaluateMobility(board, phase_weight) +
            EvaluateSmallBonuses(board, phase_weight);
        
        return board.turn == Color::kWhite
            ? white_relative_score
            : -white_relative_score;
    }
} // namespace lightknight::eval