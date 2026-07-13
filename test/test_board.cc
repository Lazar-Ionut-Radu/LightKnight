// test_board.cc
#include <catch2/catch_test_macros.hpp>
#include "board.h"
#include "exceptions.h"
#include "types.h"
#include <array>
#include <vector>

TEST_CASE(
    "Board Initialization Invalid FEN strings",
    "[UnitTest][Board][FEN]"
) {
    static const std::array<std::pair<std::string, std::string>, 22> tests = {
        std::make_pair(std::string("rnbqkbnr/pppppppp/8/8/8/PPPPPPPP/RNBQBNR w KQkq - 0 1"),     std::string("Too few ranks specified")),
        std::make_pair(std::string("rnbqkbnr/pppppppp/8/8/8/8/8/PPPPPPPP/RNBQBNR w KQkq - 0 1"), std::string("Too many ranks specified")),
        std::make_pair(std::string("rnbqkbnr/pppppppp/8/9/9/PPPPPPPppP/RNBQBNR w KQkq - 0 1"),   std::string("Too many files specified, last rank")),
        std::make_pair(std::string("rnbqkbnr/pppppppp/8/9/9/PPPPPPPppP/RNBQBNRr w KQkq - 0 1"),  std::string("Too many files specified")),
        std::make_pair(std::string("rnbqkbnr/pppp/8/8/5/PPPPPPPP/2B3 w KQkq - 0 1"),             std::string("Too few files specified")),
        std::make_pair(std::string("rnbqkbnr//8/8/8/PPPPPPPP/RNBQBNR w KQkq - 0 1"),             std::string("No piece specified on file")),
        std::make_pair(std::string("rnblkbnr/pppppppp/8/8/8/1p6/PrrPPjPP/RNBQWNR w KQkq - 0 1"), std::string("Invalid pieces")),
        std::make_pair(std::string("rnbqkbnr/pppppppp/8/8/8/PPPPPPPP/RNBQBNR KQkq - 0 1"),       std::string("No turn char")),
        std::make_pair(std::string("rnbqkbnr/pppppppp/8/8/8/PPPPPPPP/RNBQBNR wwb KQkq - 0 1"),   std::string("Too many turn chars")),
        std::make_pair(std::string("rnbqkbnr/pppppppp/8/8/8/PPPPPPPP/RNBQBNR t KQkq - 0 1"),     std::string("Invalid turn char")),
        std::make_pair(std::string("rnbqkbnr/pppppppp/8/8/8/PPPPPPPP/RNBQBNR w c - 0 1"),        std::string("Invalid castling char")),
        std::make_pair(std::string("rnbqkbnr/pppppppp/8/8/8/PPPPPPPP/RNBQBNR w KQ- - 0 1"),      std::string("Invalid castling chars with -")),
        std::make_pair(std::string("rnbqkbnr/pppppppp/8/8/8/PPPPPPPP/RNBQBNR w 0 1"),            std::string("No castling chars")),
        std::make_pair(std::string("rnbqkbnr/pppppppp/8/8/8/PPPPPPPP/RNBQBNR w KQkqK - 0 1"),    std::string("Too many castling chars")),
        std::make_pair(std::string("rnbqkbnr/pppppppp/8/8/8/PPPPPPPP/RNBQBNR w KQkq z3 0 1"),    std::string("Wrong en passant file")),
        std::make_pair(std::string("rnbqkbnr/pppppppp/8/8/8/PPPPPPPP/RNBQBNR w - e9 0 1"),       std::string("Wrong en passant rank")),
        std::make_pair(std::string("rnbqkbnr/pppppppp/8/8/8/PPPPPPPP/RNBQBNR w - e28z 0 1"),     std::string("Too many chars en passant")),
        std::make_pair(std::string("rnbqkbnr/pppppppp/8/8/8/PPPPPPPP/RNBQBNR w - a 0 1"),        std::string("Too few chars en passant")),
        std::make_pair(std::string("rnbqkbnr/pppppppp/8/8/8/PPPPPPPP/RNBQBNR w KQkq - plm gcc"), std::string("Invalid halfmove/fullmove")),
        std::make_pair(std::string("8/8/8/8/8/8/8/8 w - -"),                                     std::string("Too few fields specified")),
        std::make_pair(std::string("8/8/8/8/8/8/8/8 w K - 0 1 10"),                              std::string("Too many fields specified")),
        std::make_pair(std::string("bnbnn/153/nn22 ffff as/2.1//"),                              std::string("Junk FEN string")),     
    };

    for (const auto& [fen, description] : tests) {
        SECTION(fen) {
            INFO("Description: " << description);
            REQUIRE_THROWS_AS(lightknight::Board(fen), lightknight::exceptions::FENException);
        }
    }
}

