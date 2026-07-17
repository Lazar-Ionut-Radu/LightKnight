#include <board.h>
#include <move_search.h>

#include <iostream>
#include <chrono>

int main()
{
    lightknight::Board position(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR "
        "w KQkq - 0 1"
    );

    constexpr int maximum_depth = 10;
    constexpr int time_limit_ms = 10'000;

    auto time1 = std::chrono::steady_clock::now();
    const lightknight::search::SearchResult result =
        lightknight::search::IterativeDeepening<true>(
            position,
            maximum_depth,
            time_limit_ms
        );
    
    auto time2 = std::chrono::steady_clock::now();
    std::cout << "\nTime elapsed: " << std::chrono::duration_cast<std::chrono::milliseconds>(time2-time1).count() << "ms.\n";
    std::cout
        << "Evaluation: " << result.evaluation << '\n'
        << "Leaf nodes: " << result.stats.leaf_nodes << '\n'
        << "Inner nodes: " << result.stats.inner_nodes << '\n'
        << "Total nodes: " << result.stats.total_nodes() << '\n'
        << "Beta cutoffs: " << result.stats.beta_cutoffs << '\n'
        << "MNPS: " << result.stats.mnps() << '\n';
    
    return 0;
}