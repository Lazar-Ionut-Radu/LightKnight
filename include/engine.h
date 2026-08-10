#ifndef LIGHTKNIGHT_ENGINE_H
#define LIGHTKNIGHT_ENGINE_H

#include <cstddef>
#include <functional>
#include <optional>
#include <thread>

#include "board.h"
#include "move_search.h"
#include "transposition_table.h"
#include "params.h"

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
        explicit Engine();
        ~Engine();

        // No copying.
        Engine(const Engine&) = delete;
        Engine& operator=(const Engine&) = delete;

        void NewGame();
        void SetPosition(Board board);
        void SetPosition(const std::string& fen);
        void StartSearch(const SearchLimits& limits, SearchInfoCallback PrintSearchInfo, BestMoveCallback PrintBestMove);
        void StopSearch();

        void SaveParameters(const std::string& path);
        void LoadParameters(const std::string& path);
        
    private:
        // Everything making up the search algorithm.
        search::TranspositionTable tt;
        search::TimeControlStruct time_control_struct;

        // Engine options
        parameters::EngineParameters params;

        // Separate search thread from the UCI handling thread.
        std::thread _search_thread;  
    };
} // namespace lightknight

#endif // LIGHTKNIGHT_ENGINE_H