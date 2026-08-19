#ifndef LIGHTKNIGHT_TYPES_H
#define LIGHTKNIGHT_TYPES_H

#include <cstdint>
#include <cstddef>
#include <array>
#include <iostream>
#include <tuple>
#include <cassert>
#include <vector>

namespace lightknight {
    // -----------------------------------------------------------------------------------------
    // ---------------------------------------- SQUARES ----------------------------------------
    // -----------------------------------------------------------------------------------------

    inline constexpr size_t kNumSquares = 64;
    enum Square : uint8_t {
        A1, B1, C1, D1, E1, F1, G1, H1,
        A2, B2, C2, D2, E2, F2, G2, H2,
        A3, B3, C3, D3, E3, F3, G3, H3,
        A4, B4, C4, D4, E4, F4, G4, H4,
        A5, B5, C5, D5, E5, F5, G5, H5,
        A6, B6, C6, D6, E6, F6, G6, H6,
        A7, B7, C7, D7, E7, F7, G7, H7,
        A8, B8, C8, D8, E8, F8, G8, H8,
    };
    std::ostream& operator<<(std::ostream& os, Square sq);
    
    constexpr int Rank(Square square) {return square / 8;}
    constexpr int File(Square square) {return square % 8;}
    constexpr Square GetSquare(int rank, int file) {return (Square)(8*rank + file);}

    // For log2 of power of 2.
    constexpr uint64_t kDeBruijeMagic = 0x03F79D71B4CB0A89ULL;
    constexpr std::array<uint8_t, 64> kDeBruije = {
         0,  1, 48,  2, 57, 49, 28,  3,
        61, 58, 50, 42, 38, 29, 17,  4,
        62, 55, 59, 36, 53, 51, 43, 22,
        45, 39, 33, 30, 24, 18, 12,  5,
        63, 47, 56, 27, 60, 41, 37, 16,
        54, 35, 52, 21, 44, 32, 23, 11,
        46, 26, 40, 15, 34, 20, 31, 10,
        25, 14, 19, 9, 13,  8,  7,  6
    };

    constexpr Square MirrorVertically(Square square) {
        return static_cast<Square>(static_cast<int>(square) ^ 56);
    }
    
    // Expects the bitboard to only have one set bit, to be a power of 2.
    constexpr Square BitboardToSquare(uint64_t bitboard_sq) {return (Square)(kDeBruije[(bitboard_sq * kDeBruijeMagic) >> 58]);}
    constexpr uint64_t SquareToBitboard(Square square) { return (1ULL << square);}
    inline uint64_t LSB(uint64_t bitboard) {return bitboard & -bitboard;}
    inline Square LSBSquare(uint64_t bitboard) {return BitboardToSquare(LSB(bitboard));}
    
    // ----------------------------------------------------------------------------------------
    // ---------------------------------------- PIECES ----------------------------------------
    // ----------------------------------------------------------------------------------------

    inline constexpr size_t kNumPieces = 13;
    enum Piece : uint8_t {
        kWhitePawn = 0,
        kWhiteKnight,
        kWhiteBishop,
        kWhiteRook,
        kWhiteQueen,
        kWhiteKing,
        kBlackPawn,
        kBlackKnight,
        kBlackBishop,
        kBlackRook,
        kBlackQueen,
        kBlackKing,
        kEmpty
    };

    // ----------------------------------------------------------------------------------------
    // ---------------------------------------- COLORS ----------------------------------------
    // ----------------------------------------------------------------------------------------
    
    inline constexpr size_t kNumColors = 2;
    enum Color : uint8_t {
        kWhite = 0,
        kBlack
    };

    // Pieces and colors.
    inline constexpr Color OppositeColor(Color color) { return static_cast<Color>(static_cast<uint8_t>(color) ^ 1); }
    inline constexpr Color GetPieceColor(Piece piece) {
        assert(piece != kEmpty);

        return piece <= kWhiteKing
            ? kWhite
            : kBlack;
    }

    // -----------------------------------------------------------------------------------------
    // ------------------------------------ CASTLING RIGHTS ------------------------------------
    // -----------------------------------------------------------------------------------------
    
