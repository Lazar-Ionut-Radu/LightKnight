// types.cc
#include "types.h"
#include <ostream>

namespace lightknight {
    std::ostream& operator<<(std::ostream& os, Square sq) {
        return os << (char)(File(sq) + 'a') << Rank(sq) + 1;
    }

    std::ostream& operator<<(std::ostream& os, lightknight::PromotionPieceType piece)
    {
        using lightknight::PromotionPieceType;

        switch (piece) {
            case PromotionPieceType::kKnight: os << 'n'; break;
            case PromotionPieceType::kBishop: os << 'b'; break;
            case PromotionPieceType::kRook:   os << 'r'; break;
            case PromotionPieceType::kQueen:  os << 'q'; break;
            default:
                os << '?';
                break;
        }

        return os;
}

    std::ostream& operator<<(std::ostream& os, const lightknight::Move& move)
    {
        // Small func to get square str
        const auto PrintSquare = [&os](lightknight::Square square) {
            const auto value = static_cast<std::uint8_t>(square);

            os << static_cast<char>('a' + value % 8)
            << static_cast<char>('1' + value / 8);
        };

        PrintSquare(move.GetOriginSquare());
        PrintSquare(move.GetDestinationSquare());
        
        const auto from = move.GetOriginSquare();
        const auto to   = move.GetDestinationSquare();

        if (move.GetMoveType() == lightknight::MoveType::kPromotion) {
            constexpr char promotion_chars[] = {'n', 'b', 'r', 'q'};

            const auto promotion_index =
                static_cast<std::uint16_t>(
                    move.GetPromotionPieceType()
                ) >> 12;

            os << promotion_chars[promotion_index];
        }

        return os;
    }

    Piece Move::GetPromotedPiece(Color color) const {
        assert(GetMoveType() == MoveType::kPromotion);

        const unsigned prom_idx = static_cast<unsigned>(GetPromotionPieceType()) >> 12;

        // 0 = knight
        // 1 = bishop
        // 2 = rook
        // 3 = queen

        const unsigned piece_idx = 
            static_cast<unsigned>(Piece::kWhiteKnight)
            + prom_idx
            + 6 * static_cast<unsigned>(color);

        return static_cast<Piece>(piece_idx);
    } 

    void PrintBitboard(uint64_t bitboard) {
        for (int rank = 7; rank >= 0; rank--) {
            for (int file = 0; file <= 7; file++) {
                if (bitboard & (1ULL << (8*rank + file)))
                    std::cout << "O";
                else
                    std::cout << "-";
            }
            std::cout << std::endl;
        }
    }
} // namespace lightknight
