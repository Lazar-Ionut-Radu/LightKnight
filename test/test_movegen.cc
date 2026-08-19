// test/test_movegen.cc
#include <catch2/catch_test_macros.hpp>
#include "movegen.h"
#include "types.h"
#include "board.h"
#include <vector>
#include <tuple>
#include <algorithm>
#include <ranges>
#include <string>

using namespace lightknight;

TEST_CASE(
    "Sliding Piece Relevant Occupacy (for magic bitboards generation)",
    "[UnitTest][Move][MoveGen][MagicBitboards]"
) {
    SECTION("Bishop") {
        static const std::array<std::pair<Square, uint64_t>, 10> bishop_tests = {
            std::make_pair(Square::A1, 0x40201008040200ULL),
            std::make_pair(Square::H8, 0x40201008040200ULL),
            std::make_pair(Square::A8, 0x2040810204000ULL),
            std::make_pair(Square::H1, 0x2040810204000ULL),
            std::make_pair(Square::D1, 0x40221400ULL),
            std::make_pair(Square::H2, 0x4081020400000ULL),
            std::make_pair(Square::F8, 0x50080402000000ULL),
            std::make_pair(Square::C6, 0xa000a10204000ULL),
            std::make_pair(Square::E5, 0x44280028440200ULL),
            std::make_pair(Square::B7, 0x40810204000ULL)
        };

        for (auto& [sq, bb] : bishop_tests) {
            CHECK(kBishopBlockersMaskBB[sq] == bb);
        }
    }

    SECTION("Rook") {
        static const std::array<std::pair<Square, uint64_t>, 10> rook_tests = {
            std::make_pair(Square::A1, 0x101010101017eULL),
            std::make_pair(Square::H1, 0x8080808080807eULL),
            std::make_pair(Square::H8, 0x7e80808080808000ULL),
            std::make_pair(Square::H1, 0x8080808080807eULL),
            std::make_pair(Square::A5, 0x1017e01010100ULL),
            std::make_pair(Square::G1, 0x4040404040403eULL),
            std::make_pair(Square::G7, 0x3e404040404000ULL),
            std::make_pair(Square::E4, 0x1010106e101000ULL),
            std::make_pair(Square::D2, 0x8080808087600ULL),
            std::make_pair(Square::C5, 0x4047a04040400ULL)
        };

        for (auto& [sq, bb] : rook_tests) {
            CHECK(kRookBlockersMaskBB[sq] == bb);
        }
    }
}

