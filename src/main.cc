#include <board.h>
#include <move_search.h>

#include <iostream>
#include <chrono>

int main()
{
    lightknight::Board position(
        //"q6k/8/8/8/8/8/8/R6K w - - 0 1"
        "r2q2k1/2p3p1/p3p2p/bp1pB3/6P1/2PQ3P/PP1N1r2/2KR2R1 w - - 0 20"
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
    std::cout << "\nTime elapsed: " << std::chrono::duration_cast<std::chrono::milliseconds>(time2-time1).count() << "ms.\n\n";
    
    std::cout << "PV: ";
    for (int i = 0; i < result.pv_length; i++)
        std::cout << result.pv[i] << " ";
    std::cout << '\n';

    std::cout
        << "Evaluation: " << result.evaluation << "\n"
        << "Depth Searched: " << result.stats.depth_searched << "\n\n"
        << "Total nodes: " << result.stats.total_nodes() << "\n"
        << "Leaf nodes: " << result.stats.leaf_nodes << "\n\n"
        << "Nodes:     " << result.stats.search_nodes << " " << result.stats.q_nodes << "\n"
        << "PV nodes:  " << result.stats.pv_nodes << " " << result.stats.q_pv_nodes << "\n" 
        << "ALL nodes: " << result.stats.all_nodes << " " << result.stats.q_all_nodes << "\n"
        << "CUT nodes: " << result.stats.cut_nodes << " " << result.stats.q_cut_nodes << "\n\n"
        << "Evaluations: " << result.stats.evaluations << "\n"
        << "Beta cutoffs: " << result.stats.beta_cutoffs << "\n"
        << "MNPS: " << result.stats.mnps() << "\n"
        << "Q-nodes %: " << 100.0 * (double)(result.stats.q_nodes) / (double)(result.stats.total_nodes()) << '\n';  
    
    return 0;
}