    inline constexpr size_t kNumCastles = 4;
    enum Castle : uint8_t {
        kWhiteQueenSide = 1 << 0,
        kWhiteKingSide = 1 << 1,
        kBlackQueenSide = 1 << 2,
        kBlackKingSide = 1 << 3
    };
    static constexpr uint8_t kCastlesByColor[2] = {3, 12};
    
    // -----------------------------------------------------------------------------------------
    // --------------------------------------- BITBOARDS ---------------------------------------
    // -----------------------------------------------------------------------------------------

    // File / rank bitboard masks.
    static constexpr uint64_t kFileA = 0x0101010101010101ULL;
    static constexpr uint64_t kFileB = kFileA << 1;
    static constexpr uint64_t kFileC = kFileA << 2;
    static constexpr uint64_t kFileD = kFileA << 3;
    static constexpr uint64_t kFileE = kFileA << 4;
    static constexpr uint64_t kFileF = kFileA << 5;
    static constexpr uint64_t kFileG = kFileA << 6;
    static constexpr uint64_t kFileH = kFileA << 7;
    static constexpr uint64_t kRank1 = 0x00000000000000ffULL;
    static constexpr uint64_t kRank2 = kRank1 << 8;
    static constexpr uint64_t kRank3 = kRank1 << 16;
    static constexpr uint64_t kRank4 = kRank1 << 24;
    static constexpr uint64_t kRank5 = kRank1 << 32;
    static constexpr uint64_t kRank6 = kRank1 << 40;
    static constexpr uint64_t kRank7 = kRank1 << 48;
    static constexpr uint64_t kRank8 = kRank1 << 56;
    
    static constexpr uint64_t kRankPawnDoublePush[kNumColors] = {kRank2, kRank7}; 
    static constexpr uint64_t kRankPromotion[kNumColors] = {kRank8, kRank1};
    
    static constexpr uint64_t kFiles[8] = {kFileA, kFileB, kFileC, kFileD, kFileE, kFileF, kFileG, kFileH};
    static constexpr uint64_t kRanks[8] = {kRank1, kRank2, kRank3, kRank4, kRank5, kRank6, kRank7, kRank8};

    constexpr uint64_t FileBB(uint64_t bitboard_sq) {return kFiles[File(BitboardToSquare(bitboard_sq))];}
    constexpr uint64_t RankBB(uint64_t bitboard_sq) {return kRanks[Rank(BitboardToSquare(bitboard_sq))];}

    // Directional moving, handles edges correctly.
    constexpr uint64_t North(uint64_t bitboard) {return bitboard << 8;}
    constexpr uint64_t South(uint64_t bitboard) {return bitboard >> 8;}
    constexpr uint64_t East(uint64_t bitboard) {return (bitboard & ~kFileH) << 1;}
    constexpr uint64_t West(uint64_t bitboard) {return (bitboard & ~kFileA) >> 1;}
    constexpr uint64_t NorthEast(uint64_t bitboard) {return North(East(bitboard));}
    constexpr uint64_t SouthEast(uint64_t bitboard) {return South(East(bitboard));}
    constexpr uint64_t SouthWest(uint64_t bitboard) {return South(West(bitboard));}
    constexpr uint64_t NorthWest(uint64_t bitboard) {return North(West(bitboard));}
    constexpr uint64_t Forward(uint64_t bitboard, Color color) {return color ? South(bitboard) : North(bitboard);}
    constexpr uint64_t Backward(uint64_t bitboard, Color color) {return color ? North(bitboard) : South(bitboard);}

    inline size_t SetBitsCount(uint64_t bitboard) {
        size_t count = 0;
        while (bitboard) {
            bitboard &= bitboard - 1;
            ++count;
        }
        return count;
    }

