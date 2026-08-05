// uci.cc
#include "uci.h"
#include "board.h"
#include "exceptions.h"
#include "movegen.h"
#include "move_search.h"
#include "time_management.h"

#include <atomic>
#include <chrono>
#include <cctype>
#include <mutex>
#include <optional>
#include <thread>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <string>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <limits>

namespace lightknight::uci {
    // String parsing helpers.
    std::string Trim(std::string& text) {
        const size_t first = text.find_first_not_of(" \t\r\n");

        if (first == std::string::npos)
            return "";

        const size_t last = text.find_last_not_of(" \t\r\n");

        return text.substr(first, last - first + 1);
    }

    std::string ToLower(std::string text) {
        std::transform(text.begin(), text.end(), text.begin(),
            [](unsigned char character) {
                return static_cast<char>(std::tolower(character));
            }
        );

        return text;
    }

    UCI::UCI() : _engine() {};

    UCI::~UCI() {
        this->_engine.StopSearch();
    }

    void UCI::Loop() {
        std::string line;

        // Input loop.
        while (std::getline(std::cin, line)) {
            line = Trim(line);

            if (line.empty())
                continue;

            if (line == "quit") {
                // Make sure the thread is not kept running.
                this->_engine.StopSearch();
                return;
            }

            HandleCommand(line);
        }

        // Once again, don't forget the search thread running.
        this->_engine.StopSearch();
    }

    void UCI::HandleCommand(const std::string& line) {
        if (line == "uci") {
            this->HandleUci();
        }
        else if (line == "isready") {
            this->PrintLine("readyok");
        }
        else if (line == "ucinewgame") {
            this->HandleNewGame();
        }
        else if (line.starts_with("setoption")) {
            this->HandleSetOption(line);
        }
        else if (line.starts_with("position")) {
            this->HandlePosition(line);
        }
        else if (line.starts_with("go")) {
            this->HandleGo(line);
        }
        else if (line == "stop") {
            this->_engine.StopSearch();
        }
        else {
            PrintLine("info string unknown command: " + line);
        }
    }

    void UCI::HandleUci() {
        this->PrintLine("id name LightKnight v0.2.1");
        this->PrintLine("id author Lazar Ionut-Radu");
        
        // [TODO] later.
        // this->PrintLine("option name Hash type spin default 16 min 1 max 4096");

        this->PrintLine("uciok");
    }

    void UCI::HandleSetOption(const std::string& line) {
        // [TODO] later
    }

    void UCI::HandleNewGame() {
        this->_engine.NewGame();
    }

    void UCI::HandlePosition(const std::string& line) {
        // Make sure no search is ongoing.
        this->_engine.StopSearch();

        // Reading the line nicer.
        std::istringstream stream(line);
     
        std::string command;
        std::string position_type;

        // Reading the FEN part of the command.
        stream >> command >> position_type;

        if (position_type.empty()) {
            PrintLine("info string malformed position command");
            return;
        }

        Board new_board;
        try {
            if (position_type == "startpos") {
                new_board.FromFEN(kStartFen);
            }
            else if (position_type == "fen") {
                std::string fen;
                std::string fen_part;

                // Read the 6 parts of the fen.
                for (int i = 0; i < 6; ++i) {
                    if (!(stream >> fen_part)) {
                        this->PrintLine("info string incomplete FEN");
                        return;
                    }

                    if (i != 0)
                        fen += ' ';
                    fen += fen_part;
                }

                new_board.FromFEN(fen);
            }
            else {
                this->PrintLine("info string unknown position type: " + position_type);
                return;
            }
        }
        catch (const exceptions::FENException& exception) {
            this->PrintLine(std::string("info string invalid FEN: ") + exception.what());
            return;
        }

        // Reading the moves part of the command.
        std::string token;
        if (stream >> token) {
            if (token != "moves") {
                this->PrintLine("info string expected moves, got: " + token);
                return;
            }
        }

        std::string move_string;
        while (stream >> move_string) {
            // Check move legality.
            const std::optional<Move> move = FindLegalMove(new_board, move_string);

            if (!move.has_value()) {
                this->PrintLine("info string illegal move: " + move_string);
                return;
            }

            // Make move.
            UndoMoveInfo undo{};
            new_board.MakeMove(*move, undo);
        }

        this->_engine.SetPosition(std::move(new_board));
    }

