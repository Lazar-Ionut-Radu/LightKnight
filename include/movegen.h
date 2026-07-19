#ifndef LIGHTKNIGHT_MOVEGEN_H
#define LIGHTKNIGHT_MOVEGEN_H

#include "types.h"
#include "board.h"
#include <array>
#include <vector>
#include <cstdint>
#include <cstddef>

namespace lightknight::movegen {   
    // For generating and making/unmaking castling moves.
    struct CastleInfo {
        uint64_t king_origin;
        uint64_t king_destination;
        uint64_t rook_origin;
        uint64_t rook_destination;
        uint64_t needed_empty; // Squares that need to be empty.
        uint64_t needed_safe; // Squares that need to be not attacked.
    };

    // To be indexed by enum Castle. I know, redundant, I don't care.
    static constexpr CastleInfo kCastleInfo[16] = {
        {}, // unused
        // White Queen Side Castle - 1
        {
            SquareToBitboard(Square::E1), SquareToBitboard(Square::C1), 
            SquareToBitboard(Square::A1), SquareToBitboard(Square::D1),
            SquareToBitboard(Square::B1) | SquareToBitboard(Square::C1) | SquareToBitboard(Square::D1),
            SquareToBitboard(Square::C1) | SquareToBitboard(Square::D1) | SquareToBitboard(Square::E1)
        },
        // White King Side Castle - 2
        {
            SquareToBitboard(Square::E1), SquareToBitboard(Square::G1), 
            SquareToBitboard(Square::H1), SquareToBitboard(Square::F1),
            SquareToBitboard(Square::F1) | SquareToBitboard(Square::G1),
            SquareToBitboard(Square::F1) | SquareToBitboard(Square::G1) | SquareToBitboard(Square::E1)
        }, 
        {}, // unused
        // BLack Queen Side Castle - 4
        {
            SquareToBitboard(Square::E8), SquareToBitboard(Square::C8), 
            SquareToBitboard(Square::A8), SquareToBitboard(Square::D8),
            SquareToBitboard(Square::B8) | SquareToBitboard(Square::C8) | SquareToBitboard(Square::D8),
            SquareToBitboard(Square::C8) | SquareToBitboard(Square::D8) | SquareToBitboard(Square::E8)
        }, 
        {}, {}, {}, // unused
        // King Side Castle - 8
        {
            SquareToBitboard(Square::E8), SquareToBitboard(Square::G8), 
            SquareToBitboard(Square::H8), SquareToBitboard(Square::F8),
            SquareToBitboard(Square::F8) | SquareToBitboard(Square::G8),
            SquareToBitboard(Square::F8) | SquareToBitboard(Square::G8) | SquareToBitboard(Square::E8)
        },
        {}, {}, {}, {}, {}, {}, {} // unused
    };

    // The shift and magics for precomputing sliding piece attacks. Find out more at:
    // https://www.chessprogramming.org/Magic_Bitboards
    inline constexpr int kBishopMagicShift = 64 - 9;  
    inline constexpr size_t kBishopAttacksArraySize = 64 * (1 << (64 - kBishopMagicShift)); // Number of magics needed for bishops 
    inline constexpr std::array<uint64_t, kNumSquares> kBishopMagics = {
        0x410c8148000812ULL, 0x404018850420021ULL, 0x211042208a008100ULL, 0x2c41004204001000ULL, 0x80a1120501a00002ULL, 0x8780a4048040020ULL, 0x840044c03620004ULL, 0x3001a200148ULL, 
        0x20441000400890ULL, 0x80200a4827000830ULL, 0xe800822020ULL, 0x81080050080000ULL, 0x181044560800200ULL, 0x400009040900418ULL, 0xc0041402000ULL, 0x9100108406808507ULL, 
        0x804802982300140ULL, 0x42001204291004ULL, 0x8108400512018ULL, 0x10800002a001400ULL, 0x12000082208001ULL, 0x101120200405046ULL, 0x4001004194d00ULL, 0xb000a0069204090ULL, 
        0xa84510005002418ULL, 0x511320010020800ULL, 0x1201c06010010190ULL, 0x286002028008020ULL, 0x4001001001004004ULL, 0x504040200202002ULL, 0x201003c04004030ULL, 0x4800304041040010ULL, 
        0x40123000842001ULL, 0x10a04100c05c00ULL, 0x2204001210040018ULL, 0x8010020080080080ULL, 0x440010010810040ULL, 0x8424080028003003ULL, 0x480011022094304aULL, 0x9201222104000400ULL, 
        0x102011002004004ULL, 0xc0080444b8021ULL, 0x480c49541003000ULL, 0x220003401000320ULL, 0x120115080211044ULL, 0x40012260802241ULL, 0x4020113400b00040ULL, 0x8040c8640210000cULL, 
        0x2028852c02010ULL, 0xa0044144400ULL, 0x400828050792000ULL, 0x10c0401019004404ULL, 0x8104080424502ULL, 0x2422001010008ULL, 0x920805150008ULL, 0x102008020259048ULL, 
        0x10300980ca0426ULL, 0x201041044022040ULL, 0x8002208c401008ULL, 0x8104100100a1104ULL, 0x4030404004060020ULL, 0x100201108618202ULL, 0x8090a08490010c0ULL, 0x440400258029ULL, 
    };