void CheckBoard(const lightknight::Board& actual, const lightknight::Board& expected) {
    std::array<std::string, lightknight::kNumPieces> bitboard_names = {
        "White Pawn Bitboard", "White Knight Bitboard", "White Bishop Bitboard", "White Rook Bitboard",
        "White Queen Bitboard", "White King Bitboard", "Black Pawn Bitboard", "Black Knight Bitboard",
        "Black Bishop Bitboard", "Black Rook Bitboard", "Black Queen Bitboard", "Black King Bitboard",
        "Empty Squares Bitboard"
    };
    for (std::size_t idx=0; idx < lightknight::kNumPieces; ++idx) {
        SECTION(bitboard_names[idx]) {
            INFO("Expected: 0x" << std::hex << expected.piece_bitboards[idx]);
            INFO("Actual: 0x" << std::hex << actual.piece_bitboards[idx]);
            CHECK(actual.piece_bitboards[idx] == expected.piece_bitboards[idx]);
        }
    }

    CHECK(actual.turn == expected.turn);
    CHECK(actual.castling == expected.castling);
    CHECK(actual.en_passant == expected.en_passant);
    CHECK(actual.halfmoves == expected.halfmoves);
    CHECK(actual.fullmoves == expected.fullmoves);
}

TEST_CASE(
    "Board Initialization Valid FEN String",
     "[UnitTest][Board][FEN]"
) {
    static const std::array<std::pair<std::string, lightknight::Board>, 3> tests = {
        std::make_pair(
            std::string("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"),
            lightknight::Board::FromRaw(
                {
                    0x000000000000FF00ULL,
                    0x0000000000000042ULL,
                    0x0000000000000024ULL,
                    0x0000000000000081ULL,
                    0x0000000000000008ULL,
                    0x0000000000000010ULL,
                    0x00FF000000000000ULL,
                    0x4200000000000000ULL,
                    0x2400000000000000ULL,
                    0x8100000000000000ULL,
                    0x0800000000000000ULL,
                    0x1000000000000000ULL,
                    0x0000FFFFFFFF0000ULL
                },
                lightknight::Color::kWhite,
                0b1111,
                -1,
                0,
                1)),
        std::make_pair(
            std::string("r3k2r/pp1q1pp1/2p1pn2/4B2p/2BP4/8/PPP1QPP1/2KR3R b kq - 1 16"),
            lightknight::Board::FromRaw(
                {
                    0x0000000008006700ULL,
                    0x0000000000000000ULL,
                    0x0000001004000000ULL,
                    0x0000000000000088ULL,
                    0x0000000000001000ULL,
                    0x0000000000000004ULL,
                    0x0063148000000000ULL,
                    0x0000200000000000ULL,
                    0x0000000000000000ULL,
                    0x8100000000000000ULL,
                    0x0008000000000000ULL,
                    0x1000000000000000ULL,
                    0x6e94cb6ff3ff8873ULL

                },
                lightknight::Color::kBlack,
                0b1100,
                -1,
                1,
                16)),
        std::make_pair(
            std::string("6k1/5pbp/6p1/2pNp3/P1P1P3/5P2/1P3KPP/8 b - a3 0 30"),
            lightknight::Board::FromRaw(
                {
                    0x000000001520c200ULL,
                    0x0000000800000000ULL,
                    0x0000000000000000ULL,
                    0x0000000000000000ULL,
                    0x0000000000000000ULL,
                    0x0000000000002000ULL,
                    0x00a0401400000000ULL,
                    0x0000000000000000ULL,
                    0x0040000000000000ULL,
                    0x0000000000000000ULL,
                    0x0000000000000000ULL,
                    0x4000000000000000ULL,
                    0xbf1fbfe3eadf1dffULL
                },
                lightknight::Color::kBlack,
                0b0000,
                16,
                0,
                30))
    };

    for (auto& test : tests) {
        const std::string& fen_str = test.first;
        const lightknight::Board& expected_board = test.second;
        lightknight::Board board;
            
        SECTION(fen_str) {    
            REQUIRE_NOTHROW(board.FromFEN(fen_str));
            CheckBoard(board, expected_board);
        }
    }
}

