#ifndef LIGHTKNIGHT_TYPES_H
#define LIGHTKNIGHT_TYPES_H

#include <cstdint>
#include <cstddef>
#include <array>
#include <iostream>
#include <tuple>
#include <cassert>

namespace lightknight {
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

    inline constexpr size_t kNumColors = 2;
    enum Color : uint8_t {
        kWhite = 0,
        kBlack
    };

    inline constexpr size_t kNumCastles = 4;
    enum Castle : uint8_t {
        kWhiteQueenSide = 1 << 0,
        kWhiteKingSide = 1 << 1,
        kBlackQueenSide = 1 << 2,
        kBlackKingSide = 1 << 3
    };
    static constexpr uint8_t kCastlesByColor[2] = {3, 12};
    
    // Pieces and colors.
    inline constexpr Color OppositeColor(Color color) { return static_cast<Color>(static_cast<uint8_t>(color) ^ 1); }
    inline constexpr Color GetPieceColor(Piece piece) {
        assert(piece != kEmpty);

        return piece <= kWhiteKing
            ? kWhite
            : kBlack;
    }
    
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

    // Directional moving, handles edges correctly.
    constexpr uint64_t North(uint64_t bitboard) {return bitboard << 8;}
    constexpr uint64_t South(uint64_t bitboard) {return bitboard >> 8;}
    constexpr uint64_t East(uint64_t bitboard) {return (bitboard & ~kFileH) << 1;}
    constexpr uint64_t West(uint64_t bitboard) {return (bitboard & ~kFileA) >> 1;}
    constexpr uint64_t NorthEast(uint64_t bitboard) {return North(East(bitboard));}
    constexpr uint64_t SouthEast(uint64_t bitboard) {return South(East(bitboard));}
    constexpr uint64_t SouthWest(uint64_t bitboard) {return South(West(bitboard));}
    constexpr uint64_t NorthWest(uint64_t bitboard) {return North(West(bitboard));}
    
    // Pawn moves
    constexpr uint64_t Forward(uint64_t bitboard, Color color) {
        return color ? South(bitboard) : North(bitboard);
    }
    constexpr uint64_t Backward(uint64_t bitboard, Color color) {
        return color ? North(bitboard) : South(bitboard);
    }

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
    
    inline size_t SetBitsCount(uint64_t bitboard) {
        size_t count = 0;
        while (bitboard) {
            bitboard &= bitboard - 1;
            ++count;
        }
        return count;
    }

    constexpr int Rank(Square square) {return square / 8;}
    constexpr int File(Square square) {return square % 8;}
    constexpr Square GetSquare(int rank, int file) {return (Square)(8*rank + file);}

    constexpr uint64_t FileBB(uint64_t bitboard_sq) {return kFiles[File(BitboardToSquare(bitboard_sq))];}
    constexpr uint64_t RankBB(uint64_t bitboard_sq) {return kRanks[Rank(BitboardToSquare(bitboard_sq))];}

    // Pawn bitboards helpers.
    
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
                const uint64_t bb = 1ULL << square;

                table[color * kNumSquares + square] = kForwardFillBB[color * kNumSquares + BitboardToSquare(West(SquareToBitboard(static_cast<Square>(square))))]
                    | kForwardFillBB[color * kNumSquares + BitboardToSquare(East(SquareToBitboard(static_cast<Square>(square))))];
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

    // Examples for the ForwardThreeFillBB.
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

                table[color * kNumSquares + square] = kForwardFillBB[color * kNumSquares + square] 
                    | kForwardAdjacentFillBB[color * kNumSquares + square] ;
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