TEST_CASE(
    "Precomputed Attack Bitboards",
    "[UnitTest][Move][MoveGen][MagicBitboards]"
) {
    SECTION("Pawn") {
        static const std::array<std::tuple<Square, Color, uint64_t>, 8> pawn_tests = {
            std::make_tuple(Square::C2, Color::kWhite, 0xa0000ULL),
            std::make_tuple(Square::H4, Color::kWhite, 0x4000000000ULL),
            std::make_tuple(Square::A8, Color::kWhite, 0ULL),
            std::make_tuple(Square::F1, Color::kWhite, 0x5000ULL),
            std::make_tuple(Square::B6, Color::kBlack, 0x500000000ULL),
            std::make_tuple(Square::H4, Color::kBlack, 0x400000ULL),
            std::make_tuple(Square::E1, Color::kBlack, 0ULL),
            std::make_tuple(Square::C8, Color::kBlack, 0xa000000000000ULL),
        };

        for (auto& [sq, color, bb] : pawn_tests) {
            CHECK(PawnAttackBB(sq, color) == bb);
        }
    };

    SECTION("Knight") {
        static const std::array<std::pair<Square, uint64_t>, 8> knight_tests = {
            std::make_pair(Square::A1, 0x20400ULL),
            std::make_pair(Square::B7, 0x800080500000000ULL),
            std::make_pair(Square::G1, 0xa01000ULL),
            std::make_pair(Square::G3, 0xa0100010a0ULL),
            std::make_pair(Square::F6, 0x5088008850000000ULL),
            std::make_pair(Square::D5, 0x14220022140000ULL),
            std::make_pair(Square::E8, 0x44280000000000ULL),
            std::make_pair(Square::A2, 0x2040004ULL)
        };

        for (auto& [sq, bb] : knight_tests) {
            CHECK(kKnightAttacksBB[sq] == bb);
        }
    }

    SECTION("Bishop") {
        static const std::array<std::tuple<Square, uint64_t, uint64_t>, 13> bishop_tests = {
            std::make_tuple(Square::A1, 0ULL, 0x8040201008040200ULL),
            std::make_tuple(Square::E5, 0ULL, 0x8244280028448201ULL),
            std::make_tuple(Square::G7, 0ULL, 0xa000a01008040201ULL),
            std::make_tuple(Square::F1, 0ULL, 0x10204885000ULL),
            std::make_tuple(Square::D6, 0ULL, 0x2214001422418000ULL),
            std::make_tuple(Square::D4, 0x430100003000008ULL, 0x8041221400142241ULL),
            std::make_tuple(Square::C1, 0x91404040440ULL, 0x804020110a00ULL),
            std::make_tuple(Square::H1, 0xf80020000100bf7fULL, 0x102040810204000ULL),
            std::make_tuple(Square::D2, 0x404000040000ULL, 0x4020140014ULL),
            std::make_tuple(Square::E1, 0x40000ULL, 0x80442800ULL),
            std::make_tuple(Square::E5, 0x280028000000ULL, 0x280028000000ULL),
            std::make_tuple(Square::E5, 0x8200000000008001ULL, 0x8244280028448201ULL),
            std::make_tuple(Square::C5, 0x24040002207ULL, 0x20100a000a112000ULL)
        };

        for (auto& [sq, blockers, bb] : bishop_tests) {
            CHECK(BishopAttackBB(sq, blockers) == bb);
        }

    }

    SECTION("Rook") {
        static const std::array<std::tuple<Square, uint64_t, uint64_t>, 14> rook_tests = {
            std::make_tuple(Square::A1, 0ULL, 0x1010101010101feULL),
            std::make_tuple(Square::B4, 0ULL, 0x2020202fd020202ULL),
            std::make_tuple(Square::G1, 0ULL, 0x40404040404040bfULL),
            std::make_tuple(Square::H2, 0ULL, 0x8080808080807f80ULL),
            std::make_tuple(Square::E7, 0x40307030303ULL, 0x10ef101010101010ULL),
            std::make_tuple(Square::H8, 0x40307030303ULL, 0x7f80808080808080ULL),
            std::make_tuple(Square::F8, 0x1800400000001ULL, 0xdf20202020202020ULL),
            std::make_tuple(Square::B3, 0x200200200ULL, 0x2023d0200ULL),
            std::make_tuple(Square::H1, 0x80080080001cULL, 0x808070ULL),
            std::make_tuple(Square::C5, 0x4023300000000ULL, 0x4041a04040404ULL),
            std::make_tuple(Square::E2, 0x10202000ULL, 0x10102f10ULL),
            std::make_tuple(Square::F1, 0x2004ULL, 0x20dcULL),
            std::make_tuple(Square::B1, 0x205ULL, 0x205ULL),
            std::make_tuple(Square::A8, 0x201000000000000ULL, 0x201000000000000ULL)
        };
        
        for (auto& [sq, blockers, bb] : rook_tests) {
            CHECK(RookAttackBB(sq, blockers) == bb);
        }
    }

    SECTION("Queen") {
        static const std::array<std::tuple<Square, uint64_t, uint64_t>, 13> queen_tests = {
            std::make_tuple(Square::A1, 0ULL, 0x81412111090503feULL),
            std::make_tuple(Square::D1, 0ULL, 0x8080888492a1cf7ULL),
            std::make_tuple(Square::G7, 0ULL, 0xe0bfe05048444241ULL),
            std::make_tuple(Square::D5, 0ULL, 0x492a1cf71c2a4988ULL),
            std::make_tuple(Square::B4, 0x6183c0a10ULL, 0x70d070200ULL),
            std::make_tuple(Square::A8, 0x20202020301c0700ULL, 0x3e03050911010100ULL),
            std::make_tuple(Square::F8, 0x440082000000000ULL, 0xdc70282000000000ULL),
            std::make_tuple(Square::D3, 0x440082000021c00ULL, 0x92a1cf61c00ULL),
            std::make_tuple(Square::F3, 0x200000200000ULL, 0x10224a870df70a8ULL),
            std::make_tuple(Square::G7, 0x40000000000000ULL, 0xe0bfe05048444241ULL),
            std::make_tuple(Square::F5, 0x1000010000030088ULL, 0x24a870df70a82422ULL),
            std::make_tuple(Square::H6, 0x1f3f800000000703ULL, 0xa0c07fc0a0908884ULL),
            std::make_tuple(Square::H1, 0x7e7d7b776f5f3f00ULL, 0x8182848890a0c07fULL),
        };

        for (auto& [sq, blockers, bb] : queen_tests) {
            CHECK(QueenAttackBB(sq, blockers) == bb);
        }
    }

    SECTION("King") {
        static const std::array<std::pair<Square, uint64_t>, 6> king_tests = {
            std::make_pair(Square::H1, 0xc040ULL),
            std::make_pair(Square::H3, 0xc040c000ULL),
            std::make_pair(Square::C6, 0xe0a0e00000000ULL),
            std::make_pair(Square::G8, 0xa0e0000000000000ULL),
            std::make_pair(Square::F3, 0x70507000ULL),
            std::make_pair(Square::A8, 0x203000000000000ULL),
        };

        for (auto& [sq, bb] : king_tests) {
            CHECK(kKingAttacksBB[sq] == bb);
        }
    }
}