    // Rook attacks hashtable memory:
    inline constexpr int kRookMagicShift = 64 - 12; 
    inline constexpr size_t kRookAttacksArraySize = 64 * (1 << (64 - kRookMagicShift)); // Number of magics needed for rooks.
    inline constexpr std::array<uint64_t, kNumSquares> kRookMagics = {
        0x1480004000201080ULL, 0x10400028004050a4ULL, 0x1020004400080020ULL, 0x8200020024082070ULL, 0x4020010200440020ULL, 0x4500040082080100ULL, 0x81010004001000cULL, 0x500008100002042ULL, 
        0x420500880008050ULL, 0x801080010002004ULL, 0xa0040a0004800840ULL, 0x80100500222400ULL, 0x90810001082080ULL, 0x2000090824419ULL, 0x2480091d00200ULL, 0x4018008204c8001ULL, 
        0x280a80010011ULL, 0x4010004081208008ULL, 0x80400a8800040100ULL, 0x200c20010084c302ULL, 0x8e8012c100050800ULL, 0x82008042100c00ULL, 0x1001c00400a011ULL, 0x1000024000200080ULL, 
        0x80600300c008000ULL, 0x2082004081842ULL, 0x8040c810000e08ULL, 0x110021280048100ULL, 0x80084342a8000401ULL, 0x40002060400c9ULL, 0x8102958040400080ULL, 0x8c06152050090002ULL, 
        0x100400888800020ULL, 0x6582021081020ULL, 0x204084042a02000ULL, 0x40202022401c0052ULL, 0x20004081c020200ULL, 0x200140001800600ULL, 0x39002a8020054200ULL, 0x4000201440200584ULL, 
        0x9018003002009000ULL, 0x40140e100154000ULL, 0x100010040802a008ULL, 0x41085008a0098800ULL, 0x1048420080028100ULL, 0x4124100b0080121ULL, 0x2120020108140088ULL, 0x4400400452009ULL, 
        0x618020104640a0ULL, 0x8c840890028c04ULL, 0x214203011041024ULL, 0x1114410000900ULL, 0x281000224904910ULL, 0x1012800402010018ULL, 0x8028801411020ULL, 0xa0000880002ec008ULL, 
        0x2800028410011ULL, 0x1621008008694009ULL, 0x881004a4800dc202ULL, 0x822a004c002014eULL, 0x646006408102042ULL, 0x8020000e4081002ULL, 0x204004c800dULL, 0x210040025004082ULL, 
    };

    // Precomputes the relevant occupancy masks for computing rook and bishop attacks. Those are the squares along the
    // 4 direction each piece attacks, excluding the last square for each direction. 
    consteval std::array<uint64_t, kNumSquares> PrecomputeBishopRelevantOccupancy() {
        std::array<uint64_t, kNumSquares> bishop_masks = {0ULL};

        for (uint8_t i = 0; i < kNumSquares; i++) {
            int rank = Rank((Square)(i));
            int file = File((Square)(i));
            
            // NE
            for (int r = rank + 1, f = file + 1; r <= 6 && f <= 6; r++, f++)
                bishop_masks[i] |= SquareToBitboard(GetSquare(r, f));
            
            // NW
            for (int r = rank + 1, f = file - 1; r <= 6 && f >= 1; r++, f--)
                bishop_masks[i] |= SquareToBitboard(GetSquare(r, f));

            // SE
            for (int r = rank - 1, f = file + 1; r >= 1 && f <= 6; r--, f++)
                bishop_masks[i] |= SquareToBitboard(GetSquare(r, f));

            // SW
            for (int r = rank - 1, f = file - 1; r >= 1 && f >= 1; r--, f--)
                bishop_masks[i] |= SquareToBitboard(GetSquare(r, f));
        }
        return bishop_masks;
    }
    consteval std::array<uint64_t, kNumSquares> PrecomputeRookRelevantOccupancy() {
        std::array<uint64_t, kNumSquares> rook_masks = {0ULL};

        for (uint8_t i = 0; i < kNumSquares; i++) {
            int rank = Rank((Square)(i));
            int file = File((Square)(i));
            
            // Horizontal
            for (int f = file + 1; f <= 6; f++) rook_masks[i] |= SquareToBitboard(GetSquare(rank, f));
            for (int f = file - 1; f >= 1; f--) rook_masks[i] |= SquareToBitboard(GetSquare(rank, f));
            
            // Vertical
            for (int r = rank + 1; r <= 6; r++) rook_masks[i] |= SquareToBitboard(GetSquare(r, file));
            for (int r = rank - 1; r >= 1; r--) rook_masks[i] |= SquareToBitboard(GetSquare(r, file));
        }
        return rook_masks;
    }
    
    inline constexpr std::array<uint64_t, kNumSquares> kBishopRelevantOccupancy = PrecomputeBishopRelevantOccupancy();
    inline constexpr std::array<uint64_t, kNumSquares> kRookRelevantOccupancy = PrecomputeRookRelevantOccupancy();
        