TEST_CASE(
    "MakeMove and UnmakeMove restore the board",
    "[UnitTest][Board][Move]"
) {
    using namespace lightknight;

    struct MoveTestCase {
        const char* name;
        const char* before_fen;
        Move move;
        const char* after_fen;
        Piece expected_captured_piece;
    };

    static const std::array<MoveTestCase, 14> tests = {{
        {
            "Normal quiet move",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            Move(Square::G1, Square::F3),
            "rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq - 1 1",
            Piece::kEmpty
        },
        {
            "White double pawn push",
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            Move(Square::E2, Square::E4),
            "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
            Piece::kEmpty
        },
        {
            "Normal capture",
            "8/8/8/3p4/4P3/8/8/4K2k w - - 7 20",
            Move(Square::E4, Square::D5),
            "8/8/8/3P4/8/8/8/4K2k b - - 0 20",
            Piece::kBlackPawn
        },
        {
            "White en passant",
            "8/8/8/3pP3/8/8/8/4K2k w - d6 12 30",
            Move(
                Square::E5,
                Square::D6,
                PromotionPieceType::kKnight,
                MoveType::kEnPassant
            ),
            "8/8/3P4/8/8/8/8/4K2k b - - 0 30",
            Piece::kBlackPawn
        },
        {
            "Black en passant",
            "4k3/8/8/8/3pP3/8/8/7K b - e3 4 18",
            Move(
                Square::D4,
                Square::E3,
                PromotionPieceType::kKnight,
                MoveType::kEnPassant
            ),
            "4k3/8/8/8/8/4p3/8/7K w - - 0 19",
            Piece::kWhitePawn
        },
        {
            "Promotion without capture",
            "7k/P7/8/8/8/8/8/7K w - - 3 40",
            Move(
                Square::A7,
                Square::A8,
                PromotionPieceType::kQueen,
                MoveType::kPromotion
            ),
            "Q6k/8/8/8/8/8/8/7K b - - 0 40",
            Piece::kEmpty
        },
        {
            "Promotion with capture",
            "1r5k/P7/8/8/8/8/8/7K w - - 3 40",
            Move(
                Square::A7,
                Square::B8,
                PromotionPieceType::kKnight,
                MoveType::kPromotion
            ),
            "1N5k/8/8/8/8/8/8/7K b - - 0 40",
            Piece::kBlackRook
        },
        {
            "White kingside castling",
            "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 5 10",
            Move(
                Square::E1,
                Square::G1,
                PromotionPieceType::kKnight,
                MoveType::kCastling
            ),
            "r3k2r/8/8/8/8/8/8/R4RK1 b kq - 6 10",
            Piece::kEmpty
        },
        {
            "White queenside castling",
            "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 5 10",
            Move(
                Square::E1,
                Square::C1,
                PromotionPieceType::kKnight,
                MoveType::kCastling
            ),
            "r3k2r/8/8/8/8/8/8/2KR3R b kq - 6 10",
            Piece::kEmpty
        },
        {
            "Black kingside castling",
            "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 5 10",
            Move(
                Square::E8,
                Square::G8,
                PromotionPieceType::kKnight,
                MoveType::kCastling
            ),
            "r4rk1/8/8/8/8/8/8/R3K2R w KQ - 6 11",
            Piece::kEmpty
        },
        {
            "Black queenside castling",
            "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 5 10",
            Move(
                Square::E8,
                Square::C8,
                PromotionPieceType::kKnight,
                MoveType::kCastling
            ),
            "2kr3r/8/8/8/8/8/8/R3K2R w KQ - 6 11",
            Piece::kEmpty
        },
        {
            "King move removes both castling rights",
            "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
            Move(Square::E1, Square::F1),
            "r3k2r/8/8/8/8/8/8/R4K1R b kq - 1 1",
            Piece::kEmpty
        },
        {
            "Rook move removes kingside castling right",
            "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",
            Move(Square::H1, Square::H2),
            "r3k2r/8/8/8/8/8/7R/R3K3 b Qkq - 1 1",
            Piece::kEmpty
        },
        {
            "Rook capture removes captured side castling right",
            "r3k2r/8/8/8/8/8/7q/R3K2R b KQkq - 0 1",
            Move(Square::H2, Square::H1),
            "r3k2r/8/8/8/8/8/8/R3K2q w Qkq - 0 2",
            Piece::kWhiteRook
        }
    }};

    for (const MoveTestCase& test : tests) {
        DYNAMIC_SECTION(test.name) {
            const Board original(test.before_fen);
            const Board expected(test.after_fen);

            Board board = original;
            UndoMoveInfo undo{};

            board.MakeMove(test.move, undo);

            INFO("Checking position after MakeMove");
            CheckBoard(board, expected);

            CHECK(undo.captured_piece ==
                  test.expected_captured_piece);

            CHECK(undo.castling == original.castling);
            CHECK(undo.en_passant == original.en_passant);
            CHECK(undo.halfmoves == original.halfmoves);
            CHECK(undo.fullmoves == original.fullmoves);

            board.UnmakeMove(test.move, undo);

            INFO("Checking restoration after UnmakeMove");
            CheckBoard(board, original);
        }
    }
}