void TestEqualMoveLists(std::vector<Move> actual_moves,
                        std::vector<Move> expected_moves)
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
    std::string fen;
    std::vector<Move> expected_moves;
};


TEST_CASE(
    "Generate All Moves",
    "[ComponentTest][Move][MoveGen]"
) {
    SECTION("Pawn Moves") {
        const std::vector<MoveGenerationTestStruct> pawn_tests = {
            { // White pawn pushes, 1 or 2 squares, no pins.
                "rnbqkbnr/pppppppp/8/4P3/1P3P2/8/P1PP2PP/RNBQKBNR w KQkq - 0 1",
                {
                    Move(Square::A2,Square::A3),
                    Move(Square::A2,Square::A4),
                    Move(Square::B4,Square::B5),
                    Move(Square::C2,Square::C3),
                    Move(Square::C2,Square::C4),
                    Move(Square::D2,Square::D3),
                    Move(Square::D2,Square::D4),
                    Move(Square::E5,Square::E6),
                    Move(Square::F4,Square::F5),
                    Move(Square::G2,Square::G3),
                    Move(Square::G2,Square::G4),
                    Move(Square::H2,Square::H3),
                    Move(Square::H2,Square::H4)
                }
            },
            { // Black, pawn pushes, 1 or 2 squares, no pins
                "rnbqkbnr/2p4p/3p4/p7/1p6/8/PPPPPPPR/RNBQKBN1 b kq - 0 1",
                {
                    Move(Square::A5, Square::A4),
                    Move(Square::B4, Square::B3),
                    Move(Square::C7, Square::C6),
                    Move(Square::C7, Square::C5),
                    Move(Square::D6, Square::D5),
                    Move(Square::H7, Square::H6),
                    Move(Square::H7, Square::H5),
                }
            },
            { // White, pawn promotions, captures or not, pinned or not to the king.
                "1qn1kb2/P1ppp1P1/7K/8/8/8/3Q3R/RNB5 w - - 0 1",
                {
                    Move(Square::A7, Square::A8,  PromotionPieceType::kKnight, MoveType::kPromotion),
                    Move(Square::A7, Square::A8,  PromotionPieceType::kBishop, MoveType::kPromotion),
                    Move(Square::A7, Square::A8,  PromotionPieceType::kRook, MoveType::kPromotion),
                    Move(Square::A7, Square::A8,  PromotionPieceType::kQueen, MoveType::kPromotion),
                    Move(Square::A7, Square::B8,  PromotionPieceType::kKnight, MoveType::kPromotion),
                    Move(Square::A7, Square::B8,  PromotionPieceType::kBishop, MoveType::kPromotion),
                    Move(Square::A7, Square::B8,  PromotionPieceType::kQueen, MoveType::kPromotion),
                    Move(Square::A7, Square::B8,  PromotionPieceType::kRook, MoveType::kPromotion),
                    Move(Square::G7, Square::F8,  PromotionPieceType::kKnight, MoveType::kPromotion),
                    Move(Square::G7, Square::F8,  PromotionPieceType::kBishop, MoveType::kPromotion),
                    Move(Square::G7, Square::F8,  PromotionPieceType::kRook, MoveType::kPromotion),
                    Move(Square::G7, Square::F8,  PromotionPieceType::kQueen, MoveType::kPromotion),
                }
            },
            { // Black, pawn promotions, captures or not, pinned or not to the king.
                "8/P5P1/7K/8/8/8/1p2Q1pk/B6N b - - 0 1",
                {
                    Move(Square::B2, Square::B1, PromotionPieceType::kKnight, MoveType::kPromotion),
                    Move(Square::B2, Square::B1, PromotionPieceType::kBishop, MoveType::kPromotion),
                    Move(Square::B2, Square::B1, PromotionPieceType::kRook, MoveType::kPromotion),
                    Move(Square::B2, Square::B1, PromotionPieceType::kQueen, MoveType::kPromotion),
                    Move(Square::B2, Square::A1, PromotionPieceType::kKnight, MoveType::kPromotion),
                    Move(Square::B2, Square::A1, PromotionPieceType::kBishop, MoveType::kPromotion),
                    Move(Square::B2, Square::A1, PromotionPieceType::kRook, MoveType::kPromotion),
                    Move(Square::B2, Square::A1, PromotionPieceType::kQueen, MoveType::kPromotion),
                }
            },
            { // White, pawn captures, pinned or not.
                "r3k1nr/pbppp2p/8/3nqpp1/4PP1b/6P1/7P/RNBQKBNR w KQkq - 0 1",
                {
                    Move(Square::F4, Square::E5),
                    Move(Square::F4, Square::G5),
                    Move(Square::G3, Square::H4),
                    Move(Square::H2, Square::H3)
                }
            },
            { // Black, pawn captures, pinned or not.
                "r3k1nr/1b2p2p/2pR2p1/1N2Rp1Q/B3PP1b/6P1/7P/RNBQKBN1 b - - 0 1",
                {
                    Move(Square::C6, Square::B5),
                    Move(Square::C6, Square::C5),
                    Move(Square::E7, Square::E6),
                    Move(Square::G6, Square::H5),
                    Move(Square::F5, Square::E4),
                    Move(Square::H7, Square::H6)
                }
            },
            { // White en passant
                "rnbqkbnr/ppp1pppp/8/2Pp4/8/8/8/RNBQKBNR w KQkq d6 0 1",
                {
                    Move(Square::C5,Square::C6),
                    Move(Square::C5,Square::D6, PromotionPieceType::kKnight, MoveType::kEnPassant),
                }
            },
            { // Black en passant 
                "nqrkbbnr/8/8/8/6Pp/8/PPPPPP1P/NQRKBBNR b - g3 0 1",
                {
                    Move(Square::H4, Square::G3, PromotionPieceType::kKnight, MoveType::kEnPassant),
                    Move(Square::H4, Square::H3),
                }
            },
            { // White, en passant impossible bcs pin (horizontally, xray through 2 pawns)
                "1nbk1bn1/2p5/8/q3pP1K/8/8/8/RNB2BNR w - e6 0 1",
                {
                    Move(Square::F5, Square::F6)
                }
            },
            { // Black, en passant impossible bcs pinned (vertically)
                "rnb1Kbnr/4p3/4R3/8/4pP2/8/8/4k3 b - f3 0 1",
                {
                    Move(Square::E4, Square::E3)
                }
            },
            { // White, to block check.
                "rnb1kbnr/pppppppp/8/8/4K2q/8/PPPPPPPP/RNBQ1BNR w kq - 0 1",
                {
                    Move(Square::F2, Square::F4),
                    Move(Square::G2, Square::G4)
                }
            },
            { // Black, to block check.
                "rnbqkbnr/ppp1pppp/8/8/B7/8/PPPPPPPP/RN1QKBNR b kq - 0 1",
                {
                    Move(Square::C7, Square::C6),
                    Move(Square::B7, Square::B5)
                }
            }
        };

        for (auto &[fen_str, expected_moves] : pawn_tests) {
            SECTION(fen_str) {
                Board board = Board(fen_str);

                std::vector<Move> moves;
                movegen::GeneratePawnMoves<movegen::MoveGenType::kAll>(board, moves);

                TestEqualMoveLists(moves, expected_moves);
            }
        }
    }

    SECTION("Knight") {
        const std::vector<MoveGenerationTestStruct> knight_tests = {
            { // White, takes or not
                "rnbqkbnr/ppppppp1/2N5/R3P2p/3P4/8/1PPB1PPP/1N1QKB1R w Kkq - 0 1",
                {
                    Move(Square::B1, Square::A3),
                    Move(Square::B1, Square::C3),
                    Move(Square::C6, Square::B4),
                    Move(Square::C6, Square::A7),
                    Move(Square::C6, Square::B8),
                    Move(Square::C6, Square::D8),
                    Move(Square::C6, Square::E7),
                }
            },
            { // Black, taken or not
                "rnbqkb1r/ppppppp1/2n5/R3P2p/3P4/8/1PPB1PPP/1N1QKB1R b Kkq - 0 1",
                {
                    Move(Square::B8, Square::A6),
                    Move(Square::C6, Square::B4),
                    Move(Square::C6, Square::D4),
                    Move(Square::C6, Square::A5),
                    Move(Square::C6, Square::E5),
                }
            },
            { // White, to cover check
                "rnb1kbnr/ppppppqp/8/8/3K2N1/5N2/PPPPPPPP/RNBQ1B1R w kq - 0 1",
                {
                    Move(Square::F3, Square::E5),
                    Move(Square::G4, Square::E5),
                    Move(Square::G4, Square::F6),

                }
            },
            { // Black, pinned
                "rnbq1b1r/ppppppp1/4k3/4n3/6n1/4R2B/PPPPPPP1/RNBQKBN1 b - - 0 1",
                {
                    Move(Square::B8, Square::A6),
                    Move(Square::B8, Square::C6),
                }
            },
            { // White, capture piece that gives check
                "rnbqkb1r/pppppppp/8/3N4/5n2/3K4/PPPPPPPP/RNBQ1B1R w - - 0 1",
                {
                    Move(Square::D5, Square::F4)
                }
            }
        };

        for (auto &[fen_str, expected_moves] : knight_tests) {
            SECTION(fen_str) {
                Board board = Board(fen_str);

                std::vector<Move> moves;
                movegen::GenerateKnightMoves<movegen::MoveGenType::kAll>(board, moves);

                TestEqualMoveLists(moves, expected_moves);
            }
        }
    }

    SECTION("Bishop") {
        const std::vector<MoveGenerationTestStruct> bishop_tests = {
            { // White, taken or not
                "rnbqkbnr/ppppppp1/7p/3B4/8/8/PPPPPPPP/RNBQK1NR w KQkq - 0 1",
                {
                    Move(Square::D5, Square::C6),
                    Move(Square::D5, Square::B7),
                    Move(Square::D5, Square::E6),
                    Move(Square::D5, Square::F7),
                    Move(Square::D5, Square::C4),
                    Move(Square::D5, Square::B3),
                    Move(Square::D5, Square::E4),
                    Move(Square::D5, Square::F3),
                }
            },
            { // Black, cover check and take piece that checks.
                "rnbqkbnr/ppp2pp1/7p/2bb4/8/4R3/PPPPPPPP/1NBQKBNR b Kkq - 0 1",
                {
                    Move(Square::C5, Square::E7),
                    Move(Square::C5, Square::E3),
                    Move(Square::D5, Square::E6),
                    Move(Square::D5, Square::E4),
                    Move(Square::C8, Square::E6),
                    Move(Square::F8, Square::E7),
                }
            },
            { // White, pinned piece
                "rnbqkbnr/ppp2pp1/7p/8/3B4/3K4/PPPP1PPP/1N1Q1BNR w - - 0 1",
                {
                    Move(Square::F1, Square::E2)
                }
            }
        };

        for (auto &[fen_str, expected_moves] : bishop_tests) {
            SECTION(fen_str) {
                Board board = Board(fen_str);

                std::vector<Move> moves;
                movegen::GenerateBishopMoves<movegen::MoveGenType::kAll>(board, moves);

                TestEqualMoveLists(moves, expected_moves);
            }
        }
    }

    SECTION("Rook") {
        const std::vector<MoveGenerationTestStruct> rook_tests = {
            { // White, takes or not, pinned piece.
                "rn1qkbnr/pppppppp/8/b7/5R2/2R5/PPP1PPPP/1NBQKBN1 w - - 0 1",
                {
                    Move(Square::F4, Square::F3),
                    Move(Square::F4, Square::A4),
                    Move(Square::F4, Square::B4),
                    Move(Square::F4, Square::C4),
                    Move(Square::F4, Square::D4),
                    Move(Square::F4, Square::E4),
                    Move(Square::F4, Square::G4),
                    Move(Square::F4, Square::H4),
                    Move(Square::F4, Square::F5),
                    Move(Square::F4, Square::F6),
                    Move(Square::F4, Square::F7),
                }
            },
            { // Black, take piece that gives check.
                "rnbq1bn1/pppppppp/8/3k4/4P2r/P7/1PPPPPP1/RNBQKBNR b KQ - 0 1",
                {
                    Move(Square::H4, Square::E4)
                }
            }
        };

        for (auto &[fen_str, expected_moves] : rook_tests) {
            SECTION(fen_str) {
                Board board = Board(fen_str);

                std::vector<Move> moves;
                movegen::GenerateRookMoves<movegen::MoveGenType::kAll>(board, moves);

                TestEqualMoveLists(moves, expected_moves);
            }
        }
    }

    SECTION("Queen") {
        const std::vector<MoveGenerationTestStruct> queen_tests = {
            { // White, takes or not, pinned piece.
                "rnbq1knr/pppQp1p1/8/8/1q2Qp2/3K4/PP1PPPPP/RN4NR w - - 0 1",
                {
                    Move(Square::E4, Square::D4),
                    Move(Square::E4, Square::C4),
                    Move(Square::E4, Square::B4),
                    Move(Square::E4, Square::F4),
                    Move(Square::E4, Square::E5),
                    Move(Square::E4, Square::E6),
                    Move(Square::E4, Square::E7),
                    Move(Square::E4, Square::E3),
                    Move(Square::E4, Square::F3),
                    Move(Square::E4, Square::F5),
                    Move(Square::E4, Square::G6),
                    Move(Square::E4, Square::H7),
                    Move(Square::E4, Square::D5),
                    Move(Square::E4, Square::C6),
                    Move(Square::E4, Square::B7),
                    Move(Square::D7, Square::D8),
                    Move(Square::D7, Square::D6),
                    Move(Square::D7, Square::D5),
                    Move(Square::D7, Square::D4),
                }
            },
            { // Black, take piece that gives check
                "rnbq2nr/pppQp1p1/8/5k2/1q3p2/3K4/PP1PPPPP/RN4NR b - - 0 1",
                {
                    Move(Square::D8, Square::D7)
                }
            }
        };

        for (auto &[fen_str, expected_moves] : queen_tests) {
            SECTION(fen_str) {
                Board board = Board(fen_str);

                std::vector<Move> moves;
                movegen::GenerateQueenMoves<movegen::MoveGenType::kAll>(board, moves);

                TestEqualMoveLists(moves, expected_moves);
            }
        }
    }

    SECTION("King") {
        const std::vector<MoveGenerationTestStruct> king_tests = {
            { // White, dont move into check / near the other king
                "8/8/6r1/2K5/4k3/8/3n4/8 w - - 0 1",
                {
                    Move(Square::C5, Square::B4),
                    Move(Square::C5, Square::B5),
                }
            },
            { // Black, take attacker, can't take defended attacker
                "8/8/6r1/2K5/4k3/3P1P2/6P1/8 b - - 0 1",
                {
                    Move(Square::E4, Square::E3),
                    Move(Square::E4, Square::D3),
                    Move(Square::E4, Square::E5),
                    Move(Square::E4, Square::F5),
                    Move(Square::E4, Square::F4),
                }
            },
            { // White, castling
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w Kkq - 0 1",
                {
                    Move(Square::E1, Square::D1),
                    Move(Square::E1, Square::F1),
                    Move(Square::E1, Square::G1, PromotionPieceType::kKnight, MoveType::kCastling),
                }
            },
            { // Black, can't castle bcs of piece attacks or non empty squares.
                "r3k1Nr/8/1N6/8/8/8/PPPPPPPP/R3K2R b Kkq - 0 1",
                {
                    Move(Square::E8, Square::D8),
                    Move(Square::E8, Square::F7),
                    Move(Square::E8, Square::F8),
                }
            },
            { // Black, king in check can't castle
                "r3k2r/4Q3/8/8/8/8/PPPPPPPP/R3K2R b Kkq - 0 1",
                {
                    Move(Square::E8, Square::E7),
                }
            }
        };

        for (auto &[fen_str, expected_moves] : king_tests) {
            SECTION(fen_str) {
                Board board = Board(fen_str);

                std::vector<Move> moves;
                movegen::GenerateKingMoves<movegen::MoveGenType::kAll>(board, moves);

                TestEqualMoveLists(moves, expected_moves);
            }
        }
    }
}

