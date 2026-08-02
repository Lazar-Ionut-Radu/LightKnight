// test/test_perft.cc
#include <catch2/catch_test_macros.hpp>

#include "board.h"
#include "perft.h"

#include <cstdint>
#include <string>
#include <vector>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>

namespace {

struct PerftTest {
    std::string name;
    std::string fen;
    std::vector<std::uint64_t> nodes;
};

// I which it was fast enough to be able to check into billions of nodes (with bulk counting)
// Thanks to https://www.chessprogramming.org/Perft_Results.
const std::vector<PerftTest> kPerftTests = {
    { 
        "Initial position",
        "rnbqkbnr/pppppppp/8/8/8/8/"
        "PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        {1, 20, 400, 8'902, 197'281, 4'865'609, 119'060'324} // Depth 6
    },
    {
        "Kiwipete",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/"
        "1p2P3/2N2Q1p/PPPBBPPP/R3K2R "
        "w KQkq - 0 1",
        {1, 48, 2'039, 97'862, 4'085'603, 193'690'690} // Depth 5
    },
    {
        "Position 3 - Chessprogramming wiki",
        "8/2p5/3p4/KP5r/1R3p1k/"
        "8/4P1P1/8 w - - 0 1",
        {1, 14, 191, 2'812, 43'238, 674'624, 11'030'083, 178'633'661} // Depth 7
    },
    {
        "Position 4 - Chessprogramming wiki",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/"
        "BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 "
        "w kq - 0 1",
        {1, 6, 264, 9'467, 422'333, 15'833'292, 706'045'033} // Depth 6
    },
    {
        "Position 5 - Chessprogramming wiki",
        "rnbq1k1r/pp1Pbppp/2p5/8/"
        "2B5/8/PPP1NnPP/RNBQK2R "
        "w KQ - 1 8",
        {1, 44, 1'486, 62'379, 2'103'487, 89'941'194} // Depth 5
    },
    {
        "Position 6 - Chessprogramming wiki",
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/"
        "2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 "
        "w - - 0 10",
        {1, 46, 2'079, 89'890, 3'894'594, 164'075'551} // Depth 5
    }
};

} // namespace

TEST_CASE(
    "Reference perft positions",
    "[IntegrationTest][Perft][MoveGen][Move]"
) {
    for (const PerftTest& test : kPerftTests) {
        for (std::size_t depth = 0; depth < test.nodes.size(); ++depth) {
            DYNAMIC_SECTION(
                test.name << " at depth " << depth
            ) {
                lightknight::Board board(test.fen);

                const std::uint64_t actual =
                    lightknight::Perft(
                        board,
                        static_cast<int>(depth),
                        true
                    );

                CHECK(actual == test.nodes[depth]);
            }
        }
    }
}

namespace {

struct PerftBenchmarkResult {
    int depth;
    std::uint64_t nodes;

    double mean_time_ms;
    double std_time_ms;

    double mean_mnps;
    double std_mnps;
};

double Mean(const std::vector<double>& values) {
    return std::accumulate(
        values.begin(),
        values.end(),
        0.0
    ) / static_cast<double>(values.size());
}

double SampleStandardDeviation(
    const std::vector<double>& values,
    double mean
) {
    if (values.size() < 2)
        return 0.0;

    double sum = 0.0;

    for (const double value : values) {
        const double difference = value - mean;
        sum += difference * difference;
    }

    return std::sqrt(
        sum / static_cast<double>(values.size() - 1)
    );
}

PerftBenchmarkResult BenchmarkPerftDepth(
    const std::string& fen,
    int depth,
    int repetitions = 5
) {
    lightknight::Board board(fen);

    // Warm-up run, not measured.
    std::uint64_t nodes =
        lightknight::Perft(board, depth);

    std::vector<double> times_ms;
    std::vector<double> speeds_mnps;

    times_ms.reserve(repetitions);
    speeds_mnps.reserve(repetitions);

    for (int repetition = 0;
         repetition < repetitions;
         ++repetition) {
        const auto start =
            std::chrono::steady_clock::now();

        nodes = lightknight::Perft(board, depth);

        const auto end =
            std::chrono::steady_clock::now();

        const double seconds =
            std::chrono::duration<double>(
                end - start
            ).count();

        times_ms.push_back(seconds * 1'000.0);

        speeds_mnps.push_back(
            static_cast<double>(nodes) /
            seconds /
            1'000'000.0
        );
    }

    const double mean_time_ms = Mean(times_ms);
    const double mean_mnps = Mean(speeds_mnps);

    return {
        .depth = depth,
        .nodes = nodes,
        .mean_time_ms = mean_time_ms,
        .std_time_ms =
            SampleStandardDeviation(times_ms, mean_time_ms),
        .mean_mnps = mean_mnps,
        .std_mnps =
            SampleStandardDeviation(speeds_mnps, mean_mnps)
    };
}

void PrintPerftBenchmarkTable(
    const std::string& name,
    const std::string& fen,
    int maximum_depth,
    int repetitions = 10
) {
    std::cout
        << '\n'
        << name << ": " << fen << '\n'
        << std::left
        << std::setw(8)  << "Depth"
        << std::setw(18) << "Nodes"
        << std::setw(26) << "Time (ms)"
        << std::setw(26) << "Speed (MNPS)"
        << '\n'
        << std::string(78, '-')
        << '\n';

    for (int depth = 1;
         depth <= maximum_depth;
         ++depth) {
        const PerftBenchmarkResult result =
            BenchmarkPerftDepth(
                fen,
                depth,
                repetitions
            );

        std::ostringstream time;
        time
            << std::fixed
            << std::setprecision(3)
            << result.mean_time_ms
            << " ± "
            << result.std_time_ms;

        std::ostringstream speed;
        speed
            << std::fixed
            << std::setprecision(3)
            << result.mean_mnps
            << " +/- "
            << result.std_mnps;

        std::cout
            << std::left
            << std::setw(8)  << result.depth
            << std::setw(18) << result.nodes
            << std::setw(26) << time.str()
            << std::setw(26) << speed.str()
            << '\n';
    }
}
} // namespace

TEST_CASE(
    "Perft speed by depth",
    "[!benchmark][Perft]"
) {
    PrintPerftBenchmarkTable(
        "Starting position",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        6,
        4
    );

    PrintPerftBenchmarkTable(
        "Endgame position",
        "8/5p2/k3r1p1/P6p/5K1P/6P1/R4P2/8 w - - 16 65",
        6,
        4
    );

    PrintPerftBenchmarkTable(
        "Midgame position",
        "r1bq1rk1/2p1bppp/p1n2n2/1p1pp3/4P3/1BP2N2/PP1P1PPP/RNBQR1K1 w - d6 0 9",
        6,
        4
    );
}