    //        Forward                             Backward
    //   8  . . . * . . . .                 8  . . . . . . . .
    //   7  . . . * . . . .                 7  . . . . . . . .
    //   6  . . . * . . . .                 6  . . . . . . . .
    //   5  . . . * . . . .                 5  . . . . . . . .
    //   4  . . . P . . . .                 4  . . . p . . . .
    //   3  . . . . . . . .                 3  . . . * . . . .
    //   2  . . . . . . . .                 2  . . . * . . . .
    //   1  . . . . . . . .                 1  . . . * . . . .
    constexpr auto kForwardFillBB = [] {
        std::array<uint64_t, kNumColors * kNumSquares> table{};

        for (int color = 0; color < 2; ++color) {
            for (int square = 0; square < 64; ++square) {
                const uint64_t bb = 1ULL << square;

                table[color * kNumSquares + square] = (color == static_cast<int>(Color::kWhite)) 
                    ? ~(bb | (bb - 1)) & FileBB(bb) : (bb - 1) & FileBB(bb);
            }
        }

        return table;
    }();

    constexpr uint64_t ForwardFillBB(uint64_t bitboard_sq, Color color) {
        if (!bitboard_sq)
            return 0ull;

        return kForwardFillBB[64 * color + BitboardToSquare(bitboard_sq)];
    }
    constexpr uint64_t BackwardFillBB(uint64_t bitboard_sq, Color color) {
        if (!bitboard_sq)
            return 0ull;
        
        return kForwardFillBB[64 * (1-color) + BitboardToSquare(bitboard_sq)];
    }

    //        Forward                             Backward
    //   8  . . * . * . . .                 8  . . . . . . . .
    //   7  . . * . * . . .                 7  . . . . . . . .
    //   6  . . * . * . . .                 6  . . . . . . . .
    //   5  . . * . * . . .                 5  . . . . . . . .
    //   4  . . . P . . . .                 4  . . . p . . . .
    //   3  . . . . . . . .                 3  . . * . * . . .
    //   2  . . . . . . . .                 2  . . * . * . . .
    //   1  . . . . . . . .                 1  . . * . * . . .
    constexpr auto kForwardAdjacentFillBB = [] {
        std::array<uint64_t, kNumColors * kNumSquares> table{};

        for (int color = 0; color < 2; ++color) {
            for (int square = 0; square < 64; ++square) {
                const uint64_t bb = SquareToBitboard(static_cast<Square>(square));

                table[color * kNumSquares + square] = ForwardFillBB(West(bb), static_cast<Color>(color)) 
                    | ForwardFillBB(East(bb), static_cast<Color>(color));
            }
        }

        return table;
    } ();
    constexpr uint64_t ForwardAdjacentFillBB(uint64_t bitboard_sq, Color color) {
        if (!bitboard_sq)
            return 0ull;

        return kForwardAdjacentFillBB[64 * color + BitboardToSquare(bitboard_sq)];
    }
    constexpr uint64_t BackwardAdjacentFillBB(uint64_t bitboard_sq, Color color) {
        if (!bitboard_sq)
            return 0ull;

        return kForwardAdjacentFillBB[64 * (1-color) + BitboardToSquare(bitboard_sq)];
    }

    //        Forward                             Backward
    //   8  . . * * * . . .                 8  . . . . . . . .
    //   7  . . * * * . . .                 7  . . . . . . . .
    //   6  . . * * * . . .                 6  . . . . . . . .
    //   5  . . * * * . . .                 5  . . . . . . . .
    //   4  . . . P . . . .                 4  . . . p . . . .
    //   3  . . . . . . . .                 3  . . * * * . . .
    //   2  . . . . . . . .                 2  . . * * * . . .
    //   1  . . . . . . . .                 1  . . * * * . . .
    constexpr auto kForwardThreeFillBB = [] {
        std::array<uint64_t, kNumColors * kNumSquares> table{};

        for (int color = 0; color < 2; ++color) {
            for (int square = 0; square < 64; ++square) {
                const uint64_t bb = 1ULL << square;

                table[color * kNumSquares + square] = ForwardFillBB(bb, static_cast<Color>(color)) 
                    | ForwardAdjacentFillBB(bb, static_cast<Color>(color));
            }
        }

        return table;
    } ();
    constexpr uint64_t ForwardThreeFillBB(uint64_t bitboard_sq, Color color) {
        if (!bitboard_sq)
            return 0ull;

        return kForwardThreeFillBB[64 * color + BitboardToSquare(bitboard_sq)];
    }
    constexpr uint64_t BackwardThreeFillBB(uint64_t bitboard_sq, Color color) {
        if (!bitboard_sq)
            return 0ull;

        return kForwardThreeFillBB[64 * (1-color) + BitboardToSquare(bitboard_sq)];
    }
    