TEST_CASE(
    "Generate Capture Moves",
    "[ComponentTest][Move][MoveGen]"
) {
    const std::vector<MoveGenerationTestStruct> tests = {
        {
            "r1b1k2r/ppp1nppp/2n5/b3P2q/2B4N/1Q1p1N1P/PP3PP1/R1B2RK1 w kq - 0 13",
            {
                Move(Square::B3, Square::B7),
                Move(Square::B3, Square::D3),
                Move(Square::C4, Square::D3),
                Move(Square::C4, Square::F7)
            }
        },
        {
            "r3k2r/1pp1bppp/p2qbn2/2p5/4PN2/4BP2/PPP3PP/RN1Q1RK1 b kq - 3 12",
            {
                Move(Square::D6, Square::D1),
                Move(Square::D6, Square::F4),
                Move(Square::E6, Square::A2),
                Move(Square::F6, Square::E4)
            }
        },
        { // Check, no capture move
            "2R3k1/Qr3pp1/4pn1p/4N3/Pq6/6P1/1P2PP1P/6K1 b - - 2 30",
            {}
        },
        { // Check, captures possible.
            "4r1k1/R1p2bp1/2p4p/2PnPp2/1PP2P2/4K3/3B3P/7B w - - 0 1",
            {
                Move(Square::C4, Square::D5),
                Move(Square::H1, Square::D5)
            }
        },
        { // Double check.
            "4r1k1/R1p2bp1/7p/2b1Pp2/1PP2P2/4K3/3B2nP/7B w - - 0 1",
            {}
        },
        { // Starting position
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            {}
        }
    };

    for (auto &[fen_str, expected_moves] : tests) {
        SECTION(fen_str) {
            Board board = Board(fen_str);

            std::vector<Move> moves;
            movegen::GenerateMoves<movegen::MoveGenType::kCapture>(board, moves);

            TestEqualMoveLists(moves, expected_moves);
        }
    }
}

