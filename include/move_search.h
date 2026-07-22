#ifndef LIGHTKNIGHT_MOVE_SEARCH_H
#define LIGHTKNIGHT_MOVE_SEARCH_H

#include <chrono>
#include <vector>

#include "types.h"
#include "board.h"
#include "transposition_table.h"

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

    enum class SearchNodeType : bool {
        kPVNode,
        kNonPVNode
    };

    struct SearchStats {
        uint64_t search_nodes = 0;
        uint64_t q_nodes = 0;
        uint64_t leaf_nodes = 0;

        uint64_t pv_nodes = 0;
        uint64_t cut_nodes = 0;
        uint64_t all_nodes = 0;

        uint64_t q_pv_nodes = 0;
        uint64_t q_cut_nodes = 0;
        uint64_t q_all_nodes = 0;
        
        uint64_t evaluations = 0;
        uint64_t beta_cutoffs = 0;

        int depth_searched = 0;
        double elapsed_ms = 0.0;

        constexpr uint64_t total_nodes() const {
            return search_nodes + q_nodes;
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
    
    
    int IterativeDeepening(
        Board& board,
        int max_depth,
        TranspositionTable& tt,
        int time_limit_ts
    );
    
    template<SearchNodeType node_type>
    int PrincipalVariationSearch(
        Board& board,
        int alpha, // The best (highest) score this player can guarantee so far.
        int beta, // The best (lowest) score the opponent can guarantee so far.
        int max_depth, // Max search depth.
        int depth, // Current depth.
        std::vector<std::vector<Move>>& move_lists, // Preallocated vectors.
        TranspositionTable& tt, // Memoization of positions.
        TimeControlStruct& time_control
    );

    template<SearchNodeType node_type>
    int QuiescenceSearch(
        Board& board,
        int alpha, // The best (highest) score this player can guarantee so far.
        int beta, // The best (lowest) score the opponent can guarantee so far.
        int depth,
        std::vector<std::vector<Move>>& move_lists, // Preallocated vectors.
        TranspositionTable& tt,
        TimeControlStruct& time_control
    );

} // namespace lightknight::search

#endif // LIGHTKNIGHT_MOVE_SEARCH_H
