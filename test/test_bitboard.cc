// test/test_bitboard.cc
#include <catch2/catch_test_macros.hpp>
#include <types.h>
#include <vector>

using namespace lightknight;

TEST_CASE("Bitboard Directional Movement", "[UnitTest][Bitboard]") {
    SECTION ("North") {
        const std::vector<std::pair<uint64_t, uint64_t>> tests_north = {
            std::make_pair(0x80000000000ULL, 0x8000000000000ULL),
            std::make_pair(0x20000ULL, 0x2000000ULL),
            std::make_pair(0x8000000000000000ULL, 0ULL)
        };

        for (const auto& [start, target] : tests_north)
            REQUIRE(lightknight::North(start) == target);
    }

    SECTION ("South") {
        const std::vector<std::pair<uint64_t, uint64_t>> tests_south = {
            std::make_pair(0x8000000000000000ULL, 0x80000000000000ULL),
            std::make_pair(0x200000ULL, 0x2000ULL),
            std::make_pair(0x4ULL, 0ULL)
        };

        for (const auto& [start, target] : tests_south)
            REQUIRE(lightknight::South(start) == target);
    }

    SECTION ("East") {
        const std::vector<std::pair<uint64_t, uint64_t>> tests_east = {
            std::make_pair(0x10ULL, 0x20ULL),
            std::make_pair(0x8000000ULL, 0x10000000ULL),
            std::make_pair(0x8000ULL, 0ULL)
        };

        for (const auto& [start, target] : tests_east)
            REQUIRE(lightknight::East(start) == target);
    }

    SECTION ("West") {
        const std::vector<std::pair<uint64_t, uint64_t>> tests_west = {
            std::make_pair(0x8000ULL, 0x4000ULL),
            std::make_pair(0x200000000ULL, 0x100000000ULL),
            std::make_pair(0x100000000ULL, 0ULL)
        };

        for (const auto& [start, target] : tests_west)
            REQUIRE(lightknight::West(start) == target);
    }

    SECTION("North-West") {
        const std::vector<std::pair<uint64_t, uint64_t>> tests_northwest = {
            std::make_pair(0x100000000ULL, 0ULL),
            std::make_pair(0x8000000000000000ULL, 0ULL),
            std::make_pair(0x100000000000000ULL, 0ULL),
            std::make_pair(0x8ULL, 0x400ULL)
        };

        for (const auto& [start, target] : tests_northwest)
            REQUIRE(lightknight::NorthWest(start) == target);
    }

    SECTION("North-East") {
        const std::vector<std::pair<uint64_t, uint64_t>> tests_northeast = {
            std::make_pair(0x200ULL, 0x40000ULL),
            std::make_pair(0x8000ULL, 0ULL),
            std::make_pair(0x8000000000000000ULL, 0ULL),
            std::make_pair(0x200000000000000ULL, 0ULL)
        };

        for (const auto& [start, target] : tests_northeast)
            REQUIRE(lightknight::NorthEast(start) == target);
    }

    SECTION("South-West") {
        const std::vector<std::pair<uint64_t, uint64_t>> tests_southwest = {
            std::make_pair(0x1ULL, 0ULL),
            std::make_pair(0x1000000ULL, 0ULL),
            std::make_pair(0x4ULL, 0ULL),
            std::make_pair(0x40000ULL, 0x200ULL)
        };

        for (const auto& [start, target] : tests_southwest)
            REQUIRE(lightknight::SouthWest(start) == target);
    }

    SECTION("South-East") {
        const std::vector<std::pair<uint64_t, uint64_t>> tests_southeast = {
            std::make_pair(0x400ULL, 0x8ULL),
            std::make_pair(0x8ULL, 0ULL),
            std::make_pair(0x80ULL, 0ULL),
            std::make_pair(0x800000ULL, 0ULL)
        };
        
        for (const auto& [start, target] : tests_southeast)
            REQUIRE(lightknight::SouthEast(start) == target);
    }
}

struct LSBTestStruct {
    uint64_t lsb;
    lightknight::Square sq;
    size_t set_bits_cnt;
};

