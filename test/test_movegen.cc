// test/test_movegen.cc
#include <catch2/catch_test_macros.hpp>
#include "movegen.h"
#include "types.h"
#include "board.h"
#include <array>
#include <vector>
#include <tuple>
#include <algorithm>
#include <ranges>
#include <string>

TEST_CASE(
    "Sliding Piece Relevant Occupacy (for magic bitboards generation)",
    "[UnitTest][Move][MoveGen][MagicBitboards]"
) {
    SECTION("Bishop") {
        static const std::array<std::pair<lightknight::Square, uint64_t>, 10> bishop_tests = {
            std::make_pair(lightknight::Square::A1, 0x40201008040200ULL),
            std::make_pair(lightknight::Square::H8, 0x40201008040200ULL),
            std::make_pair(lightknight::Square::A8, 0x2040810204000ULL),
            std::make_pair(lightknight::Square::H1, 0x2040810204000ULL),
            std::make_pair(lightknight::Square::D1, 0x40221400ULL),
            std::make_pair(lightknight::Square::H2, 0x4081020400000ULL),
            std::make_pair(lightknight::Square::F8, 0x50080402000000ULL),
            std::make_pair(lightknight::Square::C6, 0xa000a10204000ULL),
            std::make_pair(lightknight::Square::E5, 0x44280028440200ULL),
            std::make_pair(lightknight::Square::B7, 0x40810204000ULL)
        };

        for (auto& [sq, bb] : bishop_tests) {
            CHECK(lightknight::movegen::kBishopRelevantOccupancy[sq] == bb);
        }
    }

    SECTION("Rook") {
        static const std::array<std::pair<lightknight::Square, uint64_t>, 10> rook_tests = {
            std::make_pair(lightknight::Square::A1, 0x101010101017eULL),
            std::make_pair(lightknight::Square::H1, 0x8080808080807eULL),
            std::make_pair(lightknight::Square::H8, 0x7e80808080808000ULL),
            std::make_pair(lightknight::Square::H1, 0x8080808080807eULL),
            std::make_pair(lightknight::Square::A5, 0x1017e01010100ULL),
            std::make_pair(lightknight::Square::G1, 0x4040404040403eULL),
            std::make_pair(lightknight::Square::G7, 0x3e404040404000ULL),
            std::make_pair(lightknight::Square::E4, 0x1010106e101000ULL),
            std::make_pair(lightknight::Square::D2, 0x8080808087600ULL),
            std::make_pair(lightknight::Square::C5, 0x4047a04040400ULL)
        };

        for (auto& [sq, bb] : rook_tests) {
            CHECK(lightknight::movegen::kRookRelevantOccupancy[sq] == bb);
        }
    }
}

