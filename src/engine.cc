// engine.cc
#include "engine.h"

#include <algorithm>
#include <chrono>
#include <sstream>
#include <utility>
#include <vector>

#include "movegen.h"

namespace lightknight {
    const std::string kStartFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    Engine::Engine(std::size_t hash_size_mb) {
        this->board.FromFEN(kStartFen);
        this->_tt = search::TranspositionTable(hash_size_mb);

        this->_hash_size_mb = hash_size_mb;
    }

    Engine::~Engine() {
        this->StopSearch();
    }

    void Engine::NewGame() {
        this->StopSearch();

        this->_tt.Clear();
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

        // For now to be safe lets just clear the tt even though its dumb.
        this->_tt.Clear();

        // Copy the board for the search.
        Board search_board = this->board;

        // Configure search depth.
        const int max_depth = std::clamp(limits.max_depth.value_or(search::kMaxDepth), 1, search::kMaxDepth);

        // Configure search time.
        this->_time_control.stopped = false;
        this->_time_control.calls_until_clock_check = search::kCallsPerClockCheck;
        
        if (limits.time_limit_ms) {
            const int time_limit_ms = std::max(limits.time_limit_ms, 1);

            this->_time_control.deadline = std::chrono::steady_clock::now();
            this->_time_control.deadline += std::chrono::milliseconds(time_limit_ms);
        } else {
            // No time limit -> infinite time basically
            this->_time_control.deadline = std::chrono::steady_clock::time_point::max();
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
                search::IterativeDeepening(board, max_depth, this->_tt, this->_time_control, stats, print_search_info_fn);

                // Get the best move
                search::TTEntry* tt_entry = this->_tt.Probe(board.zobrist_hash);
                Move move{};
                if (tt_entry)
                    move = tt_entry->move;
                
                // Null move is a1a1.
                print_best_move_fn(move);
            }
        );
    }

    void Engine::StopSearch() {
        this->_time_control.stopped = true;

        if (_search_thread.joinable())
            _search_thread.join();
    }
} // namespace lightknight