    // ---------- Magic Bitboards ----------
    // Bishop attacks magics. More info at: https://www.chessprogramming.org/Magic_Bitboards
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

    // Rook attacks magics.
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
    
    // Bitboard mask for possible blockers of a bishop.
    // Includes all squares along the bishop's rank and file, excluding board edges.
    //
    //   8  . . . . . . . .                 8  . . . . . . . .
    //   7  . * . * . . . .                 7  . . . . . . . .
    //   6  . . B . . . . .                 6  . . . . . . . .
    //   5  . * . * . . . .                 5  . . . . . . * .
    //   4  . . . . * . . .                 4  . . . . . * . .
    //   3  . . . . . * . .                 3  . . . . * . . .
    //   2  . . . . . . * .                 2  . * . * . . . .
    //   1  . . . . . . . .                 1  . . B . . . . .
    inline constexpr std::array<uint64_t, kNumSquares> kBishopBlockersMaskBB = [] {
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
    } ();

    // Bitboard mask for possible blockers of a rook.
    // Includes all squares along the rook's rank and file, excluding board edges.
    //
    //   8  . . . . . . . .                 8  . . . . . . . .
    //   7  . . . . * . . .                 7  * . . . . . . .
    //   6  . . . . * . . .                 6  * . . . . . . .
    //   5  . . . . * . . .                 5  R * * * * * * .
    //   4  . * * * R * * .                 4  * . . . . . . .
    //   3  . . . . * . . .                 3  * . . . . . . .
    //   2  . . . . * . . .                 2  * . . . . . . .
    //   1  . . . . . . . .                 1  . . . . . . . .
    inline constexpr std::array<uint64_t, kNumSquares> kRookBlockersMaskBB = [] {
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
    } ();