    // Precompute pawn attacks, the 2 diagonal squares in front.
    consteval std::array<uint64_t, 2 * kNumSquares> PrecomputePawnAttacks() {
        std::array<uint64_t, 2 * kNumSquares> pawn_attacks = {0ULL};

        for (size_t sq = 0; sq < kNumSquares; sq++) {
            uint64_t sq_bb = SquareToBitboard((Square)sq);

            // White & black pawn attacks
            pawn_attacks[sq] = NorthWest(sq_bb) | NorthEast(sq_bb);
            pawn_attacks[kNumSquares + sq] = SouthWest(sq_bb) | SouthEast(sq_bb);
        }

        return pawn_attacks;
    }
    // Precomputed knight attacks, without taking into account the occupancy of the destination square.
    consteval std::array<uint64_t, kNumSquares> PrecomputeKnightAttacks() {
        std::array<uint64_t, kNumSquares> knight_attacks = {0ULL};

        for (uint8_t i = 0; i < kNumSquares; i++) {
            uint64_t bb = SquareToBitboard((Square)(i));

            uint64_t attacks = 0ULL;
            attacks |= North(NorthWest(bb));
            attacks |= North(NorthEast(bb));
            attacks |= East(NorthEast(bb));
            attacks |= East(SouthEast(bb));
            attacks |= South(SouthEast(bb));
            attacks |= South(SouthWest(bb));
            attacks |= West(SouthWest(bb));
            attacks |= West(NorthWest(bb));

            knight_attacks[i] = attacks;
        }

        return knight_attacks;
    }
    // Precompute king attacks, just the 8 (or less) positions surrounding the origin square.
    consteval std::array<uint64_t, kNumSquares> PrecomputeKingAttacks() {
        std::array<uint64_t, kNumSquares> king_attacks = {0ULL};
        
        for (uint8_t i = 0; i < kNumSquares; i++) {
            uint64_t bb = SquareToBitboard((Square)(i));

            uint64_t attacks = 0ULL;
            attacks |= North(bb);
            attacks |= NorthEast(bb);
            attacks |= East(bb);
            attacks |= SouthEast(bb);
            attacks |= South(bb);
            attacks |= SouthWest(bb);
            attacks |= West(bb);
            attacks |= NorthWest(bb);

            king_attacks[i] = attacks;
        }

        return king_attacks;
    }
    // Precompute bishop attacks using magic bitboards.
    consteval std::array<uint64_t, kBishopAttacksArraySize> PrecomputeBishopAttacks() {
        std::array<uint64_t, kBishopAttacksArraySize> bishop_attacks = {0ULL};
        
        for (size_t sq = 0; sq < kNumSquares; sq++) {
            // The diagonal lines centered on that sq.
            uint64_t relevant_occupancy = kBishopRelevantOccupancy[sq];

            // Generate all relevant blocker configurations.
            std::vector<int> bit_indices;
            bit_indices.clear();

            for (int sq = 0; sq < 64; sq++) {
                if (relevant_occupancy & (1ULL << sq)) {
                    bit_indices.push_back(sq);
                }
            }
            
            uint64_t num_blocker_bitboards = 1ULL << bit_indices.size();
            std::vector<uint64_t> blocker_bitboards;
            blocker_bitboards.clear();
            
            for (uint64_t i = 0; i < num_blocker_bitboards; i++) {
                uint64_t bitboard = 0ULL;
                for (size_t j = 0; j < bit_indices.size(); j++) {
                    if (i & (1ULL << j)) {
                        bitboard |= (1ULL << bit_indices[j]);
                    }
                }
                blocker_bitboards.push_back(bitboard);
            }

            // Calculate the attacks for each blocker configuration and store it.
            for (uint64_t blockers : blocker_bitboards) {
                uint64_t attacks = 0ULL;
                int rank = sq / 8;
                int file = sq % 8;

                // NE
                for (int r = rank + 1, f = file + 1; r <= 7 && f <= 7; r++, f++) {
                    uint64_t bb = SquareToBitboard(GetSquare(r, f));
                    attacks |= bb;
                    if (blockers & bb) break;
                }
                
                // NW
                for (int r = rank + 1, f = file - 1; r <= 7 && f >= 0; r++, f--) {
                    uint64_t bb = SquareToBitboard(GetSquare(r, f));
                    attacks |= bb;
                    if (blockers & bb) break;
                }
                // SE
                for (int r = rank - 1, f = file + 1; r >= 0 && f <= 7; r--, f++) {
                    uint64_t bb = SquareToBitboard(GetSquare(r, f));
                    attacks |= bb;
                    if (blockers & bb) break;
                }

                // SW
                for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; r--, f--) {
                    uint64_t bb = SquareToBitboard(GetSquare(r, f));
                    attacks |= bb;
                    if (blockers & bb) break;
                }
                
                // Find the place in the attackers array.
                uint64_t magics_square_hash = (blockers * kBishopMagics[sq]) >> kBishopMagicShift;
                uint64_t magics_hash = sq * (1ULL << (64 - kBishopMagicShift)) + magics_square_hash;
                bishop_attacks[magics_hash] = attacks;
            }
        }

