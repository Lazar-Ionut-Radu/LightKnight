// eval.cc
#include "eval.h"

#include <cstddef>
#include "board.h"
#include "movegen.h"
#include "types.h"

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

    template int GetPieceMobility<Piece::kWhiteKnight>(const Board&, Square);
    template int GetPieceMobility<Piece::kBlackKnight>(const Board&, Square);
    template int GetPieceMobility<Piece::kWhiteBishop>(const Board&, Square);
    template int GetPieceMobility<Piece::kBlackBishop>(const Board&, Square);
    template int GetPieceMobility<Piece::kWhiteRook>(const Board&, Square);
    template int GetPieceMobility<Piece::kBlackRook>(const Board&, Square);
    template int GetPieceMobility<Piece::kWhiteQueen>(const Board&, Square);
    template int GetPieceMobility<Piece::kBlackQueen>(const Board&, Square);

    uint64_t PassedPawnsBB(const Board& board, Color color) { 
        uint64_t my_pawns_bb = (color == Color::kWhite) ? board.piece_bitboards[Piece::kWhitePawn] : board.piece_bitboards[Piece::kBlackPawn];
        uint64_t enemy_pawns_bb = (color == Color::kWhite) ? board.piece_bitboards[Piece::kBlackPawn] : board.piece_bitboards[Piece::kWhitePawn];
        uint64_t passed_pawns_bb = 0ull;

        for (uint64_t bb = my_pawns_bb; bb; bb &= ~LSB(bb)) {
            uint64_t pawn_bb = LSB(bb);
            uint64_t test_bb = ForwardThreeFillBB(pawn_bb, color);

            if (!(enemy_pawns_bb & test_bb))
                passed_pawns_bb |= pawn_bb;
        }

        // If a pawn can be taken en passant its not a passed pawn.
        uint64_t ep_pawn = passed_pawns_bb & Forward(board.en_passant, color);
        if (ep_pawn) {
            uint64_t potential_takers = West(ep_pawn) | East(ep_pawn);

            if (potential_takers & enemy_pawns_bb)
                passed_pawns_bb &= ~ep_pawn;
        } 

        return passed_pawns_bb;
    }

    uint64_t IsolatedPawnsBB(const Board& board, Color color) {
        uint64_t my_pawns_bb = color ? board.piece_bitboards[Piece::kBlackPawn] : board.piece_bitboards[Piece::kWhitePawn];
        uint64_t isolated_pawns_bb = 0ull;

        for (uint64_t bb = my_pawns_bb; bb; bb &= ~LSB(bb)) {
            uint64_t pawn_bb = LSB(bb);
            uint64_t adjacent_file_bb = FileBB(East(pawn_bb)) | FileBB(West(pawn_bb));

            if (!(my_pawns_bb & adjacent_file_bb))
                isolated_pawns_bb |= pawn_bb;
        }

        return isolated_pawns_bb;
    }

    uint64_t ProtectedPawnsBB(const Board& board, Color color) {
        const uint64_t pawns_bb = (color == Color::kWhite)
            ? board.piece_bitboards[kWhitePawn] 
            : board.piece_bitboards[kBlackPawn];

        return pawns_bb & (East(Forward(pawns_bb, color)) | West(Forward(pawns_bb, color)));
    }

    uint64_t ConnectedPawnsBB(const Board& board, Color color) {
        const uint64_t pawns_bb = (color == Color::kWhite)
            ? board.piece_bitboards[kWhitePawn]
            : board.piece_bitboards[kBlackPawn];

        return pawns_bb &
            (East(pawns_bb) | West(pawns_bb) |
            NorthEast(pawns_bb) | NorthWest(pawns_bb) |
            SouthEast(pawns_bb) | SouthWest(pawns_bb));
    }

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
        evaluation += (board.turn == Color::kWhite) ? kTempoBonus[game_phase] : -kTempoBonus[game_phase];

        // Bishop Pair.
        // Technically this is wrong because you may have only 2 bishops of the same color but
        // should not happen much, bishop promotions shouldn't happen much lol.
        if (SetBitsCount(board.piece_bitboards[Piece::kWhiteBishop]) > 1)
            evaluation += kBishopPairBonus[game_phase];
        if (SetBitsCount(board.piece_bitboards[Piece::kBlackBishop]) > 1)
            evaluation -= kBishopPairBonus[game_phase];
        
        return evaluation; 
    }

    int EvaluatePawns(const Board& board, GamePhase game_phase) {
        int evaluation = 0;

        // Passed pawn bonus        
        const uint64_t white_passed_pawns_bb = PassedPawnsBB(board, Color::kWhite);  
        const uint64_t black_passed_pawns_bb = PassedPawnsBB(board, Color::kBlack);

        for (uint64_t bb = white_passed_pawns_bb; bb; bb &= ~LSB(bb)) {
            const Square pawn_sq = LSBSquare(bb);
            evaluation += kPassedPawnBonuses[game_phase][pawn_sq];
        }
        for (uint64_t bb = black_passed_pawns_bb; bb; bb &= ~LSB(bb)) {
            const Square pawn_sq = LSBSquare(bb);
            evaluation -= kPassedPawnBonuses[game_phase][MirrorVertically(pawn_sq)];
        }

        // Isolated pawn penalty
        const uint64_t white_isolated_pawns_bb = IsolatedPawnsBB(board, Color::kWhite);
        const uint64_t black_isolated_pawns_bb = IsolatedPawnsBB(board, Color::kBlack);
    
        for (uint64_t bb = white_isolated_pawns_bb; bb; bb &= ~LSB(bb)) {
            const Square pawn_sq = LSBSquare(bb);
            evaluation += kIsolatedPawnPenalties[game_phase][pawn_sq];
        }
        for (uint64_t bb = black_isolated_pawns_bb; bb; bb &= ~LSB(bb)) {
            const Square pawn_sq = LSBSquare(bb);
            evaluation -= kIsolatedPawnPenalties[game_phase][MirrorVertically(pawn_sq)];
        }
        
        // Doubled / Tripled pawns penalty.
        for (int pawn_type : {Piece::kWhitePawn, Piece::kBlackPawn}) {
            const uint64_t pawns = board.piece_bitboards[pawn_type];
            const int weight = (pawn_type == Piece::kWhitePawn) ? 1 : -1;

            for (int file = 0; file < 8; ++file) {
                int num_pawns = SetBitsCount(pawns & kFiles[file]);

                if (num_pawns == 2) {
                    evaluation += kDoubledPawnsPenalty[game_phase] * weight;
                }
                else if (num_pawns > 2) {
                    evaluation += kTripledPawnsPenalty[game_phase] * weight;
                }
            }
        }

        // Connected pawns bonus.
        uint64_t white_connected_pawns_bb = ConnectedPawnsBB(board, Color::kWhite);
        uint64_t black_connected_pawns_bb = ConnectedPawnsBB(board, Color::kBlack);

        for (uint64_t bb = white_connected_pawns_bb; bb; bb &= ~LSB(bb)) {
            const int pawn_rank = Rank(BitboardToSquare(LSB(bb)));
            evaluation += kConnectedPawnsBonuses[game_phase][pawn_rank];
        }
        for (uint64_t bb = black_connected_pawns_bb; bb; bb &= ~LSB(bb)) {
            const int pawn_rank = 7 - Rank(BitboardToSquare(LSB(bb)));
            evaluation -= kConnectedPawnsBonuses[game_phase][pawn_rank];
        }

        // Protected pawns bonus.
        uint64_t white_protected_pawns_bb = ProtectedPawnsBB(board, Color::kWhite);
        uint64_t black_protected_pawns_bb = ProtectedPawnsBB(board, Color::kBlack);

        for (uint64_t bb = white_protected_pawns_bb; bb; bb &= ~LSB(bb)) {
            const int pawn_rank = Rank(BitboardToSquare(LSB(bb)));
            evaluation += kProtectedPawnsBonuses[game_phase][pawn_rank];
        }
        for (uint64_t bb = black_protected_pawns_bb; bb; bb &= ~LSB(bb)) {
            const int pawn_rank = 7 - Rank(BitboardToSquare(LSB(bb)));
            evaluation -= kProtectedPawnsBonuses[game_phase][pawn_rank];
        }

        return evaluation;
    }

    int EvaluateKings(const Board& board, GamePhase game_phase) {
        int evaluation = 0;

        // King pawn shield
        const uint64_t w_king_bb = board.piece_bitboards[Piece::kWhiteKing];
        const uint64_t w_pawns_bb = board.piece_bitboards[Piece::kWhitePawn];
        const uint64_t w_king_shield_close = Forward(w_king_bb, kWhite) 
            & East(Forward(w_king_bb, kWhite)) 
            & West(Forward(w_king_bb, kWhite));
        const uint64_t w_king_shield_far = Forward(w_king_shield_close, kWhite);
        const int w_shield_cnt = SetBitsCount(w_king_shield_close);

        const uint64_t b_king_bb = board.piece_bitboards[Piece::kBlackKing];
        const uint64_t b_pawns_bb = board.piece_bitboards[Piece::kBlackPawn];
        const uint64_t b_king_shield_close = Forward(b_king_bb, kBlack) 
            & East(Forward(b_king_bb, kBlack)) 
            & West(Forward(b_king_bb, kBlack));
        const uint64_t b_king_shield_far = Forward(b_king_shield_close, kBlack);
        const int b_shield_cnt = SetBitsCount(b_king_shield_close);
        
        if (w_shield_cnt) {
            evaluation += SetBitsCount(w_king_shield_close & w_pawns_bb) * kKingPawnShieldBonuses[game_phase][0] * 3 / w_shield_cnt;
            evaluation += SetBitsCount(w_king_shield_far & w_pawns_bb) * kKingPawnShieldBonuses[game_phase][1] * 3 / w_shield_cnt;
        }
        if (b_shield_cnt) {
            evaluation -= SetBitsCount(b_king_shield_close & b_pawns_bb) * kKingPawnShieldBonuses[game_phase][0] * 3 / b_shield_cnt;
            evaluation -= SetBitsCount(b_king_shield_far & b_pawns_bb) * kKingPawnShieldBonuses[game_phase][1] * 3 / b_shield_cnt;
        }
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

    int EvaluatePawns(const Board& board, int phase_weight) {
        return ComputeWeightedEval(
            phase_weight,
            EvaluatePawns(board, GamePhase::kMG),
            EvaluatePawns(board, GamePhase::kEG)
        );
    }

    int EvaluateKings(const Board& board, int phase_weight) {
        return ComputeWeightedEval(
            phase_weight,
            EvaluateKings(board, GamePhase::kMG),
            EvaluateKings(board, GamePhase::kEG)
        );
    }


    int Evaluate(const Board& board, GamePhase game_phase) {
        const int white_relative_score = 
            EvaluateMaterial(board, game_phase) + 
            EvaluatePieceSquare(board, game_phase) +
            EvaluateMobility(board, game_phase) +
            EvaluateSmallBonuses(board, game_phase) +
            EvaluatePawns(board, game_phase) +
            EvaluateKings(board, game_phase);
        
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
            EvaluateSmallBonuses(board, phase_weight) +
            EvaluatePawns(board, phase_weight) +
            EvaluateKings(board, phase_weight);

        return board.turn == Color::kWhite
            ? white_relative_score
            : -white_relative_score;
    }
} // namespace lightknight::eval