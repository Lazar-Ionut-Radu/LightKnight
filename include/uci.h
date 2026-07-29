#ifndef LIGHTKNIGHT_UCI_H
#define LIGHTKNIGHT_UCI_H

#include <string>
#include <string_view>
#include <optional>
#include <thread>
#include <limits>
#include <atomic>

#include "board.h"
#include "transposition_table.h"
#include "move_search.h"

namespace lightknight::uci {
    const std::string kStartFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    const int kMinSearchTime = 10;
    const int kMaxSearchTime = std::numeric_limits<int>::max() / 2;

    std::string Trim(std::string& text);
    std::string ToLower(std::string text);

    struct GoCmdInfo {
        std::optional<int> white_time_ms;
        std::optional<int> black_time_ms;
        std::optional<int> white_increment_ms;
        std::optional<int> black_increment_ms;
        std::optional<int> moves_to_go;
        std::optional<int> move_time_ms;
        std::optional<int> depth;
        bool infinite;
    };
    class UCI {
    public:
        UCI();
        ~UCI();

        // No copying a UCI object
        UCI(const UCI&) = delete;
        UCI& operator=(const UCI&) = delete;

        // Input parsing loop.
        void Loop();

    private:
        // Engine state.
        Board _board;
        search::TranspositionTable _tt;
        search::TimeControlStruct _time_control;
        
        // Engine options.
        size_t hash_size_mb = 16;

        // UCI related fields
        // Thread that the engine will operate on.
        std::thread _search_thread;
        // Flag for stopping a search, requested by the GUI, passed to the search function.
        std::atomic_bool _stop_request{false}; 
        // Mutex for outputting text, either by this thread or the search thread.
        std::mutex _output_mutex; 

        // Print to output.
        void PrintLine(const std::string& line);
        
        // Parsing / resolving input commands from the GUI.
        void HandleCommand(const std::string& line);
        void HandleUci();
        void HandleSetOption(const std::string& line);
        void HandleNewGame();
        void HandlePosition(const std::string& line);
        void HandleGo(const std::string& line);

        void StartSearch(int max_depth, int time_limit_ms);
        void StopSearch();

        // Convert string to Move type if the move is legal in the position.
        std::optional<Move> FindLegalMove(Board& board, const std::string& move_string) const;

        // Return a GoCmdInfo struct when parsing the go command.
        GoCmdInfo ParseGoCommand(const std::string& line) const;
        
        // Time limit for the subsequent search from the parsed go command info.
        int ComputeTimeLimitMs(const GoCmdInfo& go_info) const;

        // Print info command during a search.
        void PrintSearchInfo(const search::SearchInfo& info);
    };
} // namespace lightknight::uci

#endif // LIGHTKNIGHT_UCI_H