TEST_CASE("Bitboard Least Significant Bit", "[UnitTest][Bitboard]") {
    const std::vector<std::pair<uint64_t, LSBTestStruct>> tests = {
        std::make_pair(0x1ULL, LSBTestStruct{0x1ULL, lightknight::Square::A1, 1}),
        std::make_pair(0x4000000ULL, LSBTestStruct{0x4000000ULL, lightknight::Square::C4, 1}),
        std::make_pair(0xdc000004800002ULL, LSBTestStruct{0x2ULL, lightknight::Square::B1, 8}),
        std::make_pair(0xffffffffffffffffULL, LSBTestStruct{0x1ULL, lightknight::Square::A1, 64}),
        std::make_pair(0x8100008000000000ULL, LSBTestStruct{0x8000000000ULL, lightknight::Square::H5, 3})
    };
    
    SECTION ("LSB") {
        for (auto& [bitboard, lsb_test_struct] : tests) {
            INFO("Bitboard: 0x" << std::hex << bitboard);
            REQUIRE(lightknight::LSB(bitboard) == lsb_test_struct.lsb);
        }
    }

    SECTION ("LSBSquare") {
        for (auto& [bitboard, lsb_test_struct] : tests) {
            INFO("Bitboard: 0x" << std::hex << bitboard);
            REQUIRE(lightknight::LSBSquare(bitboard) == lsb_test_struct.sq);
        }
    }

    SECTION ("SetBitsCount") {
        for (auto& [bitboard, lsb_test_struct] : tests) {
            INFO("Bitboard: 0x" << std::hex << bitboard);
            REQUIRE(lightknight::SetBitsCount(bitboard) == lsb_test_struct.set_bits_cnt);
       }
    }
}

TEST_CASE("Square-Bitboard Conversions", "[UnitTest][Bitboard][Square]") {
    const std::vector<std::pair<lightknight::Square, uint64_t>> tests = {
        std::make_pair(lightknight::Square::A1, 0x1ULL),
        std::make_pair(lightknight::Square::G1, 0x40ULL),
        std::make_pair(lightknight::Square::E3, 0x100000ULL),
        std::make_pair(lightknight::Square::B7, 0x2000000000000ULL),
        std::make_pair(lightknight::Square::H8, 0x8000000000000000ULL)
    };

    for (auto& [square, bitboard] : tests) {
        INFO("Square " << square);
        INFO("Bitboard: 0x" << std::hex << bitboard);
        REQUIRE(lightknight::SquareToBitboard(square) == bitboard);
        REQUIRE(lightknight::BitboardToSquare(bitboard) == square);
    }
}

struct SquareTestStruct {
    int rank;
    int file;
};

TEST_CASE("Square Rank/File", "[UnitTest][Square]") {
    const std::vector<std::pair<lightknight::Square, SquareTestStruct>> tests = {
        std::make_pair(lightknight::Square::A1, SquareTestStruct{0, 0}),
        std::make_pair(lightknight::Square::H5, SquareTestStruct{4, 7}),
        std::make_pair(lightknight::Square::F8, SquareTestStruct{7, 5}),
        std::make_pair(lightknight::Square::C3, SquareTestStruct{2, 2}),
        std::make_pair(lightknight::Square::D5, SquareTestStruct{4, 3}),
        std::make_pair(lightknight::Square::B6, SquareTestStruct{5, 1}),
    };

    for (auto& [square, square_test_struct] : tests) {
        INFO("Square " << square);
        REQUIRE(lightknight::Rank(square) == square_test_struct.rank);
        REQUIRE(lightknight::File(square) == square_test_struct.file);
    }
}

struct BBComputeTests {
    uint64_t bb;
    Color color;
    uint64_t expected_bb;
};