TEST_CASE(
    "Generate Tactical Moves",
    "[ComponentTest][Move][MoveGen]"
) {
    const std::vector<MoveGenerationTestStruct> tests = {
        { // With impossible promotion cause in check.
            "4k3/2P5/3Kr2p/2BB4/8/8/8/8 w - - 0 76",
            {
                Move(Square::D5, Square::E6),
                Move(Square::D6, Square::E6)
            }
        },
        { // Starting position.
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            {}
        },
        { // Capture and promotions.
            "8/6P1/3k4/6BK/8/4b3/pp6/6r1 b - - 0 1",
            {
                Move(Square::A2, Square::A1, PromotionPieceType::kKnight, MoveType::kPromotion),
                Move(Square::A2, Square::A1, PromotionPieceType::kBishop, MoveType::kPromotion),
                Move(Square::A2, Square::A1, PromotionPieceType::kRook, MoveType::kPromotion),
                Move(Square::A2, Square::A1, PromotionPieceType::kQueen, MoveType::kPromotion),
                Move(Square::B2, Square::B1, PromotionPieceType::kKnight, MoveType::kPromotion),
                Move(Square::B2, Square::B1, PromotionPieceType::kBishop, MoveType::kPromotion),
                Move(Square::B2, Square::B1, PromotionPieceType::kRook, MoveType::kPromotion),
                Move(Square::B2, Square::B1, PromotionPieceType::kQueen, MoveType::kPromotion),
                Move(Square::E3, Square::G5),
                Move(Square::G1, Square::G5)
            }   
        },
        { // Promotion to get out of check.
            "8/6P1/8/7K/8/4k3/pp6/2B3r1 b - - 0 1",
            {
                Move(Square::B2, Square::C1, PromotionPieceType::kKnight, MoveType::kPromotion),
                Move(Square::B2, Square::C1, PromotionPieceType::kBishop, MoveType::kPromotion),
                Move(Square::B2, Square::C1, PromotionPieceType::kRook, MoveType::kPromotion),
                Move(Square::B2, Square::C1, PromotionPieceType::kQueen, MoveType::kPromotion),
                Move(Square::G1, Square::C1)
            }
        }
    };

    for (auto &[fen_str, expected_moves] : tests) {
        SECTION(fen_str) {
            Board board = Board(fen_str);

            std::vector<Move> moves;
            movegen::GenerateMoves<movegen::MoveGenType::kTactical>(board, moves);

            TestEqualMoveLists(moves, expected_moves);
        }
    }
}