        return bishop_attacks;
    }
    // Precompute rook attacks using magic bitboards.
    consteval std::array<uint64_t, kRookAttacksArraySize> PrecomputeRookAttacks() {
        std::array<uint64_t, kRookAttacksArraySize> rook_attacks = {0ULL};
        
        for (size_t sq = 0; sq < kNumSquares; sq++) {
            // The diagonal lines centered on that sq.
            uint64_t relevant_occupancy = kRookRelevantOccupancy[sq];

            // Generate all relevant blocker configurations.
            std::vector<int> bit_indices;
            bit_indices.clear();

            for (int sq = 0; sq < 64; sq++) {
                if (relevant_occupancy & (1ULL << sq)) {
                    bit_indices.push_back(sq);
                }
            }
            
            uint64_t num_blocker_bitboards = 1ULL << bit_indices.size();
            std::vector<uint64_t> blocker_bitboards;
            blocker_bitboards.clear();
            
            for (uint64_t i = 0; i < num_blocker_bitboards; i++) {
                uint64_t bitboard = 0ULL;
                for (size_t j = 0; j < bit_indices.size(); j++) {
                    if (i & (1ULL << j)) {
                        bitboard |= (1ULL << bit_indices[j]);
                    }
                }
                blocker_bitboards.push_back(bitboard);
            }

            // Calculate the attacks for each blocker configuration and store it.
            for (uint64_t blockers : blocker_bitboards) {
                uint64_t attacks = 0ULL;
                int rank = sq / 8;
                int file = sq % 8;

                // Horizontal
                for (int f = file + 1; f <= 7; f++) {
                    uint64_t bb = SquareToBitboard(GetSquare(rank, f));
                    attacks |= bb;
                    if (blockers & bb) break;
                }

                for (int f = file - 1; f >= 0; f--) {
                    uint64_t bb = SquareToBitboard(GetSquare(rank, f));
                    attacks |= bb;
                    if (blockers & bb) break;
                }
                
                // Vertical
                for (int r = rank + 1; r <= 7; r++) {
                    uint64_t bb = SquareToBitboard(GetSquare(r, file));
                    attacks |= bb;
                    if (blockers & bb) break;
                }
                for (int r = rank - 1; r >= 0; r--) {
                    uint64_t bb = SquareToBitboard(GetSquare(r, file));
                    attacks |= bb;
                    if (blockers & bb) break;
                }
                
                // Find the place in the attackers array.
                uint64_t magics_square_hash = (blockers * kRookMagics[sq]) >> kRookMagicShift;
                uint64_t magics_hash = sq * (1ULL << (64 - kRookMagicShift)) + magics_square_hash;
                rook_attacks[magics_hash] = attacks;
            }
        }

        return rook_attacks;
    }

    inline constexpr std::array<uint64_t, 2 * kNumSquares> kPawnAttacks = PrecomputePawnAttacks();
    inline constexpr std::array<uint64_t, kNumSquares> kKnightAttacks = PrecomputeKnightAttacks();
    inline constexpr std::array<uint64_t, kBishopAttacksArraySize> kBishopAttacks = PrecomputeBishopAttacks();
    inline constexpr std::array<uint64_t, kRookAttacksArraySize> kRookAttacks = PrecomputeRookAttacks();
    inline constexpr std::array<uint64_t, kNumSquares> kKingAttacks = PrecomputeKingAttacks();

    // Functions to get attacks.
    inline constexpr uint64_t PawnAttackBitboard(Square sq, Color color) {
        return kPawnAttacks[color * kNumSquares + sq];
    }
    inline constexpr uint64_t BishopAttackBitboard(Square sq, uint64_t blockers_bitboard) {
        uint64_t relevant_blockers = blockers_bitboard & kBishopRelevantOccupancy[sq];
        uint64_t magics_square_hash = (relevant_blockers * kBishopMagics[sq]) >> kBishopMagicShift;
        uint64_t magics_hash = sq * (1ULL << (64 - kBishopMagicShift)) + magics_square_hash;
        
        return kBishopAttacks[magics_hash];
    }
    inline constexpr uint64_t RookAttackBitboard(Square sq, uint64_t blockers_bitboard) {
        uint64_t relevant_blockers = blockers_bitboard & kRookRelevantOccupancy[sq];
        uint64_t magics_square_hash = (relevant_blockers * kRookMagics[sq]) >> kRookMagicShift;
        uint64_t magics_hash = sq * (1ULL << (64 - kRookMagicShift)) + magics_square_hash;
        
        return kRookAttacks[magics_hash];
    }
    inline constexpr uint64_t QueenAttackBitboard(Square sq, uint64_t blockers_bitboard) {
        return BishopAttackBitboard(sq, blockers_bitboard) | RookAttackBitboard(sq, blockers_bitboard);
    }

    enum class MoveGenType {
        kAll,
        kTactical,  // Moves that change the material balance i.e. captures & promotions.
        kCapture,
        kQuiet      // Moves that are not tactical.
    };

    // [TODO]: I'm gonna need to change vector at some point to something faster, for now it's gonna do.
    // Also, this is gon' be straight legal moves.
    
    template<MoveGenType Type>
    size_t GeneratePawnMoves(Board& board, std::vector<Move>& moves) {
        // Booleans for type of move generated
        constexpr bool generate_captures =
            Type == MoveGenType::kAll ||
            Type == MoveGenType::kTactical ||
            Type == MoveGenType::kCapture;

        constexpr bool generate_quiet_promotions =
            Type == MoveGenType::kAll ||
            Type == MoveGenType::kTactical;

        constexpr bool generate_quiet_moves =
            Type == MoveGenType::kAll ||
            Type == MoveGenType::kQuiet;

        // Useful values.
        const Color my_color = board.turn;
        const Color opposite_color =
            static_cast<Color>(1 - my_color);

        const uint64_t pawns = board.piece_bitboards[Piece::kWhitePawn + 6 * my_color];
        const uint64_t empty = board.piece_bitboards[Piece::kEmpty];
        const uint64_t enemy_pieces = board.color_bitboards[opposite_color];
        const uint64_t blockers = board.color_bitboards[0] | board.color_bitboards[1];
        const Square king_sq = BitboardToSquare(board.piece_bitboards[Piece::kWhiteKing + 6 * my_color]);
        const uint64_t enemy_rooks_queens =
            board.piece_bitboards[Piece::kWhiteRook + 6 * opposite_color] |
            board.piece_bitboards[Piece::kWhiteQueen + 6 * opposite_color];
        const uint64_t enemy_bishops_queens =
            board.piece_bitboards[Piece::kWhiteBishop + 6 * opposite_color] |
            board.piece_bitboards[Piece::kWhiteQueen + 6 * opposite_color];
        const uint64_t enemy_pawns = board.piece_bitboards[Piece::kWhitePawn + 6 * opposite_color];
        const uint64_t enemy_knights = board.piece_bitboards[Piece::kWhiteKnight + 6 * opposite_color];
            
        size_t new_moves_count = 0;

        // Helper funcs.
        const auto leaves_king_in_check =
            [&](uint64_t origin_bb,
                uint64_t destination_bb,
                uint64_t captured_bb = 0ULL) {
                const uint64_t relevant_blockers =
                    (blockers & ~origin_bb & ~captured_bb) |
                    destination_bb;

                const uint64_t straight_attacks =
                    RookAttackBitboard(
                        king_sq,
                        relevant_blockers
                    );

                const uint64_t diagonal_attacks =
                    BishopAttackBitboard(
                        king_sq,
                        relevant_blockers
                    );

                const bool straight_check =
                    straight_attacks &
                    (enemy_rooks_queens & ~captured_bb);

                const bool diagonal_check =
                    diagonal_attacks &
                    (enemy_bishops_queens & ~captured_bb);

                const bool pawn_check =
                    PawnAttackBitboard(king_sq, my_color) &
                    (enemy_pawns & ~captured_bb);

                const bool knight_check =
                    kKnightAttacks[king_sq] &
                    (enemy_knights & ~captured_bb);

                return straight_check ||
                    diagonal_check ||
                    pawn_check ||
                    knight_check;
            };

        const auto add_promotions =
            [&](Square origin, Square destination) {
                moves.push_back(
                    Move(
                        origin,
                        destination,
                        PromotionPieceType::kQueen,
                        MoveType::kPromotion
                    )
                );
                moves.push_back(
                    Move(
                        origin,
                        destination,
                        PromotionPieceType::kRook,
                        MoveType::kPromotion
                    )
                );
                moves.push_back(
                    Move(
                        origin,
                        destination,
                        PromotionPieceType::kBishop,
                        MoveType::kPromotion
                    )
                );
                moves.push_back(
                    Move(
                        origin,
                        destination,
                        PromotionPieceType::kKnight,
                        MoveType::kPromotion
                    )
                );

                new_moves_count += 4;
            };
        
        // Generate captures.
        if constexpr (generate_captures) {
            uint64_t remaining_pawns = pawns;

            // Normal captures.
            while (remaining_pawns) {
                const uint64_t pawn_bb = LSB(remaining_pawns);
                const Square origin_sq = BitboardToSquare(pawn_bb);

                uint64_t captures = PawnAttackBitboard(origin_sq, my_color) & enemy_pieces;

                while (captures) {
                    const uint64_t destination_bb = LSB(captures);
                    const Square destination_sq = BitboardToSquare(destination_bb);

                    if (!leaves_king_in_check(pawn_bb, destination_bb, destination_bb)) {
                        if (destination_bb & kRankPromotion[my_color]
                        ) {
                            add_promotions(origin_sq, destination_sq);
                        } else {
                            moves.push_back(Move(origin_sq, destination_sq));
                            ++new_moves_count;
                        }
                    }
                    captures &= ~destination_bb;
                }
                remaining_pawns &= ~pawn_bb;
            }

            // En passants.
            if (board.en_passant) {
                uint64_t en_passant_takers = pawns & 
                    (Backward(West(board.en_passant), my_color) | Backward(East(board.en_passant), my_color));

                while (en_passant_takers) {
                    const uint64_t origin_bb = LSB(en_passant_takers);
                    const uint64_t destination_bb = board.en_passant;
                    const uint64_t captured_bb = Backward(destination_bb, my_color);

                    if (!leaves_king_in_check(origin_bb, destination_bb, captured_bb)
                    ) {
                        moves.push_back(Move(BitboardToSquare(origin_bb), BitboardToSquare(destination_bb), PromotionPieceType::kKnight, MoveType::kEnPassant));
                        ++new_moves_count;
                    }

                    en_passant_takers &= ~origin_bb;
                }
            }
        }

        // Generate 1 square pawn pushes.
        if constexpr (generate_quiet_moves || generate_quiet_promotions) {
            uint64_t remaining_pawns = pawns;

            while (remaining_pawns) {
                const uint64_t pawn_bb = LSB(remaining_pawns);
                const uint64_t destination_bb = Forward(pawn_bb, my_color);

                if (destination_bb & empty) {
                    const bool is_promotion = destination_bb & kRankPromotion[my_color];

                    if (!leaves_king_in_check(pawn_bb, destination_bb)) {
                        if (is_promotion) {
                            if constexpr (generate_quiet_promotions) {
                                add_promotions(BitboardToSquare(pawn_bb), BitboardToSquare(destination_bb));
                            }
                        } else {
                            if constexpr (generate_quiet_moves) {
                                moves.push_back(Move(BitboardToSquare(pawn_bb), BitboardToSquare(destination_bb)));
                                ++new_moves_count;
                            }
                        }
                    }
                }

                remaining_pawns &= ~pawn_bb;
            }
        }

        // Generate double pushes.
        if constexpr (generate_quiet_moves) {
            uint64_t remaining_pawns = pawns;

            while (remaining_pawns) {
                const uint64_t pawn_bb = LSB(remaining_pawns);

                if (pawn_bb & kRankPawnDoublePush[my_color]) {
                    const uint64_t middle_bb = Forward(pawn_bb, my_color);
                    const uint64_t destination_bb = Forward(middle_bb, my_color);

                    if ((middle_bb & empty) && (destination_bb & empty) && !leaves_king_in_check(pawn_bb, destination_bb)) {
                        moves.push_back(Move(BitboardToSquare(pawn_bb), BitboardToSquare(destination_bb)));
                        ++new_moves_count;
                    }
                }

                remaining_pawns &= ~pawn_bb;
            }
        }

        return new_moves_count;
    }

    template <MoveGenType type>
    size_t GenerateKnightMoves(Board &board, std::vector<Move> &moves) {
        Color my_color = board.turn;
        Color opposite_color = (Color)(1 - my_color);

        // For generating candidate moves for a knight.
        uint64_t knights_bb = board.piece_bitboards[Piece::kWhiteKnight + 6 * my_color];
        uint64_t good_dests_bb = 0ULL;
        if constexpr (type == movegen::MoveGenType::kAll) {
            good_dests_bb = board.color_bitboards[opposite_color] | board.piece_bitboards[Piece::kEmpty];
        }
        else if constexpr (type == movegen::MoveGenType::kQuiet) {
            good_dests_bb = board.piece_bitboards[Piece::kEmpty];
        } 
        else if constexpr (
            type == movegen::MoveGenType::kTactical ||
            type == movegen::MoveGenType::kCapture
        ) {
            good_dests_bb = board.color_bitboards[opposite_color];
        }

        // For checking later if a move leaves you in check
        uint64_t blockers = (board.color_bitboards[0] | board.color_bitboards[1]);
        uint64_t king_bb = board.piece_bitboards[Piece::kWhiteKing + 6 * my_color];
        Square king_sq = BitboardToSquare(king_bb); 
        
        size_t new_moves_count = 0;

        // Itterate through all knight of that color.
        while (knights_bb) {
            uint64_t origin_bb = LSB(knights_bb);

            // Get the moves that don't capture one of your pieces.
            uint64_t attacks_bb = kKnightAttacks[BitboardToSquare(origin_bb)];
            attacks_bb &= good_dests_bb;

            // Itterate through them
            while (attacks_bb) {
                uint64_t dest_bb = LSB(attacks_bb);

                uint64_t relevant_blockers = (blockers | dest_bb) & ~origin_bb;
                uint64_t straights = RookAttackBitboard(king_sq, relevant_blockers) & ~dest_bb;
                uint64_t diagonals = BishopAttackBitboard(king_sq, relevant_blockers) & ~dest_bb;

                bool check_on_straights = straights & (board.piece_bitboards[Piece::kWhiteRook + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool check_on_diagonals = diagonals & (board.piece_bitboards[Piece::kWhiteBishop + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool pawn_check = (PawnAttackBitboard(BitboardToSquare(king_bb), my_color) & ~dest_bb) & board.piece_bitboards[Piece::kWhitePawn + 6*opposite_color];
                bool knight_check = (kKnightAttacks[BitboardToSquare(king_bb)] & ~dest_bb) & board.piece_bitboards[Piece::kWhiteKnight + 6*opposite_color];

                if (check_on_diagonals || check_on_straights || pawn_check || knight_check) {
                    attacks_bb &= ~dest_bb;
                    continue;
                }

                // Add to move list.
                moves.push_back(Move(BitboardToSquare(origin_bb), BitboardToSquare(dest_bb)));
                new_moves_count++;

                // Remove LSB
                attacks_bb &= ~dest_bb;
            }

            // Remove LSB
            knights_bb &= ~origin_bb;
        }

        return new_moves_count;
    }

    template<MoveGenType type>
    size_t GenerateBishopMoves(Board &board, std::vector<Move> &moves) {
        Color my_color = board.turn;
        Color opposite_color = (Color)(1 - my_color);

        // For generating candidate moves for a bishop
        uint64_t bishops_bb = board.piece_bitboards[Piece::kWhiteBishop + 6 * my_color];
        uint64_t good_dests_bb = 0ULL;
        if constexpr (type == movegen::MoveGenType::kAll) {
            good_dests_bb = board.color_bitboards[opposite_color] | board.piece_bitboards[Piece::kEmpty];
        }
        else if constexpr (type == movegen::MoveGenType::kQuiet) {
            good_dests_bb = board.piece_bitboards[Piece::kEmpty];
        } 
        else if constexpr (
            type == movegen::MoveGenType::kTactical ||
            type == movegen::MoveGenType::kCapture
        ) {
            good_dests_bb = board.color_bitboards[opposite_color];
        }

        // For checking later if a move leaves you in check.
        uint64_t king_bb = board.piece_bitboards[Piece::kWhiteKing + 6 * my_color];
        Square king_sq = BitboardToSquare(king_bb); 
        uint64_t blockers = (board.color_bitboards[0] | board.color_bitboards[1]);
                
        // Count number of new moves added.
        size_t new_moves_count = 0;

        // Itterate through all the bishops.
        while (bishops_bb) {
            uint64_t origin_bb = LSB(bishops_bb);

            // Get the possible destination squares for this move, that don't capture one of your pieces.
            uint64_t attacks_bb = BishopAttackBitboard(BitboardToSquare(origin_bb), blockers);
            attacks_bb &= good_dests_bb;

            // Itterate through them
            while (attacks_bb) {
                uint64_t dest_bb = LSB(attacks_bb);

                // Make sure moving this bishop does not leave your king in check.
                uint64_t relevant_blockers = (blockers | dest_bb) & ~origin_bb;
                uint64_t straights = RookAttackBitboard(king_sq, relevant_blockers) & ~dest_bb;
                uint64_t diagonals = BishopAttackBitboard(king_sq, relevant_blockers) & ~dest_bb;
                
                bool check_on_straights = straights & (board.piece_bitboards[Piece::kWhiteRook + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool check_on_diagonals = diagonals & (board.piece_bitboards[Piece::kWhiteBishop + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool pawn_check = (PawnAttackBitboard(BitboardToSquare(king_bb), my_color) & ~dest_bb) & board.piece_bitboards[Piece::kWhitePawn + 6*opposite_color];
                bool knight_check = (kKnightAttacks[BitboardToSquare(king_bb)] & ~dest_bb) & board.piece_bitboards[Piece::kWhiteKnight + 6*opposite_color];

                if (check_on_diagonals || check_on_straights || pawn_check || knight_check) {
                    attacks_bb &= ~dest_bb;
                    continue;
                }
                
                // Move is legal.
                moves.push_back(Move(BitboardToSquare(origin_bb), BitboardToSquare(dest_bb)));
                new_moves_count++;

                // Pop LSB
                attacks_bb &= ~dest_bb;
            }

            // Pop LSB
            bishops_bb &= ~origin_bb;
        }

        return new_moves_count;
    }

    template <MoveGenType type>
    size_t GenerateRookMoves(Board &board, std::vector<Move> &moves) {
        Color my_color = board.turn;
        Color opposite_color = (Color)(1 - my_color);

        // For generating candidate moves for a rook
        uint64_t rooks_bb = board.piece_bitboards[Piece::kWhiteRook + 6 * my_color];
        uint64_t good_dests_bb = 0ULL; 
        if constexpr (type == movegen::MoveGenType::kAll) {
            good_dests_bb = board.color_bitboards[opposite_color] | board.piece_bitboards[Piece::kEmpty];
        }
        else if constexpr (type == movegen::MoveGenType::kQuiet) {
            good_dests_bb = board.piece_bitboards[Piece::kEmpty];
        } 
        else if constexpr (
            type == movegen::MoveGenType::kTactical ||
            type == movegen::MoveGenType::kCapture
        ) {
            good_dests_bb = board.color_bitboards[opposite_color];
        }

        // For checking later if a move leaves you in check.
        uint64_t king_bb = board.piece_bitboards[Piece::kWhiteKing + 6 * my_color];
        Square king_sq = BitboardToSquare(king_bb); 
        uint64_t blockers = (board.color_bitboards[0] | board.color_bitboards[1]);
                
        // Count number of new moves added.
        size_t new_moves_count = 0;

        // Itterate through all the bishops.
        while (rooks_bb) {
            uint64_t origin_bb = LSB(rooks_bb);

            // Get the possible destination squares for this move, that don't capture one of your pieces.
            uint64_t attacks_bb = RookAttackBitboard(BitboardToSquare(origin_bb), blockers);
            attacks_bb &= good_dests_bb;

            // Itterate through them
            while (attacks_bb) {
                uint64_t dest_bb = LSB(attacks_bb);

                // Make sure moving this bishop does not leave your king in check.
                uint64_t relevant_blockers = (blockers | dest_bb) & ~origin_bb;
                uint64_t straights = RookAttackBitboard(king_sq, relevant_blockers) & ~dest_bb;
                uint64_t diagonals = BishopAttackBitboard(king_sq, relevant_blockers) & ~dest_bb;
                
                bool check_on_straights = straights & (board.piece_bitboards[Piece::kWhiteRook + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool check_on_diagonals = diagonals & (board.piece_bitboards[Piece::kWhiteBishop + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool pawn_check = (PawnAttackBitboard(BitboardToSquare(king_bb), my_color) & ~dest_bb) & board.piece_bitboards[Piece::kWhitePawn + 6*opposite_color];
                bool knight_check = (kKnightAttacks[BitboardToSquare(king_bb)] & ~dest_bb) & board.piece_bitboards[Piece::kWhiteKnight + 6*opposite_color];

                if (check_on_diagonals || check_on_straights || pawn_check || knight_check) {
                    attacks_bb &= ~dest_bb;
                    continue;
                }
                
                // Move is legal.
                moves.push_back(Move(BitboardToSquare(origin_bb), BitboardToSquare(dest_bb)));
                new_moves_count++;

                // Pop LSB
                attacks_bb &= ~dest_bb;
            }

            // Pop LSB
            rooks_bb &= ~origin_bb;
        }

        return new_moves_count;
    }

    template<MoveGenType type>
    size_t GenerateQueenMoves(Board &board, std::vector<Move> &moves) {
        Color my_color = board.turn;
        Color opposite_color = (Color)(1 - my_color);

        // For generating candidate moves for a queen.
        uint64_t queens_bb = board.piece_bitboards[Piece::kWhiteQueen + 6 * my_color];
        uint64_t good_dests_bb = 0ULL; 
        if constexpr (type == movegen::MoveGenType::kAll) {
            good_dests_bb = board.color_bitboards[opposite_color] | board.piece_bitboards[Piece::kEmpty];
        }
        else if constexpr (type == movegen::MoveGenType::kQuiet) {
            good_dests_bb = board.piece_bitboards[Piece::kEmpty];
        } 
        else if constexpr (
            type == movegen::MoveGenType::kTactical ||
            type == movegen::MoveGenType::kCapture
        ) {
            good_dests_bb = board.color_bitboards[opposite_color];
        }

        // For checking later if a move leaves you in check.
        Square king_sq = BitboardToSquare(board.piece_bitboards[Piece::kWhiteKing + 6 * my_color]); 
        uint64_t blockers = (board.color_bitboards[0] | board.color_bitboards[1]);
                
        // Count number of new moves added.
        size_t new_moves_count = 0;

        // Itterate through all the bishops.
        while (queens_bb) {
            uint64_t origin_bb = LSB(queens_bb);

            // Get the possible destination squares for this move, that don't capture one of your pieces.
            uint64_t attacks_bb = QueenAttackBitboard(BitboardToSquare(origin_bb), blockers);
            attacks_bb &= good_dests_bb;

            // Itterate through them
            while (attacks_bb) {
                uint64_t dest_bb = LSB(attacks_bb);

                // Make sure moving this bishop does not leave your king in check.
                uint64_t relevant_blockers = (blockers | dest_bb) & ~origin_bb;
                uint64_t straights = RookAttackBitboard(king_sq, relevant_blockers) & ~dest_bb;
                uint64_t diagonals = BishopAttackBitboard(king_sq, relevant_blockers) & ~dest_bb;
                
                bool check_on_straights = straights & (board.piece_bitboards[Piece::kWhiteRook + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool check_on_diagonals = diagonals & (board.piece_bitboards[Piece::kWhiteBishop + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
                bool pawn_check = (PawnAttackBitboard(king_sq, my_color) & ~dest_bb) & board.piece_bitboards[Piece::kWhitePawn + 6*opposite_color];
                bool knight_check = (kKnightAttacks[king_sq] & ~dest_bb) & board.piece_bitboards[Piece::kWhiteKnight + 6*opposite_color];

                if (check_on_diagonals || check_on_straights || knight_check || pawn_check) {
                    attacks_bb &= ~dest_bb;
                    continue;
                }
                
                // Move is legal.
                moves.push_back(Move(BitboardToSquare(origin_bb), BitboardToSquare(dest_bb)));
                new_moves_count++;

                // Pop LSB
                attacks_bb &= ~dest_bb;
            }

            // Pop LSB
            queens_bb &= ~origin_bb;
        }

        return new_moves_count;
    }

    template<MoveGenType type>
    size_t GenerateKingMoves(Board &board, std::vector<Move> &moves) {
        Color my_color = board.turn;
        Color opposite_color = (Color)(1 - my_color);
        Square king_sq = BitboardToSquare(board.piece_bitboards[Piece::kWhiteKing + 6 * my_color]);
        Square enemy_king_sq = BitboardToSquare(board.piece_bitboards[Piece::kWhiteKing + 6 * opposite_color]);
        uint64_t blockers = (board.color_bitboards[0] | board.color_bitboards[1]);
        
        // Get king destination squares that don't put the king near the enemy king or take its own pieces.
        uint64_t king_attacks_bb = kKingAttacks[king_sq] & ~kKingAttacks[enemy_king_sq];
        king_attacks_bb &= ~board.color_bitboards[my_color];

        // Count the numver of new moves.
        size_t new_moves_count = 0;

        // To generate captures only.
        if constexpr (
            type == movegen::MoveGenType::kTactical ||
            type == movegen::MoveGenType::kCapture
        ) {
            king_attacks_bb &= board.color_bitboards[opposite_color];
        }

        // To generate quiet moves only.
        if constexpr (
            type == movegen::MoveGenType::kQuiet
        ) {
            king_attacks_bb &= ~board.color_bitboards[opposite_color];
        }

        // Itterate through them.
        while (king_attacks_bb) {
            uint64_t dest_bb = LSB(king_attacks_bb);

            // Check if this move leaves the king in check.
            uint64_t relevant_blockers = (blockers & ~SquareToBitboard(king_sq)) & ~dest_bb;
            uint64_t straights = RookAttackBitboard(BitboardToSquare(dest_bb), relevant_blockers);
            uint64_t diagonals = BishopAttackBitboard(BitboardToSquare(dest_bb), relevant_blockers); 

            bool check_on_staights = straights & (board.piece_bitboards[Piece::kWhiteRook + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
            bool check_on_diagonals = diagonals & (board.piece_bitboards[Piece::kWhiteBishop + 6*opposite_color] | board.piece_bitboards[Piece::kWhiteQueen + 6*opposite_color]);
            bool pawn_check = PawnAttackBitboard(BitboardToSquare(dest_bb), my_color) & board.piece_bitboards[Piece::kWhitePawn + 6*opposite_color];
            bool knight_check = kKnightAttacks[BitboardToSquare(dest_bb)] & board.piece_bitboards[Piece::kWhiteKnight + 6*opposite_color];

            if (check_on_diagonals || check_on_staights || pawn_check || knight_check) {
                king_attacks_bb &= ~dest_bb;
                continue;
            }

            // Move is legal.
            moves.push_back(Move(BitboardToSquare(SquareToBitboard(king_sq)), BitboardToSquare(dest_bb)));
            new_moves_count++;

            // Pop LSB
            king_attacks_bb &= ~dest_bb;
        }

        // Castling
        uint8_t relevant_castles = board.castling & kCastlesByColor[my_color];
        
        // Castles are quiet moves.
        if constexpr (
            type == lightknight::movegen::MoveGenType::kAll ||
            type == lightknight::movegen::MoveGenType::kQuiet
        ) {
            while (relevant_castles) {
                uint8_t current_castle = LSB(relevant_castles);
                
                // Check the needed squares are empty.
                if (kCastleInfo[current_castle].needed_empty & (board.color_bitboards[0] | board.color_bitboards[1])) {
                    relevant_castles &= ~current_castle;
                    continue;
                }

                // Check the square that need to be safe are safe.
                uint64_t needed_safe = kCastleInfo[current_castle].needed_safe;
                bool safe = true;
                while (safe && needed_safe) {
                    uint64_t needed_safe_lsb = LSB(needed_safe);
                    
                    if (board.IsSquareAttacked(needed_safe_lsb, my_color)) {
                        safe = false;
                    }
                
                    // Pop LSB
                    needed_safe &= ~needed_safe_lsb;
                }
                if (!safe) {
                    relevant_castles &= ~current_castle;
                    continue;
                }

                // We trust that the board.castles flag ensures the king / rook have not been moved.
                // As such, this castle is legal.
                Square origin_sq = BitboardToSquare(kCastleInfo[current_castle].king_origin);
                Square dest_sq = BitboardToSquare(kCastleInfo[current_castle].king_destination);
                moves.push_back(Move(origin_sq, dest_sq, PromotionPieceType::kKnight, MoveType::kCastling));
                new_moves_count++;

                // Pop LSB
                relevant_castles &= ~current_castle;
            }    
        }
        return new_moves_count;
    }

    template<MoveGenType type>
    size_t GenerateMoves(Board &board, std::vector<Move> &moves) {
        size_t moves_count = 0;

        moves_count += GeneratePawnMoves<type>(board, moves);
        moves_count += GenerateKnightMoves<type>(board, moves);
        moves_count += GenerateBishopMoves<type>(board, moves);
        moves_count += GenerateRookMoves<type>(board, moves);
        moves_count += GenerateQueenMoves<type>(board, moves);
        moves_count += GenerateKingMoves<type>(board, moves);
        
        return moves_count;
    }
} // namespace lightknight::movegen

#endif // LIGHTKNIGHT_MOVEGEN_H