TEST_CASE(
    "Make and unmake a 20-ply game sequence",
    "[UnitTest][Board][Move]"
) {
    using namespace lightknight;

    Board board(
        "rnbqkbnr/pppppppp/8/8/8/8/"
        "PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    );

    const Board original = board;

    // Italian Game: 10 full moves, 20 plies.
    const std::array<Move, 20> moves = {
        Move(Square::E2, Square::E4), // 1. e4
        Move(Square::E7, Square::E5), // ... e5

        Move(Square::G1, Square::F3), // 2. Nf3
        Move(Square::B8, Square::C6), // ... Nc6

        Move(Square::F1, Square::C4), // 3. Bc4
        Move(Square::F8, Square::C5), // ... Bc5

        Move(Square::C2, Square::C3), // 4. c3
        Move(Square::G8, Square::F6), // ... Nf6

        Move(Square::D2, Square::D3), // 5. d3
        Move(Square::D7, Square::D6), // ... d6

        Move(
            Square::E1,
            Square::G1,
            PromotionPieceType::kKnight,
            MoveType::kCastling
        ), // 6. O-O

        Move(
            Square::E8,
            Square::G8,
            PromotionPieceType::kKnight,
            MoveType::kCastling
        ), // ... O-O

        Move(Square::F1, Square::E1), // 7. Re1
        Move(Square::A7, Square::A6), // ... a6

        Move(Square::C4, Square::B3), // 8. Bb3
        Move(Square::C5, Square::A7), // ... Ba7

        Move(Square::B1, Square::D2), // 9. Nbd2
        Move(Square::H7, Square::H6), // ... h6

        Move(Square::D2, Square::F1), // 10. Nf1
        Move(Square::F8, Square::E8)  // ... Re8
    };

    std::array<UndoMoveInfo, 20> undo_stack{};
    std::vector<Board> positions;

    positions.reserve(moves.size() + 1);
    positions.push_back(board);

    for (std::size_t ply = 0; ply < moves.size(); ++ply) {
        INFO("Making ply " << ply + 1);
        INFO("Move: " << moves[ply]);

        board.MakeMove(moves[ply], undo_stack[ply]);
        positions.push_back(board);
    }

    const Board expected_final(
        "r1bqr1k1/bpp2pp1/p1np1n1p/4p3/"
        "4P3/1BPP1N2/PP3PPP/R1BQRNK1 w - - 2 11"
    );

    INFO("Checking final position after 20 plies");
    CheckBoard(board, expected_final);

    for (std::size_t ply = moves.size(); ply-- > 0;) {
        INFO("Unmaking ply " << ply + 1);
        INFO("Move: " << moves[ply]);

        board.UnmakeMove(moves[ply], undo_stack[ply]);

        INFO("Checking position restored after unmake");
        CheckBoard(board, positions[ply]);
    }

    INFO("Checking complete restoration of initial position");
    CheckBoard(board, original);
}