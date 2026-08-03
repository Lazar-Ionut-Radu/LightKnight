#ifndef LIGHTKNIGHT_ENGINE_H
#define LIGHTKNIGHT_ENGINE_H

#include <cstddef>
#include <functional>
#include <optional>
#include <thread>

#include "board.h"
#include "move_search.h"
#include "transposition_table.h"

namespace lightknight {

    const int kDefaultHashSizeMB = 256;
    struct SearchLimits {
        // Search no deeper than this depth.
        std::optional<int> max_depth;

        // Stop once this many nodes have been searched.
        std::optional<uint64_t> max_nodes;

        // Search for at most this many ms.
        int time_limit_ms;
    };

    class Engine {
    public:
        // TODO: I should really make this private
        // Its important not to be modified during a search
        Board board;
        
        // Functions to print info & bestmove with uci.
        using SearchInfoCallback = std::function<void(const search::SearchInfo&)>;
        using BestMoveCallback = std::function<void(Move)>;

        // Constructor destructor.
        explicit Engine(size_t hash_size_mb = kDefaultHashSizeMB);
        ~Engine();

        // No copying.
        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;

        void NewGame();
        void SetPosition(Board board);
        void SetPosition(const std::string& fen);
        void StartSearch(const SearchLimits& limits, SearchInfoCallback PrintSearchInfo, BestMoveCallback PrintBestMove);
        void StopSearch();
        
    private:
        // Everything making up the search algorithm.
        search::TranspositionTable _tt;
        search::TimeControlStruct _time_control;

        // Engine options
        size_t _hash_size_mb;

        std::thread _search_thread;  
    };
} // namespace lightknight

#endif // LIGHTKNIGHT_ENGINE_H