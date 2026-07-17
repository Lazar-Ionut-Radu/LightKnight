#ifndef LIGHTKNIGHT_MOVE_SEARCH_H
#define LIGHTKNIGHT_MOVE_SEARCH_H

#include <chrono>
#include <vector>

#include "types.h"
#include "board.h"

namespace lightknight::search {
    inline constexpr int kInfinity = 128'000;
    inline constexpr int kMateScore = 120'000;
    inline constexpr int kMaxDepth = 128;
    inline constexpr int kCallsPerClockCheck = 1024;

    struct PrincipalVariation {
        // table[depth] = best line found at that depth .
        lightknight::Move table[kMaxDepth][kMaxDepth]{};
        
        // Depth of pv for each one. 
        int length[kMaxDepth]{};
    };

    struct TimeControlStruct {
        std::chrono::steady_clock::time_point deadline;

        // Number of search function calls before reading the clock again.
        int calls_until_clock_check = kCallsPerClockCheck;

        // Has the time allocated elapsed.
        bool stopped = false;
    };

    struct SearchStats {
        uint64_t leaf_nodes = 0;
        uint64_t inner_nodes = 0;
        uint64_t beta_cutoffs = 0;
        double elapsed_ms = 0.0;

        constexpr uint64_t total_nodes() const {
            return leaf_nodes + inner_nodes;
        }

        constexpr double mnps() const {
            if (elapsed_ms <= 0.0)
                return 0.0;

            return static_cast<double>(total_nodes()) / (elapsed_ms * 1000.0);
        }
    };

    struct SearchResult {
        int evaluation = 0;
        lightknight::Move pv[kMaxDepth];
        int pv_length = 0;
        SearchStats stats{};
    };

    bool ShouldStopSearch(TimeControlStruct& time_control);

    template<bool collect_stats>
    int AlphaBeta(
        lightknight::Board& board,
        std::vector<std::vector<lightknight::Move>>& move_lists,
        int alpha,
        int beta,
        int max_depth,
        int depth,
        PrincipalVariation& pv,
        TimeControlStruct& time_control,
        SearchStats& stats
    );

    template<bool collect_stats>
    SearchResult IterativeDeepening(
        Board& board,
        int max_depth,
        int time_limit_ms
    );

} // namespace lightknight::search

#endif // LIGHTKNIGHT_MOVE_SEARCH_H
