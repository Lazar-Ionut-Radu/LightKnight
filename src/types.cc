// types.cc
#include "types.h"
#include <ostream>

namespace lightknight {
    std::ostream& operator<<(std::ostream& os, Square sq) {
        return os << (char)(File(sq) + 'a') << Rank(sq) + 1;
    }

    std::ostream& operator<<(std::ostream& os, lightknight::PromPieceType piece) {
        switch (piece) {
            case PromPieceType::kKnight: os << 'n'; break;
            case PromPieceType::kBishop: os << 'b'; break;
            case PromPieceType::kRook:   os << 'r'; break;
            case PromPieceType::kQueen:  os << 'q'; break;
            default:
                os << '?';
                break;
        }

        return os;
    }

    std::ostream& operator<<(std::ostream& os, const Move& move) {
        // Small func to get square str
        const auto PrintSquare = [&os](Square square) {
            const auto value = static_cast<std::uint8_t>(square);

            os << static_cast<char>('a' + value % 8)
            << static_cast<char>('1' + value / 8);
        };

        PrintSquare(move.OriginSquare());
        PrintSquare(move.DestSquare());
        
        if (move.GetMoveType() == MoveType::kPromotion) {
            constexpr char promo_chars[] = {'n', 'b', 'r', 'q'};

            const auto prom_index = static_cast<std::uint32_t>(move.GetPromPieceType()) >> 20;
            os << promo_chars[prom_index];
        }

        return os;
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