    void UCI::HandleGo(const std::string& line) {
        const GoCmdInfo go_info = this->ParseGoCommand(line);

        // Setup search limits.
        SearchLimits limits{};

        // Depth of the search.
        if (go_info.depth.has_value()) {
            limits.max_depth = *go_info.depth;
        }
        else {
            limits.max_depth = search::kMaxDepth;
        }

        // Time of the search
        if (go_info.infinite) {
            limits.time_limit_ms = time_management::kMaxSearchTimeMs;
        }
        else if (go_info.move_time_ms.has_value()) {
            limits.time_limit_ms = *go_info.move_time_ms;
        }
        else {
            const bool white_to_move = this->_engine.board.turn == Color::kWhite;

            std::optional<int> remaining_time_ms = white_to_move ? go_info.white_time_ms : go_info.black_time_ms;
            std::optional<int> increment_ms = white_to_move ? go_info.white_increment_ms : go_info.black_increment_ms;
            std::optional<int> moves_to_go = go_info.moves_to_go;

            limits.time_limit_ms = time_management::ComputeTimeLimitMs(remaining_time_ms, increment_ms, moves_to_go);
        }

        // Maximum number of nodes to search.
        // [TODO]: For now I don't care.
        limits.max_nodes = std::numeric_limits<uint64_t>::max();

        this->_engine.StartSearch(
            limits,
            [this](const search::SearchInfo& info) {
                this->PrintSearchInfo(info);
            },
            [this](Move move) {
                if (move.IsNull()) {
                    this->PrintLine("bestmove 0000");
                    return;
                }

                std::ostringstream output;
                output << "bestmove " << move;

                this->PrintLine(output.str());
            }
        );
    }

    void UCI::PrintLine(const std::string& line) {
        // Lock because search thread and these Handle* functions may write to stdout.
        const std::lock_guard lock(this->_output_mutex);

        std::cout << line << '\n';
        std::cout.flush();
    }
    
    // Convert string to Move type if the move is legal in the position.
    std::optional<Move> UCI::FindLegalMove(Board& board, const std::string& move_string) const {
        // Generate the legal moves in the position.
        std::vector<Move> legal_moves;
        legal_moves.reserve(256);
        movegen::GenerateMoves<movegen::MoveGenType::kAll>(board, legal_moves);
    
        // Use the << operator to compare strings.
        for (const Move move : legal_moves) {
            std::ostringstream stream;
            stream << move;

            if (stream.str() == move_string)
                return move;
        }

        return std::nullopt;
    }

    // Return a GoCmdInfo struct when parsing the go command.
    GoCmdInfo UCI::ParseGoCommand(const std::string& line) const {
        GoCmdInfo go_info{};

        // Nice read line.
        std::istringstream stream(line);
        std::string command;
        std::string token;

        stream >> command; // Get rid of "go".
        while (stream >> token) {
            // Function to read <name> <int> pair.
            auto read_value = [&stream](std::optional<int>& destination) {
                int value = 0;

                if (stream >> value)
                    destination = value;
            };

            if (token == "wtime") {
                read_value(go_info.white_time_ms);
            }
            else if (token == "btime") {
                read_value(go_info.black_time_ms);
            }
            else if (token == "winc") {
                read_value(go_info.white_increment_ms);
            }
            else if (token == "binc") {
                read_value(go_info.black_increment_ms);
            }
            else if (token == "movestogo") {
                read_value(go_info.moves_to_go);
            }
            else if (token == "movetime") {
                read_value(go_info.move_time_ms);
            }
            else if (token == "depth") {
                read_value(go_info.depth);
            }
            else if (token == "infinite") {
                go_info.infinite = true;
            }

            // [TODO] Currently unimplemented:
            //  - searchmoves
            //  - ponder
            //  - nodes
            //  - mate
        }
        
        return go_info;
    }

    void UCI::PrintSearchInfo(const search::SearchInfo& info) {
        const uint64_t nps = info.time_ms == 0 ? 0 : info.nodes * 1000 / info.time_ms;

        std::ostringstream output;

        output << "info" 
            << " depth " << info.depth
            << " seldepth " << info.selective_depth;
        
        // Score, either normal or mate score.
        if (search::IsMateScore(info.score))
            output << " score mate " << search::GetMateMoves(info.score);
        else 
            output << " score cp " << info.score;
        
        output
            << " time " << info.time_ms
            << " nodes " << info.nodes
            << " nps " << nps;

        // PV
        if (!info.pv.empty()) {
            output << " pv";

            for (const Move move : info.pv)
                output << ' ' << move;
        }

        // Print all of this.
        this->PrintLine(output.str());
    }

} // namespace lightknight::uci