TEST_CASE(
    "Precomputed Attack Bitboards",
    "[UnitTest][Move][MoveGen][MagicBitboards]"
) {
    SECTION("Pawn") {
        static const std::array<std::tuple<lightknight::Square, lightknight::Color, uint64_t>, 8> pawn_tests = {
            std::make_tuple(lightknight::Square::C2, lightknight::Color::kWhite, 0xa0000ULL),
            std::make_tuple(lightknight::Square::H4, lightknight::Color::kWhite, 0x4000000000ULL),
            std::make_tuple(lightknight::Square::A8, lightknight::Color::kWhite, 0ULL),
            std::make_tuple(lightknight::Square::F1, lightknight::Color::kWhite, 0x5000ULL),
            std::make_tuple(lightknight::Square::B6, lightknight::Color::kBlack, 0x500000000ULL),
            std::make_tuple(lightknight::Square::H4, lightknight::Color::kBlack, 0x400000ULL),
            std::make_tuple(lightknight::Square::E1, lightknight::Color::kBlack, 0ULL),
            std::make_tuple(lightknight::Square::C8, lightknight::Color::kBlack, 0xa000000000000ULL),
        };

        for (auto& [sq, color, bb] : pawn_tests) {
            CHECK(lightknight::movegen::PawnAttackBitboard(sq, color) == bb);
        }
    };

    SECTION("Knight") {
        static const std::array<std::pair<lightknight::Square, uint64_t>, 8> knight_tests = {
            std::make_pair(lightknight::Square::A1, 0x20400ULL),
            std::make_pair(lightknight::Square::B7, 0x800080500000000ULL),
            std::make_pair(lightknight::Square::G1, 0xa01000ULL),
            std::make_pair(lightknight::Square::G3, 0xa0100010a0ULL),
            std::make_pair(lightknight::Square::F6, 0x5088008850000000ULL),
            std::make_pair(lightknight::Square::D5, 0x14220022140000ULL),
            std::make_pair(lightknight::Square::E8, 0x44280000000000ULL),
            std::make_pair(lightknight::Square::A2, 0x2040004ULL)
        };

        for (auto& [sq, bb] : knight_tests) {
            CHECK(lightknight::movegen::kKnightAttacks[sq] == bb);
        }
    }

    SECTION("Bishop") {
        static const std::array<std::tuple<lightknight::Square, uint64_t, uint64_t>, 13> bishop_tests = {
            std::make_tuple(lightknight::Square::A1, 0ULL, 0x8040201008040200ULL),
            std::make_tuple(lightknight::Square::E5, 0ULL, 0x8244280028448201ULL),
            std::make_tuple(lightknight::Square::G7, 0ULL, 0xa000a01008040201ULL),
            std::make_tuple(lightknight::Square::F1, 0ULL, 0x10204885000ULL),
            std::make_tuple(lightknight::Square::D6, 0ULL, 0x2214001422418000ULL),
            std::make_tuple(lightknight::Square::D4, 0x430100003000008ULL, 0x8041221400142241ULL),
            std::make_tuple(lightknight::Square::C1, 0x91404040440ULL, 0x804020110a00ULL),
            std::make_tuple(lightknight::Square::H1, 0xf80020000100bf7fULL, 0x102040810204000ULL),
            std::make_tuple(lightknight::Square::D2, 0x404000040000ULL, 0x4020140014ULL),
            std::make_tuple(lightknight::Square::E1, 0x40000ULL, 0x80442800ULL),
            std::make_tuple(lightknight::Square::E5, 0x280028000000ULL, 0x280028000000ULL),
            std::make_tuple(lightknight::Square::E5, 0x8200000000008001ULL, 0x8244280028448201ULL),
            std::make_tuple(lightknight::Square::C5, 0x24040002207ULL, 0x20100a000a112000ULL)
        };

        for (auto& [sq, blockers, bb] : bishop_tests) {
            CHECK(lightknight::movegen::BishopAttackBitboard(sq, blockers) == bb);
        }

    }

    SECTION("Rook") {
        static const std::array<std::tuple<lightknight::Square, uint64_t, uint64_t>, 14> rook_tests = {
            std::make_tuple(lightknight::Square::A1, 0ULL, 0x1010101010101feULL),
            std::make_tuple(lightknight::Square::B4, 0ULL, 0x2020202fd020202ULL),
            std::make_tuple(lightknight::Square::G1, 0ULL, 0x40404040404040bfULL),
            std::make_tuple(lightknight::Square::H2, 0ULL, 0x8080808080807f80ULL),
            std::make_tuple(lightknight::Square::E7, 0x40307030303ULL, 0x10ef101010101010ULL),
            std::make_tuple(lightknight::Square::H8, 0x40307030303ULL, 0x7f80808080808080ULL),
            std::make_tuple(lightknight::Square::F8, 0x1800400000001ULL, 0xdf20202020202020ULL),
            std::make_tuple(lightknight::Square::B3, 0x200200200ULL, 0x2023d0200ULL),
            std::make_tuple(lightknight::Square::H1, 0x80080080001cULL, 0x808070ULL),
            std::make_tuple(lightknight::Square::C5, 0x4023300000000ULL, 0x4041a04040404ULL),
            std::make_tuple(lightknight::Square::E2, 0x10202000ULL, 0x10102f10ULL),
            std::make_tuple(lightknight::Square::F1, 0x2004ULL, 0x20dcULL),
            std::make_tuple(lightknight::Square::B1, 0x205ULL, 0x205ULL),
            std::make_tuple(lightknight::Square::A8, 0x201000000000000ULL, 0x201000000000000ULL)
        };
        
        for (auto& [sq, blockers, bb] : rook_tests) {
            CHECK(lightknight::movegen::RookAttackBitboard(sq, blockers) == bb);
        }
    }

    SECTION("Queen") {
        static const std::array<std::tuple<lightknight::Square, uint64_t, uint64_t>, 13> queen_tests = {
            std::make_tuple(lightknight::Square::A1, 0ULL, 0x81412111090503feULL),
            std::make_tuple(lightknight::Square::D1, 0ULL, 0x8080888492a1cf7ULL),
            std::make_tuple(lightknight::Square::G7, 0ULL, 0xe0bfe05048444241ULL),
            std::make_tuple(lightknight::Square::D5, 0ULL, 0x492a1cf71c2a4988ULL),
            std::make_tuple(lightknight::Square::B4, 0x6183c0a10ULL, 0x70d070200ULL),
            std::make_tuple(lightknight::Square::A8, 0x20202020301c0700ULL, 0x3e03050911010100ULL),
            std::make_tuple(lightknight::Square::F8, 0x440082000000000ULL, 0xdc70282000000000ULL),
            std::make_tuple(lightknight::Square::D3, 0x440082000021c00ULL, 0x92a1cf61c00ULL),
            std::make_tuple(lightknight::Square::F3, 0x200000200000ULL, 0x10224a870df70a8ULL),
            std::make_tuple(lightknight::Square::G7, 0x40000000000000ULL, 0xe0bfe05048444241ULL),
            std::make_tuple(lightknight::Square::F5, 0x1000010000030088ULL, 0x24a870df70a82422ULL),
            std::make_tuple(lightknight::Square::H6, 0x1f3f800000000703ULL, 0xa0c07fc0a0908884ULL),
            std::make_tuple(lightknight::Square::H1, 0x7e7d7b776f5f3f00ULL, 0x8182848890a0c07fULL),
        };

        for (auto& [sq, blockers, bb] : queen_tests) {
            CHECK(lightknight::movegen::QueenAttackBitboard(sq, blockers) == bb);
        }
    }

    SECTION("King") {
        static const std::array<std::pair<lightknight::Square, uint64_t>, 6> king_tests = {
            std::make_pair(lightknight::Square::H1, 0xc040ULL),
            std::make_pair(lightknight::Square::H3, 0xc040c000ULL),
            std::make_pair(lightknight::Square::C6, 0xe0a0e00000000ULL),
            std::make_pair(lightknight::Square::G8, 0xa0e0000000000000ULL),
            std::make_pair(lightknight::Square::F3, 0x70507000ULL),
            std::make_pair(lightknight::Square::A8, 0x203000000000000ULL),
        };

        for (auto& [sq, bb] : king_tests) {
            CHECK(lightknight::movegen::kKingAttacks[sq] == bb);
        }
    }
}

