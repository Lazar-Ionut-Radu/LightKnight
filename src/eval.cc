// eval.cc
#include "eval.h"

#include <cstddef>
#include "board.h"
#include "move_gen.h"
#include "types.h"
#include "pawn_hash.h"

namespace lightknight::eval {
    template<typename T>
    std::pair<T, T> operator+(const std::pair<T, T>& a, const std::pair<T, T>& b)
    {
        return {a.first + b.first, a.second + b.second};
    }

    template<typename T>
    std::pair<T, T>& operator-=(std::pair<T, T>& a, const std::pair<T, T>& b)
    {
        a.first -= b.first;
        a.second -= b.second;
        return a;
    }

    template<typename T>
    std::pair<T, T> operator-(const std::pair<T, T>& a)
    {
        return {-a.first, -a.second};
    }

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
            attack_bb = kKnightAttacksBB[piece_sq];
        }
        else if constexpr (piece_type == Piece::kWhiteBishop || piece_type == Piece::kBlackBishop) {
            uint64_t blockers = board.color_bitboards[Color::kWhite] | board.color_bitboards[Color::kBlack];
            attack_bb = BishopAttackBB(piece_sq, blockers);
        }
        else if constexpr (piece_type == Piece::kWhiteRook || piece_type == Piece::kBlackRook) {
            uint64_t blockers = board.color_bitboards[Color::kWhite] | board.color_bitboards[Color::kBlack];
            attack_bb = RookAttackBB(piece_sq, blockers);
        }
        else if constexpr (piece_type == Piece::kWhiteQueen || piece_type == Piece::kBlackQueen) {
            uint64_t blockers = board.color_bitboards[Color::kWhite] | board.color_bitboards[Color::kBlack];
            attack_bb = QueenAttackBB(piece_sq, blockers);
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

    PawnHashEntry ComputePawnHashEntry(const Board& board) {
        PawnHashEntry pawn_hash_entry{};

        // Build the bitboards
        for (Color color : {Color::kWhite, Color::kBlack}) {
            uint64_t my_pawns_bb = (color == Color::kWhite) ? board.piece_bitboards[Piece::kWhitePawn] : board.piece_bitboards[Piece::kBlackPawn];
            uint64_t enemy_pawns_bb = (color == Color::kWhite) ? board.piece_bitboards[Piece::kBlackPawn] : board.piece_bitboards[Piece::kWhitePawn];
        
            // ---- Passed pawns. ----
            uint64_t passed_pawns_bb = 0ull;
            for (uint64_t bb = my_pawns_bb; bb; bb &= ~LSB(bb)) {
                uint64_t pawn_bb = LSB(bb);
                uint64_t test_bb = ForwardThreeFillBB(pawn_bb, color);

                if (!(enemy_pawns_bb & test_bb))
                    passed_pawns_bb |= pawn_bb;
            }

            uint64_t ep_pawn = passed_pawns_bb & Forward(board.en_passant, color);
            if (ep_pawn) {
                uint64_t potential_takers = West(ep_pawn) | East(ep_pawn);

                if (potential_takers & enemy_pawns_bb)
                    passed_pawns_bb &= ~ep_pawn;
            }
            pawn_hash_entry.passed_pawns_bb[color] = passed_pawns_bb;
        
            // ---- Isolated pawns. ----
            uint64_t isolated_pawns_bb = 0ull;
            for (uint64_t bb = my_pawns_bb; bb; bb &= ~LSB(bb)) {
                uint64_t pawn_bb = LSB(bb);
                uint64_t adjacent_file_bb = FileBB(East(pawn_bb)) | FileBB(West(pawn_bb));

                if (!(my_pawns_bb & adjacent_file_bb))
                    isolated_pawns_bb |= pawn_bb;
            }
            pawn_hash_entry.isolated_pawns_bb[color] = isolated_pawns_bb;

            // ---- Protected pawns. ----
            pawn_hash_entry.protected_pawns_bb[color] = my_pawns_bb & (East(Forward(my_pawns_bb, color)) | West(Forward(my_pawns_bb, color)));
        
            // ---- Connectes pawns. ----        
            pawn_hash_entry.connected_pawns_bb[color] = my_pawns_bb &
                (East(my_pawns_bb) | West(my_pawns_bb) |
                NorthEast(my_pawns_bb) | NorthWest(my_pawns_bb) |
                SouthEast(my_pawns_bb) | SouthWest(my_pawns_bb));
        }
        
        return pawn_hash_entry;
    } 

    std::pair<int, int> EvaluateMaterial(const Board& board, const parameters::EngineParameters& params) {
        std::pair<int, int> eval(0, 0);

        for (size_t piece = Piece::kWhitePawn; piece < Piece::kWhiteKing; ++piece) {
            int num_pieces = SetBitsCount(board.piece_bitboards[piece]);
            eval.first += params.eval.piece_values[0][piece] * num_pieces;
            eval.second += params.eval.piece_values[1][piece] * num_pieces;

        }

        for (size_t piece = Piece::kBlackPawn; piece < Piece::kBlackKing; ++piece) {
            int num_pieces = SetBitsCount(board.piece_bitboards[piece]);
            eval.first -= params.eval.piece_values[0][piece % 6] * num_pieces;
            eval.second -= params.eval.piece_values[1][piece % 6] * num_pieces;
        }

        return eval;
    }

    std::pair<int, int> EvaluatePieceSquare(const Board& board, const parameters::EngineParameters& params) {
        std::pair<int, int> eval(0, 0);

        for (size_t piece = 0; piece < lightknight::kNumPieces - 1; ++piece) {
            const lightknight::Color color = (piece < 6) ? lightknight::Color::kWhite : lightknight::Color::kBlack;
            const size_t piece_idx = (color == lightknight::Color::kWhite) ? piece : piece - 6;

            uint64_t piece_bb = board.piece_bitboards[piece];

            while (piece_bb) {
                const uint64_t square_bb = LSB(piece_bb);
                const lightknight::Square square = BitboardToSquare(square_bb);
                const lightknight::Square table_square = (color == lightknight::Color::kWhite) ? square : MirrorVertically(square);

                int square_value = params.eval.psqt[0][piece_idx][table_square];
                eval.first += color == lightknight::Color::kWhite ? square_value : -square_value;

                square_value = params.eval.psqt[1][piece_idx][table_square];
                eval.second += color == lightknight::Color::kWhite ? square_value : -square_value;

                piece_bb &= ~square_bb;
            }
        }

        return eval;
    }

    std::pair<int, int> EvaluateMobility(const Board& board, const parameters::EngineParameters& params) {
        std::pair<int, int> eval(0, 0);
        // Knights
        for (uint64_t piece_bb = board.piece_bitboards[Piece::kWhiteKnight]; piece_bb != 0; piece_bb &= ~LSB(piece_bb)) {
            int mobility = GetPieceMobility<Piece::kWhiteKnight>(board, LSBSquare(piece_bb));
            eval.first += params.eval.mobility[0][1][mobility];
            eval.second += params.eval.mobility[1][1][mobility];
        }
        for (uint64_t piece_bb = board.piece_bitboards[Piece::kBlackKnight]; piece_bb != 0; piece_bb &= ~LSB(piece_bb)) {
            int mobility = GetPieceMobility<Piece::kBlackKnight>(board, LSBSquare(piece_bb));
            eval.first -= params.eval.mobility[0][1][mobility];
            eval.second -= params.eval.mobility[1][1][mobility];
        }

        // Bishops
        for (uint64_t piece_bb = board.piece_bitboards[Piece::kWhiteBishop]; piece_bb != 0; piece_bb &= ~LSB(piece_bb)) {
            int mobility = GetPieceMobility<Piece::kWhiteBishop>(board, LSBSquare(piece_bb));
            eval.first += params.eval.mobility[0][2][mobility];
            eval.second += params.eval.mobility[1][2][mobility];
        }
        for (uint64_t piece_bb = board.piece_bitboards[Piece::kBlackBishop]; piece_bb != 0; piece_bb &= ~LSB(piece_bb)) {
            int mobility = GetPieceMobility<Piece::kBlackBishop>(board, LSBSquare(piece_bb));
            eval.first -= params.eval.mobility[0][2][mobility];
            eval.second -= params.eval.mobility[1][2][mobility];
        }

        // Rooks
        for (uint64_t piece_bb = board.piece_bitboards[Piece::kWhiteRook]; piece_bb != 0; piece_bb &= ~LSB(piece_bb)) {
            int mobility = GetPieceMobility<Piece::kWhiteRook>(board, LSBSquare(piece_bb));
            eval.first += params.eval.mobility[0][3][mobility];
            eval.second += params.eval.mobility[1][3][mobility];
        }
        for (uint64_t piece_bb = board.piece_bitboards[Piece::kBlackRook]; piece_bb != 0; piece_bb &= ~LSB(piece_bb)) {
            int mobility = GetPieceMobility<Piece::kBlackRook>(board, LSBSquare(piece_bb));
            eval.first -= params.eval.mobility[0][3][mobility];
            eval.second -= params.eval.mobility[1][3][mobility];
        }
        
        // Queens
        for (uint64_t piece_bb = board.piece_bitboards[Piece::kWhiteQueen]; piece_bb != 0; piece_bb &= ~LSB(piece_bb)) {
            int mobility = GetPieceMobility<Piece::kWhiteQueen>(board, LSBSquare(piece_bb));
            eval.first += params.eval.mobility[0][4][mobility];
            eval.second += params.eval.mobility[1][4][mobility];
        }
        for (uint64_t piece_bb = board.piece_bitboards[Piece::kBlackQueen]; piece_bb != 0; piece_bb &= ~LSB(piece_bb)) {
            int mobility = GetPieceMobility<Piece::kBlackQueen>(board, LSBSquare(piece_bb));
            eval.first -= params.eval.mobility[0][4][mobility];
            eval.second -= params.eval.mobility[1][4][mobility];
        }
    
        return eval;
    }

    std::pair<int, int> EvaluateSmallBonuses(const Board& board, const parameters::EngineParameters& params) {
        std::pair<int, int> eval(0, 0);
        
        // Tempo.
        eval.first += (board.turn == Color::kWhite) ? params.eval.tempo[0] : -params.eval.tempo[0];
        eval.second += (board.turn == Color::kWhite) ? params.eval.tempo[1] : -params.eval.tempo[1];

        // Bishop Pair.
        // Technically this is wrong because you may have only 2 bishops of the same color but
        // should not happen much, bishop promotions shouldn't happen much lol.
        if (SetBitsCount(board.piece_bitboards[Piece::kWhiteBishop]) > 1) {
            eval.first += params.eval.bishop_pair[0];
            eval.second += params.eval.bishop_pair[1];
        }
        if (SetBitsCount(board.piece_bitboards[Piece::kBlackBishop]) > 1) {
            eval.first -= params.eval.bishop_pair[0];
            eval.second -= params.eval.bishop_pair[1];
        }
        
        return eval; 
    }

    std::pair<int, int> EvaluatePawns(const Board& board, const parameters::EngineParameters& params, PawnHashEntry& pawn_hash_entry) {
        if (pawn_hash_entry.valid)
            return pawn_hash_entry.pawn_eval;
        
        std::pair<int, int> eval(0, 0);
        // Passed pawn bonus
        for (uint64_t bb = pawn_hash_entry.passed_pawns_bb[Color::kWhite]; bb; bb &= ~LSB(bb)) {
            const Square pawn_sq = LSBSquare(bb);
            eval.first += params.eval.passed_pawns[0][pawn_sq];
            eval.second += params.eval.passed_pawns[1][pawn_sq];
        }
        for (uint64_t bb = pawn_hash_entry.passed_pawns_bb[Color::kBlack]; bb; bb &= ~LSB(bb)) {
            const Square pawn_sq = LSBSquare(bb);
            eval.first -= params.eval.passed_pawns[0][MirrorVertically(pawn_sq)];
            eval.second -= params.eval.passed_pawns[1][MirrorVertically(pawn_sq)];
        }

        // Isolated pawn penalty
        for (uint64_t bb = pawn_hash_entry.isolated_pawns_bb[Color::kWhite]; bb; bb &= ~LSB(bb)) {
            const Square pawn_sq = LSBSquare(bb);
            eval.first += params.eval.isolated_pawns[0][pawn_sq];
            eval.second += params.eval.isolated_pawns[1][pawn_sq];
        }
        for (uint64_t bb = pawn_hash_entry.isolated_pawns_bb[Color::kBlack]; bb; bb &= ~LSB(bb)) {
            const Square pawn_sq = LSBSquare(bb);
            eval.first -= params.eval.isolated_pawns[0][MirrorVertically(pawn_sq)];
            eval.second -= params.eval.isolated_pawns[1][MirrorVertically(pawn_sq)];
        }
        
        // Doubled / Tripled pawns penalty.
        for (int pawn_type : {Piece::kWhitePawn, Piece::kBlackPawn}) {
            const uint64_t pawns = board.piece_bitboards[pawn_type];
            const int weight = (pawn_type == Piece::kWhitePawn) ? 1 : -1;

            for (int file = 0; file < 8; ++file) {
                int num_pawns = SetBitsCount(pawns & kFiles[file]);

                if (num_pawns == 2) {
                    eval.first += params.eval.doubled_pawns[0] * weight;
                    eval.second += params.eval.doubled_pawns[1] * weight;
                }
                else if (num_pawns > 2) {
                    eval.first += params.eval.tripled_pawns[0] * weight;
                    eval.second += params.eval.tripled_pawns[1] * weight;
                }
            }
        }

        // Connected pawns bonus.
        for (uint64_t bb = pawn_hash_entry.connected_pawns_bb[Color::kWhite]; bb; bb &= ~LSB(bb)) {
            const int pawn_rank = Rank(BitboardToSquare(LSB(bb)));
            eval.first += params.eval.connected_pawns[0][pawn_rank];
            eval.second += params.eval.connected_pawns[1][pawn_rank];
        }
        for (uint64_t bb = pawn_hash_entry.connected_pawns_bb[Color::kBlack]; bb; bb &= ~LSB(bb)) {
            const int pawn_rank = 7 - Rank(BitboardToSquare(LSB(bb)));
            eval.first -= params.eval.connected_pawns[0][pawn_rank];
            eval.second -= params.eval.connected_pawns[1][pawn_rank];
        }

        // Protected pawns bonus.
        for (uint64_t bb = pawn_hash_entry.protected_pawns_bb[Color::kWhite]; bb; bb &= ~LSB(bb)) {
            const int pawn_rank = Rank(BitboardToSquare(LSB(bb)));
            eval.first += params.eval.protected_pawns[0][pawn_rank];
            eval.second += params.eval.protected_pawns[1][pawn_rank];
        }
        for (uint64_t bb = pawn_hash_entry.protected_pawns_bb[Color::kBlack]; bb; bb &= ~LSB(bb)) {
            const int pawn_rank = 7 - Rank(BitboardToSquare(LSB(bb)));
            eval.first -= params.eval.protected_pawns[0][pawn_rank];
            eval.second -= params.eval.protected_pawns[1][pawn_rank];
        }

        pawn_hash_entry.pawn_eval = eval;
        return eval;
    }

    std::pair<int, int> EvaluateKings(const Board& board, const parameters::EngineParameters& params) {
        std::pair<int, int> eval(0, 0);
        
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
            int setbc1 = SetBitsCount(w_king_shield_close & w_pawns_bb), setbc2 = SetBitsCount(w_king_shield_far & w_pawns_bb);
            eval.first += setbc1 * params.eval.king_pawn_shield[0][0] * 3 / w_shield_cnt;
            eval.second += setbc1 * params.eval.king_pawn_shield[1][0] * 3 / w_shield_cnt;
            
            eval.first += setbc2 * params.eval.king_pawn_shield[0][1] * 3 / w_shield_cnt;
            eval.second += setbc2 * params.eval.king_pawn_shield[1][1] * 3 / w_shield_cnt;
        }
        if (b_shield_cnt) {
            int setbc1 = SetBitsCount(b_king_shield_close & b_pawns_bb), setbc2 = SetBitsCount(b_king_shield_far & b_pawns_bb);
            eval.first -= setbc1 * params.eval.king_pawn_shield[0][0] * 3 / b_shield_cnt;
            eval.second -= setbc1 * params.eval.king_pawn_shield[1][0] * 3 / b_shield_cnt;
            
            eval.first -= setbc2 * params.eval.king_pawn_shield[0][1] * 3 / b_shield_cnt;
            eval.second -= setbc2 * params.eval.king_pawn_shield[1][1] * 3 / b_shield_cnt;
        }
        
        return eval;
    }

    std::pair<int, int> Evaluate_(const Board& board, const parameters::EngineParameters& params, PawnHash& pawn_hash) {
        PawnHashEntry& pawn_hash_entry = pawn_hash[board.pawn_zobrist_hash];
        
        const std::pair<int, int> white_relative_score = 
            EvaluateMaterial(board, params) + 
            EvaluatePieceSquare(board, params) +
            EvaluateMobility(board, params) +
            EvaluateSmallBonuses(board, params) +
            EvaluatePawns(board, params, pawn_hash_entry) +
            EvaluateKings(board, params);
        
        return board.turn == Color::kWhite ? white_relative_score : -white_relative_score;
    }

    int Evaluate(const Board& board, const parameters::EngineParameters& params, PawnHash& pawn_hash) {
        const int phase_weight = ComputeGamePhase(board);
        const std::pair<int, int> eval = Evaluate_(board, params, pawn_hash);
        const int score = ComputeWeightedEval(phase_weight, eval.first, eval.second);

        return score;
    }
} // namespace lightknight::eval