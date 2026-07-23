#ifndef LIGHTKNIGHT_MOVE_SEARCH_H
#define LIGHTKNIGHT_MOVE_SEARCH_H

#include <chrono>
#include <vector>
#include <atomic>

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
        // Atomic so it may be modified by the engine's search thread and the uci input
        // parser that spawned it.
        std::atomic_bool stopped{false};
    };

    enum class SearchNodeType : bool {
        kPVNode,
        kNonPVNode
    };

    bool ShouldStopSearch(TimeControlStruct& time_control);
        
    int IterativeDeepening(
        Board& board,
        int max_depth,
        TranspositionTable& tt,
        TimeControlStruct& time_control
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
