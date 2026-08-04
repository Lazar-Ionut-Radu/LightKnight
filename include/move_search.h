#ifndef LIGHTKNIGHT_MOVE_SEARCH_H
#define LIGHTKNIGHT_MOVE_SEARCH_H

#include <chrono>
#include <vector>
#include <atomic>
#include <functional>

#include "types.h"
#include "board.h"
#include "transposition_table.h"

namespace lightknight::search {
    inline constexpr int kInfinity = 128'000;
    inline constexpr int kMateScore = 120'000;
    inline constexpr int kMaxDepth = 128;
    inline constexpr int kCallsPerClockCheck = 1024;

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

    struct SearchStats {
        // Stats are cummulative, in the sense that they count together all the previous depths,
        // not just the last one.

        // Work done during the entire search.
        uint64_t search_nodes = 0;
        uint64_t q_nodes = 0;
        
        // Last fully completed iterative-deepening depth.
        int depth = 0;

        // Deepest ply reached.
        int selective_depth = 0;

        // Elapsed time since the entire search started.
        uint64_t time_ms = 0;
    };

    // What is strictly necessary for the uci info command.
    struct SearchInfo {
        uint64_t nodes = 0;
        uint64_t time_ms = 0;
        
        int depth = 0;
        int selective_depth = 0;

        int score = 0;
        std::vector<Move> pv; 
    };

    // Used in iterative deepening to print out information about the search with UCI.
    using SearchInfoCallback = std::function<void(const SearchInfo&)>;

    int IterativeDeepening(
        Board& board,
        int max_depth,
        TranspositionTable& tt,
        TimeControlStruct& time_control,
        SearchStats& search_stats,
        const SearchInfoCallback& info_callback // Prints search info with UCI.
    );

    template<SearchNodeType node_type>
    int PrincipalVariationSearch(
        Board& board,
        int alpha, // The best (highest) score this player can guarantee so far.
        int beta, // The best (lowest) score the opponent can guarantee so far.
        int max_depth, // Max search depth.
        int depth, // Current depth.
        std::vector<std::vector<Move>>& move_lists, // Preallocated vectors.
        std::vector<std::vector<int>>& score_lists, // Preallocated vectors for move scores. 
        TranspositionTable& tt, // Memoization of positions.
        TimeControlStruct& time_control,
        SearchStats& stats
    );

    template<SearchNodeType node_type>
    int QuiescenceSearch(
        Board& board,
        int alpha, // The best (highest) score this player can guarantee so far.
        int beta, // The best (lowest) score the opponent can guarantee so far.
        int depth,
        std::vector<std::vector<Move>>& move_lists, // Preallocated vectors.
        std::vector<std::vector<int>>& score_lists, // Preallocated vectors for move scores.
        TranspositionTable& tt,
        TimeControlStruct& time_control,
        SearchStats& stats
    );
    
    int ScoreToTT(int score, int ply);
    int ScoreFromTT(int score, int ply);

    bool IsMateScore(int score);
    int GetMateMoves(int score);

    void OrderMoves(Board& board, std::vector<Move>& moves, TTEntry* tt_entry);
    
    bool ShouldStopSearch(TimeControlStruct& time_control);
    
    std::vector<Move> ExtractPV(Board board, const TranspositionTable& tt);
    SearchInfo BuildSearchInfo(
        const Board& board,
        const TranspositionTable& tt,
        const SearchStats& stats,
        int score
    );

} // namespace lightknight::search

#endif // LIGHTKNIGHT_MOVE_SEARCH_H