    // Bishop attack bitboards table indexed by magic hash.
    // For each square a bishop may be on and bitboard of relevant blockers (that lie on the blockers
    // mask) it stores the possible bishop attack squares, including the first blocker in that direction.
    //   8  . . . . . # . .                 8  . * . . . * . .
    //   7  . . . . . . . .                 7  . . * . * . . .
    //   6  . . . B . . . .                 6  . . . B . . . .
    //   5  . . # . . . . .     --->        5  . . * . * . . .
    //   4  . # . . . # . .                 4  . . . . . * . .
    //   3  # . . . . . . .                 3  . . . . . . . .
    //   2  . . . . . . . #                 2  . . . . . . . .
    //   1  . . . . . . . .                 1  . . . . . . . .
    inline constexpr std::array<uint64_t, kBishopAttacksArraySize> kBishopAttacksBB = [] {
        std::array<uint64_t, kBishopAttacksArraySize> bishop_attacks = {0ULL};
        
        for (size_t sq = 0; sq < kNumSquares; sq++) {
            // The diagonal lines centered on that sq.
            uint64_t relevant_occupancy = kBishopBlockersMaskBB[sq];

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
    } ();
    // Easier way to probe kBishopAttacksBB. `blockers_bitboard` may have bits set outside the
    // blockers mask.
    inline constexpr uint64_t BishopAttackBB(Square sq, uint64_t blockers_bitboard) {
        uint64_t relevant_blockers = blockers_bitboard & kBishopBlockersMaskBB[sq];
        uint64_t magics_square_hash = (relevant_blockers * kBishopMagics[sq]) >> kBishopMagicShift;
        uint64_t magics_hash = sq * (1ULL << (64 - kBishopMagicShift)) + magics_square_hash;
        
        return kBishopAttacksBB[magics_hash];
    }

    // Rook attack bitboards table indexed by magic hash.
    // For each square a rook may be on and bitboard of relevant blockers (that lie on the blockers
    // mask) it stores the possible rook attack squares, including the first blocker in that direction.
    //   8  . . . . . . . .                 8  . . . . . . . .
    //   7  . . . . # . . .                 7  . . . . . . . .
    //   6  . . . . . . . .                 6  . . . . . . . .
    //   5  . . . . # . . .     --->        5  . . . . * . . .
    //   4  . # . . R . . .                 4  . * * * R * * *
    //   3  . . . . . . . .                 3  . . . . * . . .
    //   2  . . . . . . . .                 2  . . . . * . . .
    //   1  . . . . # . . .                 1  . . . . * . . .
    inline constexpr std::array<uint64_t, kRookAttacksArraySize> kRookAttacksBB = [] {
        std::array<uint64_t, kRookAttacksArraySize> rook_attacks = {0ULL};
        
        for (size_t sq = 0; sq < kNumSquares; sq++) {
            // The diagonal lines centered on that sq.
            uint64_t relevant_occupancy = kRookBlockersMaskBB[sq];

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
    } ();
    // Easier way to probe kRookAttacksBB. `blockers_bitboard` may have bits set outside the
    // blockers mask.
    inline constexpr uint64_t RookAttackBB(Square sq, uint64_t blockers_bitboard) {
        uint64_t relevant_blockers = blockers_bitboard & kRookBlockersMaskBB[sq];
        uint64_t magics_square_hash = (relevant_blockers * kRookMagics[sq]) >> kRookMagicShift;
        uint64_t magics_hash = sq * (1ULL << (64 - kRookMagicShift)) + magics_square_hash;
        
        return kRookAttacksBB[magics_hash];
    }
    
    //   8  . . # . . . . .                 8  . . . . . . . .
    //   7  . # . . # . . #                 7  . * . . . . . *
    //   6  . . . . . . . #                 6  . . * . . . * .
    //   5  . . . . # . . #     --->        5  . . . * * * . .
    //   4  . # . . Q . # #                 4  . * * * Q * * *
    //   3  . . . . . . . .                 3  . . . * * * . .
    //   2  . . # . . . . .                 2  . . * . * . * .
    //   1  # # # # . . . .                 1  . . . . * . . *
    inline constexpr uint64_t QueenAttackBB(Square sq, uint64_t blockers_bitboard) {
        return BishopAttackBB(sq, blockers_bitboard) | RookAttackBB(sq, blockers_bitboard);
    }

    // Precompute pawn attacks, the 2 diagonal squares in front.
    inline constexpr std::array<uint64_t, 2 * kNumSquares> kPawnAttacksBB = [] {
        std::array<uint64_t, 2 * kNumSquares> pawn_attacks = {0ULL};

        for (size_t sq = 0; sq < kNumSquares; sq++) {
            uint64_t sq_bb = SquareToBitboard((Square)sq);

            // White & black pawn attacks
            pawn_attacks[sq] = NorthWest(sq_bb) | NorthEast(sq_bb);
            pawn_attacks[kNumSquares + sq] = SouthWest(sq_bb) | SouthEast(sq_bb);
        }

        return pawn_attacks;
    } ();
    // More convinient way to get pawn attacks.
    inline constexpr uint64_t PawnAttackBB(Square sq, Color color) {
        return kPawnAttacksBB[color * kNumSquares + sq];
    }

    // Precomputed knight attacks.
    //   8  . . . . . . . .                 8  . . . . . . . .
    //   7  . . . . . . . .                 7  . . . . . . . .
    //   6  . * . * . . . .                 6  . . . . . . . .
    //   5  * . . . * . . .                 5  . . . . . . . .
    //   4  . . N . . . . .                 4  . . . . . . . .
    //   3  * . . . * . . .                 3  . . . . . * . *
    //   2  . * . * . . . .                 2  . . . . * . . .
    //   1  . . . . . . . .                 1  . . . . . . N .
    inline constexpr std::array<uint64_t, kNumSquares> kKnightAttacksBB = [] {
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
    } ();

    // Precompute king attacks, the 8 (or less) positions surrounding the origin square.
    //   8  . . . . . . . .                 8  . . . . . . . .
    //   7  . . . . . . . .                 7  . . . . . . . .
    //   6  . . . . . . . .                 6  . . . . . . . .
    //   5  . . . . . . . .                 5  . . . . . . . .
    //   4  . . . * * * . .                 4  . . . . . . . .
    //   3  . . . * K * . .                 3  . . . . . . . .
    //   2  . . . * * * . .                 2  . . . * * * . .
    //   1  . . . . . . . .                 1  . . . * K * . .
    inline constexpr std::array<uint64_t, kNumSquares> kKingAttacksBB = [] {
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
    } ();

    // -----------------------------------------------------------------------------------------
    // ----------------------------------------- MOVES -----------------------------------------
    // -----------------------------------------------------------------------------------------

    enum PromotionPieceType : uint16_t {
        kKnight,
        kBishop = 1 << 12,
        kRook = 2 << 12,
        kQueen = 3 << 12
    };
    std::ostream& operator<<(std::ostream& os, lightknight::PromotionPieceType piece);

    inline constexpr Piece GetPiece(
        Color color,
        PromotionPieceType promotion_piece_type
    ) {
        return static_cast<Piece>(
            static_cast<std::uint8_t>(color) * 6
            + 1
            + (static_cast<std::uint16_t>(promotion_piece_type) >> 12)
        );
    }

    enum MoveType : uint16_t {
        kNormal,
        kPromotion = 1 << 14,
        kCastling = 2 << 14,
        kEnPassant = 3 << 14
    };

    // bits 0-5: origin square (value from 0 to 63)
    // bits 6-11: destination square (value from 0 to 63)
    // bits 12-13: promotion piece (defined above knight=0, bishop=1, rook=2, queen=3)
    // bits 14-15: move type flag (defined above promotion=1, castling=2, en_passant=3)
    class Move {
    public:
        uint16_t data;

        // Constructors
        constexpr Move() : data(0) {}
        constexpr explicit Move(uint16_t data) : data(data) {}

        constexpr Move(Square origin, Square destination, PromotionPieceType promotionPieceType = PromotionPieceType::kKnight, MoveType moveType = MoveType::kNormal)
            : data(origin + (destination << 6) + promotionPieceType + moveType) {}
        
        // Static functions to create Move objects.
        static constexpr Move Make(Square origin, Square destination, PromotionPieceType promotionPieceType = PromotionPieceType::kKnight, MoveType moveType = MoveType::kNormal) {
            return Move(origin + (destination << 6) + promotionPieceType + moveType); 
        };

        constexpr bool IsNull() const { return data == 0; }

        // Operator overloads
        constexpr bool operator==(const Move& move) const { return data == move.data; }
        constexpr bool operator!=(const Move& move) const { return data != move.data; }  
        constexpr explicit operator bool() const { return data != 0; }

        // Methods to return certain parts of the move.
        constexpr Square GetOriginSquare() const { return (Square)(data & 0x3F); }
        constexpr Square GetDestinationSquare() const { return (Square)((data >> 6) & 0x3F); }
        constexpr PromotionPieceType GetPromotionPieceType() const { return (PromotionPieceType)(data & (3 << 12)); } 
        constexpr MoveType GetMoveType() const { return (MoveType)(data & (3 << 14)); } 

        constexpr uint64_t GetOriginBitboard() const { return SquareToBitboard(this->GetOriginSquare()); }
        constexpr uint64_t GetDestionationBitboard() const { return SquareToBitboard(this->GetDestinationSquare()); }
        Piece GetPromotedPiece(Color color) const;

        constexpr bool IsPromotion() const { return GetMoveType() == MoveType::kPromotion; }
        constexpr bool IsQueenPromotion() const {
            return IsPromotion() && GetPromotionPieceType() == PromotionPieceType::kQueen;
        }
        constexpr bool IsUnderpromotion() const {
            return IsPromotion() && GetPromotionPieceType() != PromotionPieceType::kQueen; 
        }
    
        #include <ostream>
    };
    std::ostream& operator<<(std::ostream& os, const lightknight::Move& move);

    // Debug
    void PrintBitboard(uint64_t bitboard);
    
    typedef struct UndoMoveInfo {
        Piece captured_piece = Piece::kEmpty;
        uint8_t castling = 0;
        uint64_t en_passant = 0;
        int halfmoves = 0;
        int fullmoves = 0;
    } UndoMoveInfo;

}; // namespace lightknight

#endif //LIGHTKNIGHT_TYPES_H