TEST_CASE(
    "Generate Quiet Moves",
    "[ComponentTest][Move][MoveGen]"
) {
    const std::vector<MoveGenerationTestStruct> tests = {
        { // In check, don't capture
            "8/2p2pk1/6p1/1q1Rp1n1/2Q1P3/1KP5/8/8 w - - 0 1",
            {
                Move(Square::B3, Square::A3),
                Move(Square::B3, Square::A2),
                Move(Square::B3, Square::C2),
                Move(Square::C4, Square::B4)
            }
        },
        { // Don't promote.
            "8/1R5P/1p2rkp1/p2K2p1/P4P2/1P6/8/2b5 w - - 0 1",
            {
                Move(Square::B7, Square::B8),
                Move(Square::B7, Square::A7),
                Move(Square::B7, Square::C7),
                Move(Square::B7, Square::D7),
                Move(Square::B7, Square::E7),
                Move(Square::B7, Square::F7),
                Move(Square::B7, Square::G7),
                Move(Square::B3, Square::B4),
                Move(Square::F4, Square::F5),
                Move(Square::D5, Square::C4),
                Move(Square::D5, Square::D4)
            }
        }
    };

    for (auto &[fen_str, expected_moves] : tests) {
        SECTION(fen_str) {
            Board board = Board(fen_str);

            std::vector<Move> moves;
            movegen::GenerateMoves<movegen::MoveGenType::kQuiet>(board, moves);

            TestEqualMoveLists(moves, expected_moves);
        }
    }
}