TEST_CASE("Pawn helper bitboards", "[UnitTest][Bitboard]") {
    SECTION("Forward/Backward Fill BB") {
        BBComputeTests tests_f[] = {
            { 
                SquareToBitboard(Square::D4),
                Color::kWhite,
                0x808080800000000ull
            },
            {
                SquareToBitboard(Square::F6),
                Color::kBlack,
                0x2020202020ull
            },
        };
        for (const auto& test : tests_f)
            REQUIRE(ForwardFillBB(test.bb, test.color) == test.expected_bb);
    
        BBComputeTests tests_b[] = {
            { 
                SquareToBitboard(Square::D4),
                Color::kWhite,
                0x80808ull
            },
            {
                SquareToBitboard(Square::F6),
                Color::kBlack,
                0x2020000000000000ull
            },
        };
        for (const auto& test : tests_b)
            REQUIRE(BackwardFillBB(test.bb, test.color) == test.expected_bb);
    }

    SECTION("Forward/Backward Adjacent Fill BB") {
        BBComputeTests tests_fadj[] = {
            {
                SquareToBitboard(Square::B3),
                Color::kWhite,
                0x505050505000000ull
            },
            {
                SquareToBitboard(Square::A5),
                Color::kWhite,
                0x202020000000000ull
            },
            {
                SquareToBitboard(Square::H3),
                Color::kWhite,
                0x4040404040000000ull
            },
            {
                SquareToBitboard(Square::E4),
                Color::kBlack,
                0x282828ull
            },
            {
                SquareToBitboard(Square::H5),
                Color::kBlack,
                0x40404040ull
            },
            {
                SquareToBitboard(Square::A2),
                Color::kBlack,
                0x2ull
            }
        };
        for (const auto& test : tests_fadj)
            REQUIRE(ForwardAdjacentFillBB(test.bb, test.color) == test.expected_bb);
        
        BBComputeTests tests_badj[] = {
            {
                SquareToBitboard(Square::B3),
                Color::kWhite,
                0x505ull
            },
            {
                SquareToBitboard(Square::A5),
                Color::kWhite,
                0x2020202ull
            },
            {
                SquareToBitboard(Square::H3),
                Color::kWhite,
                0x4040ull
            },
            {
                SquareToBitboard(Square::E4),
                Color::kBlack,
                0x2828282800000000ull
            },
            {
                SquareToBitboard(Square::H5),
                Color::kBlack,
                0x4040400000000000ull
            },
            {
                SquareToBitboard(Square::A2),
                Color::kBlack,
                0x202020202020000ull
            }
        };
        for (const auto& test : tests_badj)
            REQUIRE(BackwardAdjacentFillBB(test.bb, test.color) == test.expected_bb);
    }

    SECTION ("Forward/Backward Three Fill BB") {
        BBComputeTests tests_f3[] = {
            {
                SquareToBitboard(Square::B3),
                Color::kWhite,
                0x707070707000000ull
            },
            {
                SquareToBitboard(Square::A5),
                Color::kWhite,
                0x303030000000000ull
            },
            {
                SquareToBitboard(Square::H3),
                Color::kWhite,
                0xc0c0c0c0c0000000ull
            },
            {
                SquareToBitboard(Square::E4),
                Color::kBlack,
                0x383838ull
            },
            {
                SquareToBitboard(Square::H5),
                Color::kBlack,
                0xc0c0c0c0ull
            },
            {
                SquareToBitboard(Square::A2),
                Color::kBlack,
                0x3ull
            }
        };
        for (const auto& test : tests_f3)
            REQUIRE(ForwardThreeFillBB(test.bb, test.color) == test.expected_bb);
        
        BBComputeTests tests_b3[] = {
            {
                SquareToBitboard(Square::B3),
                Color::kWhite,
                0x707ull
            },
            {
                SquareToBitboard(Square::A5),
                Color::kWhite,
                0x3030303ull
            },
            {
                SquareToBitboard(Square::H3),
                Color::kWhite,
                0xc0c0ull
            },
            {
                SquareToBitboard(Square::E4),
                Color::kBlack,
                0x3838383800000000ull
            },
            {
                SquareToBitboard(Square::H5),
                Color::kBlack,
                0xc0c0c00000000000ull
            },
            {
                SquareToBitboard(Square::A2),
                Color::kBlack,
                0x303030303030000ull
            }
        };
        for (const auto& test : tests_b3)
            REQUIRE(BackwardThreeFillBB(test.bb, test.color) == test.expected_bb);
    }
}