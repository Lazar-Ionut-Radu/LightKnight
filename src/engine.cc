// engine.cc
#include "engine.h"

#include <algorithm>
#include <chrono>
#include <sstream>
#include <utility>
#include <vector>

#include "movegen.h"
#include "params.h"

namespace lightknight {
    const std::string kStartFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    Engine::Engine() 
        : params(),
          tt(parameters::kDefaultHashSizeMB),
          pawn_hash(parameters::kDefaultPawnHashSizeKB)
    {
        this->board.FromFEN(kStartFen);

        this->_options = {
            { // Transposition table size.
                .name = "Hash",
                .type = OptionType::kSpin,
                .default_value = std::to_string(parameters::kDefaultHashSizeMB),
                .min = 1,
                .max = 1024
            },
            { // Set a parameters file internally to write to / read from.
                .name = "ParametersFile",
                .type = OptionType::kString,
                .default_value = kDefaultParamFilePath
            },
            { // Load parameters from the file specified by option "Parameters File".
                .name = "LoadParameters",
                .type = OptionType::kButton
            },
            { // Save parameters to a file specified by option "Parameters File"
                .name = "SaveParameters",
                .type = OptionType::kButton
            }
        };
    }

    Engine::~Engine() {
        this->StopSearch();
    }

    void Engine::NewGame() {
        this->StopSearch();

        this->tt.Clear();
        this->pawn_hash.Clear();
        this->board.FromFEN(kStartFen);
    }

    void Engine::SetPosition(Board board) {
        this->StopSearch();
        this->board = std::move(board);
    }

    void Engine::SetPosition(const std::string& fen) {
        this->StopSearch();
        this->board.FromFEN(fen);
    }

    void Engine::StartSearch(const SearchLimits& limits, SearchInfoCallback print_search_info_fn, BestMoveCallback print_best_move_fn){
        // Make sure only one search is running at a time.
        this->StopSearch();

        // Copy the board for the search.
        Board search_board = this->board;

        // Configure search depth.
        const int max_depth = std::clamp(limits.max_depth.value_or(search::kMaxDepth), 1, search::kMaxDepth);

        // Configure search time.
        this->time_control_struct.stopped = false;
        this->time_control_struct.calls_until_clock_check = search::kCallsPerClockCheck;
        
        if (limits.time_limit_ms) {
            const int time_limit_ms = std::max(limits.time_limit_ms, 1);

            this->time_control_struct.deadline = std::chrono::steady_clock::now();
            this->time_control_struct.deadline += std::chrono::milliseconds(time_limit_ms);
        } else {
            // No time limit -> infinite time basically
            this->time_control_struct.deadline = std::chrono::steady_clock::time_point::max();
        }

        // Start the search thread.
        _search_thread = std::thread(
            [
                this,
                board = std::move(search_board),
                max_depth,
                print_search_info_fn = std::move(print_search_info_fn),
                print_best_move_fn = std::move(print_best_move_fn)
            ]() mutable {
                // Search
                search::SearchStats stats{};
                search::IterativeDeepening(board, max_depth, params, tt, pawn_hash, time_control_struct, stats, print_search_info_fn);

                // Get the best move
                search::TTEntry* tt_entry = this->tt.Probe(board.zobrist_hash);
                Move move{};
                if (tt_entry)
                    move = tt_entry->move;
                
                // Null move is a1a1.
                print_best_move_fn(move);
            }
        );
    }

    void Engine::StopSearch() {
        this->time_control_struct.stopped = true;

        if (_search_thread.joinable())
            _search_thread.join();
    }

    const std::vector<Option>& Engine::GetOptions() const {
        return this->_options;
    }

    void Engine::SetOption(const std::string& name, const std::string& value, InfoStringCallback print_info_fn) {
        // Search the option by name
        Option* option = nullptr;

        for (Option& curr_option : this->_options) {
            if (curr_option.name == name) {
                option = &curr_option;
                break;
            }
        }

        // Option not found.
        if (!option) {
            print_info_fn("Unknown option: '" + name + "'");
            return;
        }
        
        // Invalid value.
        if (!IsOptionValueValid(*option, value)) {
            print_info_fn("Invalid value '" + value + "' for option '" + name + "'");
            return;
        }

        // Set the options. 
        if (name == "Hash") {
            this->StopSearch();

            this->params.tt_size_mb = std::stoi(value);
            this->tt.ResizeMB(this->params.tt_size_mb);
        }
        else if(name == "ParametersFile") {
            this->params_file_path = value;
        }
        else if (name == "LoadParameters") {
            this->StopSearch();
            this->LoadParameters(this->params_file_path);
        }
        else if (name == "SaveParameters") {
            SaveParameters(this->params_file_path);
        }
    }

    bool Engine::IsOptionValueValid(const Option& option, const std::string& value) {
        switch (option.type) {
            case OptionType::kButton:
                return value.empty();

            case OptionType::kString:
                return true;

            case OptionType::kCheck:
                return value == "true" || value == "false";

            case OptionType::kSpin:
                try {
                    size_t num_digits;
                    int number = std::stoi(value, &num_digits);

                    // Make sure the entire string was consumed.
                    if (num_digits != value.size())
                        return false;

                    return number >= option.min && number <= option.max;
                }
                catch (...) {
                    return false;
                }
        
            case OptionType::kCombo:
                return std::find(
                    option.vars.begin(),
                    option.vars.end(),
                    value
                ) != option.vars.end();
                
            default:
                return false;
        }
    }
        
    void Engine::SaveParameters(const std::string& path) {
        parameters::SaveParameters(this->params, path);
    }

    void Engine::LoadParameters(const std::string& path) {
        parameters::LoadParameters(this->params, path);
    }
} // namespace lightknight