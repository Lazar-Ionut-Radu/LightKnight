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

const std::vector<PerftTest> kPerftTests = {
    {
        "Initial position",
        "rnbqkbnr/pppppppp/8/8/8/8/"
        "PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        {1, 20, 400, 8902, 197281, 4865609}
    },
    {
        "Position 2 - Kiwipete",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/"
        "1p2P3/2N2Q1p/PPPBBPPP/R3K2R "
        "w KQkq - 0 1",
        {1, 48, 2039, 97862, 4085603, 193690690}
    },
    {
        "Position 3",
        "8/2p5/3p4/KP5r/1R3p1k/"
        "8/4P1P1/8 w - - 0 1",
        {1, 14, 191, 2812, 43238, 674624}
    },
    {
        "Position 4",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/"
        "BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 "
        "w kq - 0 1",
        {1, 6, 264, 9467, 422333, 15833292}
    },
    {
        "Position 5",
        "rnbq1k1r/pp1Pbppp/2p5/8/"
        "2B5/8/PPP1NnPP/RNBQK2R "
        "w KQ - 1 8",
        {1, 44, 1486, 62379, 2103487, 89941194}
    },
    {
        "Position 6",
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/"
        "2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 "
        "w - - 0 10",
        {1, 46, 2079, 89890, 3894594, 164075551}
    }
};

} // namespace

TEST_CASE(
    "Reference perft positions",
    "[Perft][MoveGeneration][Move]"
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
                        static_cast<int>(depth)
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
        << name << '\n'
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
    "[!benchmark][Perft][Performance]"
) {
    PrintPerftBenchmarkTable(
        "Starting position",
        "rnbqkbnr/pppppppp/8/8/8/8/"
        "PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        6,
        4
    );
}