void TestEqualMoveLists(std::vector<lightknight::Move> actual_moves,
                        std::vector<lightknight::Move> expected_moves)
{
    for (const auto& move : expected_moves) {
        INFO("Expected move missing: " << move);
        CHECK(std::ranges::find(actual_moves, move) != actual_moves.end());
    }

    for (const auto& move : actual_moves) {
        INFO("Unexpected extra move: " << move);
        CHECK(std::ranges::find(expected_moves, move) != expected_moves.end());
    }
}

struct MoveGenerationTestStruct {
    std::string name;
    std::vector<lightknight::Move> expected_moves;
};

TEST_CASE(
    "Generate Move"
     "[UnitTest][Move][MoveGen]"
) {
    SECTION("Pawn Moves") {
        static const std::array<MoveGenerationTestStruct, 12> pawn_tests = {{
            { // White pawn pushes, 1 or 2 squares, no pins.
                "rnbqkbnr/pppppppp/8/4P3/1P3P2/8/P1PP2PP/RNBQKBNR w KQkq - 0 1",
                {
                    lightknight::Move(lightknight::Square::A2,lightknight::Square::A3),
                    lightknight::Move(lightknight::Square::A2,lightknight::Square::A4),
                    lightknight::Move(lightknight::Square::B4,lightknight::Square::B5),
                    lightknight::Move(lightknight::Square::C2,lightknight::Square::C3),
                    lightknight::Move(lightknight::Square::C2,lightknight::Square::C4),
                    lightknight::Move(lightknight::Square::D2,lightknight::Square::D3),
                    lightknight::Move(lightknight::Square::D2,lightknight::Square::D4),
                    lightknight::Move(lightknight::Square::E5,lightknight::Square::E6),
                    lightknight::Move(lightknight::Square::F4,lightknight::Square::F5),
                    lightknight::Move(lightknight::Square::G2,lightknight::Square::G3),
                    lightknight::Move(lightknight::Square::G2,lightknight::Square::G4),
                    lightknight::Move(lightknight::Square::H2,lightknight::Square::H3),
                    lightknight::Move(lightknight::Square::H2,lightknight::Square::H4)
                }
            },
            { // Black, pawn pushes, 1 or 2 squares, no pins
                "rnbqkbnr/2p4p/3p4/p7/1p6/8/PPPPPPPR/RNBQKBN1 b kq - 0 1",
                {
                    lightknight::Move(lightknight::Square::A5, lightknight::Square::A4),
                    lightknight::Move(lightknight::Square::B4, lightknight::Square::B3),
                    lightknight::Move(lightknight::Square::C7, lightknight::Square::C6),
                    lightknight::Move(lightknight::Square::C7, lightknight::Square::C5),
                    lightknight::Move(lightknight::Square::D6, lightknight::Square::D5),
                    lightknight::Move(lightknight::Square::H7, lightknight::Square::H6),
                    lightknight::Move(lightknight::Square::H7, lightknight::Square::H5),
                }
            },
            { // White, pawn promotions, captures or not, pinned or not to the king.
                "1qn1kb2/P1ppp1P1/7K/8/8/8/3Q3R/RNB5 w - - 0 1",
                {
                    lightknight::Move(lightknight::Square::A7, lightknight::Square::A8,  lightknight::PromotionPieceType::kKnight, lightknight::MoveType::kPromotion),
                    lightknight::Move(lightknight::Square::A7, lightknight::Square::A8,  lightknight::PromotionPieceType::kBishop, lightknight::MoveType::kPromotion),
                    lightknight::Move(lightknight::Square::A7, lightknight::Square::A8,  lightknight::PromotionPieceType::kRook, lightknight::MoveType::kPromotion),
                    lightknight::Move(lightknight::Square::A7, lightknight::Square::A8,  lightknight::PromotionPieceType::kQueen, lightknight::MoveType::kPromotion),
                    lightknight::Move(lightknight::Square::A7, lightknight::Square::B8,  lightknight::PromotionPieceType::kKnight, lightknight::MoveType::kPromotion),
                    lightknight::Move(lightknight::Square::A7, lightknight::Square::B8,  lightknight::PromotionPieceType::kBishop, lightknight::MoveType::kPromotion),
                    lightknight::Move(lightknight::Square::A7, lightknight::Square::B8,  lightknight::PromotionPieceType::kQueen, lightknight::MoveType::kPromotion),
                    lightknight::Move(lightknight::Square::A7, lightknight::Square::B8,  lightknight::PromotionPieceType::kRook, lightknight::MoveType::kPromotion),
                    lightknight::Move(lightknight::Square::G7, lightknight::Square::F8,  lightknight::PromotionPieceType::kKnight, lightknight::MoveType::kPromotion),
                    lightknight::Move(lightknight::Square::G7, lightknight::Square::F8,  lightknight::PromotionPieceType::kBishop, lightknight::MoveType::kPromotion),
                    lightknight::Move(lightknight::Square::G7, lightknight::Square::F8,  lightknight::PromotionPieceType::kRook, lightknight::MoveType::kPromotion),
                    lightknight::Move(lightknight::Square::G7, lightknight::Square::F8,  lightknight::PromotionPieceType::kQueen, lightknight::MoveType::kPromotion),
                }
            },
            { // Black, pawn promotions, captures or not, pinned or not to the king.
                "8/P5P1/7K/8/8/8/1p2Q1pk/B6N b - - 0 1",
                {
                    lightknight::Move(lightknight::Square::B2, lightknight::Square::B1, lightknight::PromotionPieceType::kKnight, lightknight::MoveType::kPromotion),
                    lightknight::Move(lightknight::Square::B2, lightknight::Square::B1, lightknight::PromotionPieceType::kBishop, lightknight::MoveType::kPromotion),
                    lightknight::Move(lightknight::Square::B2, lightknight::Square::B1, lightknight::PromotionPieceType::kRook, lightknight::MoveType::kPromotion),
                    lightknight::Move(lightknight::Square::B2, lightknight::Square::B1, lightknight::PromotionPieceType::kQueen, lightknight::MoveType::kPromotion),
                    lightknight::Move(lightknight::Square::B2, lightknight::Square::A1, lightknight::PromotionPieceType::kKnight, lightknight::MoveType::kPromotion),
                    lightknight::Move(lightknight::Square::B2, lightknight::Square::A1, lightknight::PromotionPieceType::kBishop, lightknight::MoveType::kPromotion),
                    lightknight::Move(lightknight::Square::B2, lightknight::Square::A1, lightknight::PromotionPieceType::kRook, lightknight::MoveType::kPromotion),
                    lightknight::Move(lightknight::Square::B2, lightknight::Square::A1, lightknight::PromotionPieceType::kQueen, lightknight::MoveType::kPromotion),
                }
            },
            { // White, pawn captures, pinned or not.
                "r3k1nr/pbppp2p/8/3nqpp1/4PP1b/6P1/7P/RNBQKBNR w KQkq - 0 1",
                {
                    lightknight::Move(lightknight::Square::F4, lightknight::Square::E5),
                    lightknight::Move(lightknight::Square::F4, lightknight::Square::G5),
                    lightknight::Move(lightknight::Square::G3, lightknight::Square::H4),
                    lightknight::Move(lightknight::Square::H2, lightknight::Square::H3)
                }
            },
            { // Black, pawn captures, pinned or not.
                "r3k1nr/1b2p2p/2pR2p1/1N2Rp1Q/B3PP1b/6P1/7P/RNBQKBN1 b - - 0 1",
                {
                    lightknight::Move(lightknight::Square::C6, lightknight::Square::B5),
                    lightknight::Move(lightknight::Square::C6, lightknight::Square::C5),
                    lightknight::Move(lightknight::Square::E7, lightknight::Square::E6),
                    lightknight::Move(lightknight::Square::G6, lightknight::Square::H5),
                    lightknight::Move(lightknight::Square::F5, lightknight::Square::E4),
                    lightknight::Move(lightknight::Square::H7, lightknight::Square::H6)
                }
            },
            { // White en passant
                "rnbqkbnr/ppp1pppp/8/2Pp4/8/8/8/RNBQKBNR w KQkq d6 0 1",
                {
                    lightknight::Move(lightknight::Square::C5,lightknight::Square::C6),
                    lightknight::Move(lightknight::Square::C5,lightknight::Square::D6, lightknight::PromotionPieceType::kKnight, lightknight::MoveType::kEnPassant),
                }
            },
            { // Black en passant 
                "nqrkbbnr/8/8/8/6Pp/8/PPPPPP1P/NQRKBBNR b - g3 0 1",
                {
                    lightknight::Move(lightknight::Square::H4, lightknight::Square::G3, lightknight::PromotionPieceType::kKnight, lightknight::MoveType::kEnPassant),
                    lightknight::Move(lightknight::Square::H4, lightknight::Square::H3),
                }
            },
            { // White, en passant impossible bcs pin (horizontally, xray through 2 pawns)
                "1nbk1bn1/2p5/8/q3pP1K/8/8/8/RNB2BNR w - e6 0 1",
                {
                    lightknight::Move(lightknight::Square::F5, lightknight::Square::F6)
                }
            },
            { // Black, en passant impossible bcs pinned (vertically)
                "rnb1Kbnr/4p3/4R3/8/4pP2/8/8/4k3 b - f3 0 1",
                {
                    lightknight::Move(lightknight::Square::E4, lightknight::Square::E3)
                }
            },
            { // White, to block check.
                "rnb1kbnr/pppppppp/8/8/4K2q/8/PPPPPPPP/RNBQ1BNR w kq - 0 1",
                {
                    lightknight::Move(lightknight::Square::F2, lightknight::Square::F4),
                    lightknight::Move(lightknight::Square::G2, lightknight::Square::G4)
                }
            },
            { // Black, to block check.
                "rnbqkbnr/ppp1pppp/8/8/B7/8/PPPPPPPP/RN1QKBNR b kq - 0 1",
                {
                    lightknight::Move(lightknight::Square::C7, lightknight::Square::C6),
                    lightknight::Move(lightknight::Square::B7, lightknight::Square::B5)
                }
            }
        }};

        for (auto &[fen_str, expected_moves] : pawn_tests) {
            SECTION(fen_str) {
                lightknight::Board board = lightknight::Board(fen_str);

                std::vector<lightknight::Move> moves;
                lightknight::movegen::GeneratePawnMoves<lightknight::movegen::MoveGenType::kAll>(board, moves);

                TestEqualMoveLists(moves, expected_moves);
            }
        }
    }

    SECTION("Knight") {
        static const std::array<MoveGenerationTestStruct, 5> knight_tests = {{
            { // White, takes or not
                "rnbqkbnr/ppppppp1/2N5/R3P2p/3P4/8/1PPB1PPP/1N1QKB1R w Kkq - 0 1",
                {
                    lightknight::Move(lightknight::Square::B1, lightknight::Square::A3),
                    lightknight::Move(lightknight::Square::B1, lightknight::Square::C3),
                    lightknight::Move(lightknight::Square::C6, lightknight::Square::B4),
                    lightknight::Move(lightknight::Square::C6, lightknight::Square::A7),
                    lightknight::Move(lightknight::Square::C6, lightknight::Square::B8),
                    lightknight::Move(lightknight::Square::C6, lightknight::Square::D8),
                    lightknight::Move(lightknight::Square::C6, lightknight::Square::E7),
                }
            },
            { // Black, taken or not
                "rnbqkb1r/ppppppp1/2n5/R3P2p/3P4/8/1PPB1PPP/1N1QKB1R b Kkq - 0 1",
                {
                    lightknight::Move(lightknight::Square::B8, lightknight::Square::A6),
                    lightknight::Move(lightknight::Square::C6, lightknight::Square::B4),
                    lightknight::Move(lightknight::Square::C6, lightknight::Square::D4),
                    lightknight::Move(lightknight::Square::C6, lightknight::Square::A5),
                    lightknight::Move(lightknight::Square::C6, lightknight::Square::E5),
                }
            },
            { // White, to cover check
                "rnb1kbnr/ppppppqp/8/8/3K2N1/5N2/PPPPPPPP/RNBQ1B1R w kq - 0 1",
                {
                    lightknight::Move(lightknight::Square::F3, lightknight::Square::E5),
                    lightknight::Move(lightknight::Square::G4, lightknight::Square::E5),
                    lightknight::Move(lightknight::Square::G4, lightknight::Square::F6),

                }
            },
            { // Black, pinned
                "rnbq1b1r/ppppppp1/4k3/4n3/6n1/4R2B/PPPPPPP1/RNBQKBN1 b - - 0 1",
                {
                    lightknight::Move(lightknight::Square::B8, lightknight::Square::A6),
                    lightknight::Move(lightknight::Square::B8, lightknight::Square::C6),
                }
            },
            { // White, capture piece that gives check
                "rnbqkb1r/pppppppp/8/3N4/5n2/3K4/PPPPPPPP/RNBQ1B1R w - - 0 1",
                {
                    lightknight::Move(lightknight::Square::D5, lightknight::Square::F4)
                }
            }
        }};

        for (auto &[fen_str, expected_moves] : knight_tests) {
            SECTION(fen_str) {
                lightknight::Board board = lightknight::Board(fen_str);

                std::vector<lightknight::Move> moves;
                lightknight::movegen::GenerateKnightMoves<lightknight::movegen::MoveGenType::kAll>(board, moves);

                TestEqualMoveLists(moves, expected_moves);
            }
        }
    }

    SECTION("Bishop") {
        static const std::array<MoveGenerationTestStruct, 3> bishop_tests = {{
            { // White, taken or not
                "rnbqkbnr/ppppppp1/7p/3B4/8/8/PPPPPPPP/RNBQK1NR w KQkq - 0 1",
                {
                    lightknight::Move(lightknight::Square::D5, lightknight::Square::C6),
                    lightknight::Move(lightknight::Square::D5, lightknight::Square::B7),
                    lightknight::Move(lightknight::Square::D5, lightknight::Square::E6),
                    lightknight::Move(lightknight::Square::D5, lightknight::Square::F7),
                    lightknight::Move(lightknight::Square::D5, lightknight::Square::C4),
                    lightknight::Move(lightknight::Square::D5, lightknight::Square::B3),
                    lightknight::Move(lightknight::Square::D5, lightknight::Square::E4),
                    lightknight::Move(lightknight::Square::D5, lightknight::Square::F3),
                }
            },
            { // Black, cover check and take piece that checks.
                "rnbqkbnr/ppp2pp1/7p/2bb4/8/4R3/PPPPPPPP/1NBQKBNR b Kkq - 0 1",
                {
                    lightknight::Move(lightknight::Square::C5, lightknight::Square::E7),
                    lightknight::Move(lightknight::Square::C5, lightknight::Square::E3),
                    lightknight::Move(lightknight::Square::D5, lightknight::Square::E6),
                    lightknight::Move(lightknight::Square::D5, lightknight::Square::E4),
                    lightknight::Move(lightknight::Square::C8, lightknight::Square::E6),
                    lightknight::Move(lightknight::Square::F8, lightknight::Square::E7),
                }
            },
            { // White, pinned piece
                "rnbqkbnr/ppp2pp1/7p/8/3B4/3K4/PPPP1PPP/1N1Q1BNR w - - 0 1",
                {
                    lightknight::Move(lightknight::Square::F1, lightknight::Square::E2)
                }
            }
        }};

        for (auto &[fen_str, expected_moves] : bishop_tests) {
            SECTION(fen_str) {
                lightknight::Board board = lightknight::Board(fen_str);

                std::vector<lightknight::Move> moves;
                lightknight::movegen::GenerateBishopMoves<lightknight::movegen::MoveGenType::kAll>(board, moves);

                TestEqualMoveLists(moves, expected_moves);
            }
        }
    }

    SECTION("Rook") {
        static const std::array<MoveGenerationTestStruct, 2> rook_tests = {{
            { // White, takes or not, pinned piece.
                "rn1qkbnr/pppppppp/8/b7/5R2/2R5/PPP1PPPP/1NBQKBN1 w - - 0 1",
                {
                    lightknight::Move(lightknight::Square::F4, lightknight::Square::F3),
                    lightknight::Move(lightknight::Square::F4, lightknight::Square::A4),
                    lightknight::Move(lightknight::Square::F4, lightknight::Square::B4),
                    lightknight::Move(lightknight::Square::F4, lightknight::Square::C4),
                    lightknight::Move(lightknight::Square::F4, lightknight::Square::D4),
                    lightknight::Move(lightknight::Square::F4, lightknight::Square::E4),
                    lightknight::Move(lightknight::Square::F4, lightknight::Square::G4),
                    lightknight::Move(lightknight::Square::F4, lightknight::Square::H4),
                    lightknight::Move(lightknight::Square::F4, lightknight::Square::F5),
                    lightknight::Move(lightknight::Square::F4, lightknight::Square::F6),
                    lightknight::Move(lightknight::Square::F4, lightknight::Square::F7),
                }
            },
            { // Black, take piece that gives check.
                "rnbq1bn1/pppppppp/8/3k4/4P2r/P7/1PPPPPP1/RNBQKBNR b KQ - 0 1",
                {
                    lightknight::Move(lightknight::Square::H4, lightknight::Square::E4)
                }
            }
        }};

        for (auto &[fen_str, expected_moves] : rook_tests) {
            SECTION(fen_str) {
                lightknight::Board board = lightknight::Board(fen_str);

                std::vector<lightknight::Move> moves;
                lightknight::movegen::GenerateRookMoves<lightknight::movegen::MoveGenType::kAll>(board, moves);

                TestEqualMoveLists(moves, expected_moves);
            }
        }
    }

    SECTION("Queen") {
        static const std::array<MoveGenerationTestStruct, 2> queen_tests = {{
            { // White, takes or not, pinned piece.
                "rnbq1knr/pppQp1p1/8/8/1q2Qp2/3K4/PP1PPPPP/RN4NR w - - 0 1",
                {
                    lightknight::Move(lightknight::Square::E4, lightknight::Square::D4),
                    lightknight::Move(lightknight::Square::E4, lightknight::Square::C4),
                    lightknight::Move(lightknight::Square::E4, lightknight::Square::B4),
                    lightknight::Move(lightknight::Square::E4, lightknight::Square::F4),
                    lightknight::Move(lightknight::Square::E4, lightknight::Square::E5),
                    lightknight::Move(lightknight::Square::E4, lightknight::Square::E6),
                    lightknight::Move(lightknight::Square::E4, lightknight::Square::E7),
                    lightknight::Move(lightknight::Square::E4, lightknight::Square::E3),
                    lightknight::Move(lightknight::Square::E4, lightknight::Square::F3),
                    lightknight::Move(lightknight::Square::E4, lightknight::Square::F5),
                    lightknight::Move(lightknight::Square::E4, lightknight::Square::G6),
                    lightknight::Move(lightknight::Square::E4, lightknight::Square::H7),
                    lightknight::Move(lightknight::Square::E4, lightknight::Square::D5),
                    lightknight::Move(lightknight::Square::E4, lightknight::Square::C6),
                    lightknight::Move(lightknight::Square::E4, lightknight::Square::B7),
                    lightknight::Move(lightknight::Square::D7, lightknight::Square::D8),
                    lightknight::Move(lightknight::Square::D7, lightknight::Square::D6),
                    lightknight::Move(lightknight::Square::D7, lightknight::Square::D5),
                    lightknight::Move(lightknight::Square::D7, lightknight::Square::D4),
                }
            },
            { // Black, take piece that gives check
                "rnbq2nr/pppQp1p1/8/5k2/1q3p2/3K4/PP1PPPPP/RN4NR b - - 0 1",
                {
                    lightknight::Move(lightknight::Square::D8, lightknight::Square::D7)
                }
            }
        }};

        for (auto &[fen_str, expected_moves] : queen_tests) {
            SECTION(fen_str) {
                lightknight::Board board = lightknight::Board(fen_str);

                std::vector<lightknight::Move> moves;
                lightknight::movegen::GenerateQueenMoves<lightknight::movegen::MoveGenType::kAll>(board, moves);

                TestEqualMoveLists(moves, expected_moves);
            }
        }
    }

    SECTION("King") {
        static const std::array<MoveGenerationTestStruct, 5> king_tests = {{
            { // White, dont move into check / near the other king
                "8/8/6r1/2K5/4k3/8/3n4/8 w - - 0 1",
                {
                    lightknight::Move(lightknight::Square::C5, lightknight::Square::B4),
                    lightknight::Move(lightknight::Square::C5, lightknight::Square::B5),
                }
            },
            { // Black, take attacker, can't take defended attacker
                "8/8/6r1/2K5/4k3/3P1P2/6P1/8 b - - 0 1",
                {
                    lightknight::Move(lightknight::Square::E4, lightknight::Square::E3),
                    lightknight::Move(lightknight::Square::E4, lightknight::Square::D3),
                    lightknight::Move(lightknight::Square::E4, lightknight::Square::E5),
                    lightknight::Move(lightknight::Square::E4, lightknight::Square::F5),
                    lightknight::Move(lightknight::Square::E4, lightknight::Square::F4),
                }
            },
            { // White, castling
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w Kkq - 0 1",
                {
                    lightknight::Move(lightknight::Square::E1, lightknight::Square::D1),
                    lightknight::Move(lightknight::Square::E1, lightknight::Square::F1),
                    lightknight::Move(lightknight::Square::E1, lightknight::Square::G1, lightknight::PromotionPieceType::kKnight, lightknight::MoveType::kCastling),
                }
            },
            { // Black, can't castle bcs of piece attacks or non empty squares.
                "r3k1Nr/8/1N6/8/8/8/PPPPPPPP/R3K2R b Kkq - 0 1",
                {
                    lightknight::Move(lightknight::Square::E8, lightknight::Square::D8),
                    lightknight::Move(lightknight::Square::E8, lightknight::Square::F7),
                    lightknight::Move(lightknight::Square::E8, lightknight::Square::F8),
                }
            },
            { // Black, king in check can't castle
                "r3k2r/4Q3/8/8/8/8/PPPPPPPP/R3K2R b Kkq - 0 1",
                {
                    lightknight::Move(lightknight::Square::E8, lightknight::Square::E7),
                }
            }
        }};

        for (auto &[fen_str, expected_moves] : king_tests) {
            SECTION(fen_str) {
                lightknight::Board board = lightknight::Board(fen_str);

                std::vector<lightknight::Move> moves;
                lightknight::movegen::GenerateKingMoves<lightknight::movegen::MoveGenType::kAll>(board, moves);

                TestEqualMoveLists(moves, expected_moves);
            }
        }
    }
}
