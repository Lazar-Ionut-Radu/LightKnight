#ifndef LIGHTKNIGHT_ENGINE_H
#define LIGHTKNIGHT_ENGINE_H

#include <cstddef>
#include <functional>
#include <optional>
#include <thread>

#include "board.h"
#include "move_search.h"
#include "transposition_table.h"
#include "pawn_hash.h"
#include "params.h"

namespace lightknight {
    const std::string kDefaultParamFilePath = "params.csv";

    struct SearchLimits {
        // Search no deeper than this depth.
        std::optional<int> max_depth;

        // Stop once this many nodes have been searched.
        std::optional<uint64_t> max_nodes;

        // Search for at most this many ms.
        int time_limit_ms;
    };

    enum class OptionType {
        kCheck,
        kSpin,
        kString,
        kCombo,
        kButton
    };

    struct Option {
        std::string name;
        OptionType type;

        // Relevant for particular types.
        std::string default_value;
        int min = 0;
        int max = 0;

        std::vector<std::string> vars;
    };

    class Engine {
    public:
        // TODO: I should really make this private
        // Its important not to be modified during a search
        Board board;
        
        // Functions to print info & bestmove with uci.
        using SearchInfoCallback = std::function<void(const search::SearchInfo&)>;
        using BestMoveCallback = std::function<void(Move)>;
        using InfoStringCallback = std::function<void(const std::string&)>;
        
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

        const std::vector<Option>& GetOptions() const;
        void SetOption(const std::string& name, const std::string& value, InfoStringCallback print_info_fn);
        void SaveParameters(const std::string& path);
        void LoadParameters(const std::string& path);

    private:
        // Engine parameters
        parameters::EngineParameters params;

        // Everything making up the search algorithm.
        search::TranspositionTable tt;
        eval::PawnHash pawn_hash;
        search::TimeControlStruct time_control_struct;

        // Engine options, to be set using UCI
        std::vector<Option> _options;

        std::string params_file_path = kDefaultParamFilePath;

        // Separate search thread from the UCI handling thread.
        std::thread _search_thread;
        
        bool IsOptionValueValid(const Option& option, const std::string& value);
    };
} // namespace lightknight

#endif // LIGHTKNIGHT_ENGINE_H