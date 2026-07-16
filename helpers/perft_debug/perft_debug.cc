// helpers/perft_debug/perft_debug.cc
#include "perft_debug.h"
#include "board.h"
#include "movegen.h"
#include "perft.h"
#include "types.h"

#include <iomanip>
#include <sstream>
#include <iostream>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
#include <set>
#include <vector>
#include <optional>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace {
    std::string Trim(const std::string& text) {
        const size_t first = text.find_first_not_of(" \t\r\n");

        if (first == std::string::npos)
            return {};

        const size_t last = text.find_last_not_of(" \t\r\n");
        return text.substr(first, last - first + 1);
    }

    void SendCommand(
        FILE* input,
         const std::string& command
    ) {
        if (std::fprintf(input, "%s\n", command.c_str()) < 0)
            throw std::runtime_error("Failed to write command to Stockfish");

        std::fflush(input);
    }

    std::string MoveToString(const lightknight::Move move) {
        std::ostringstream stream;
        stream << move;
        return stream.str();
    }

    std::optional<lightknight::Move> FindMoveByUci(
        lightknight::Board& board,
        const std::string& uci_move
    ) {
        std::vector<lightknight::Move> moves;
        moves.reserve(256);

        lightknight::movegen::GenerateMoves(board, moves);

        for (const lightknight::Move move : moves) {
            if (MoveToString(move) == uci_move)
                return move;
        }

        return std::nullopt;
    }

    void PrintLine(const std::vector<std::string>& line) {
        if (line.empty()) {
            std::cout << "<root>";
            return;
        }

        for (std::size_t index = 0; index < line.size(); ++index) {
            if (index != 0)
                std::cout << ' ';

            std::cout << line[index];
        }
    }

    std::optional<PerftDiscrepancy>
    TraceFirstPerftDiscrepancyImpl(
        lightknight::Board& board,
        const std::string& stockfish_path,
        const std::string& original_fen,
        int depth,
        std::vector<std::string>& line
    ) {
        const PerftDivideResult lightknight_result =
            GetLightknightPerftDivide(board, depth);

        const PerftDivideResult stockfish_result =
            GetStockfishPerftDivide(
                stockfish_path,
                original_fen,
                depth,
                line
            );

        std::set<std::string> all_moves;

        for (const auto& [move, nodes] : lightknight_result)
            all_moves.insert(move);

        for (const auto& [move, nodes] : stockfish_result)
            all_moves.insert(move);

        for (const std::string& move : all_moves) {
            const auto lightknight_it =
                lightknight_result.find(move);

            const auto stockfish_it =
                stockfish_result.find(move);

            const bool in_lightknight =
                lightknight_it != lightknight_result.end();

            const bool in_stockfish =
                stockfish_it != stockfish_result.end();

            if (
                in_lightknight &&
                in_stockfish &&
                lightknight_it->second == stockfish_it->second
            ) {
                continue;
            }

            PerftDiscrepancy discrepancy{
                .line = line,
                .depth = depth,
                .move = move,
                .lightknight_nodes = in_lightknight
                    ? std::optional<std::uint64_t>(
                        lightknight_it->second
                    )
                    : std::nullopt,
                .stockfish_nodes = in_stockfish
                    ? std::optional<std::uint64_t>(
                        stockfish_it->second
                    )
                    : std::nullopt
            };

            // This is already the final discrepancy.
            if (!in_lightknight || !in_stockfish || depth == 1)
                return discrepancy;

            const auto selected_move =
                FindMoveByUci(board, move);

            if (!selected_move) {
                throw std::runtime_error(
                    "Could not find LightKnight move " + move
                );
            }

            lightknight::UndoMoveInfo undo{};

            board.MakeMove(*selected_move, undo);
            line.push_back(move);

            try {
                auto deeper_discrepancy =
                    TraceFirstPerftDiscrepancyImpl(
                        board,
                        stockfish_path,
                        original_fen,
                        depth - 1,
                        line
                    );

                line.pop_back();
                board.UnmakeMove(*selected_move, undo);

                // Prefer the deepest discrepancy.
                if (deeper_discrepancy)
                    return deeper_discrepancy;

                return discrepancy;
            }
            catch (...) {
                line.pop_back();
                board.UnmakeMove(*selected_move, undo);
                throw;
            }
        }

        return std::nullopt;
    }
} // namespace

PerftDivideResult GetStockfishPerftDivide(
    const std::string& stockfish_path,
    const std::string& fen,
    int depth,
    const std::vector<std::string>& moves
) {
    if (depth < 1)
        throw std::invalid_argument("Depth must be at least 1");

    int parent_to_child[2];
    int child_to_parent[2];

    if (pipe(parent_to_child) == -1)
        throw std::runtime_error(std::string("Failed to create input pipe: ") + std::strerror(errno));

    if (pipe(child_to_parent) == -1) {
        close (parent_to_child[0]);
        close (parent_to_child[1]);

        throw std::runtime_error(std::string("Failed to create output pipe ") + std::strerror(errno));
    }

    const pid_t child_pid = fork();

    if (child_pid == -1) {
        close (parent_to_child[0]);
        close (parent_to_child[1]);
        close (child_to_parent[0]);
        close (child_to_parent[1]);

        throw std::runtime_error(std::string("fork failed: ") + std::strerror(errno));
    }

    if (child_pid == 0) {
        if (dup2(parent_to_child[0], STDIN_FILENO) == -1)
            _exit(126);

        if (dup2(child_to_parent[1], STDOUT_FILENO) == -1)
            _exit(126);

        if (dup2(child_to_parent[1], STDERR_FILENO) == -1)
            _exit(126);

        close(parent_to_child[0]);
        close(parent_to_child[1]);
        close(child_to_parent[0]);
        close(child_to_parent[1]);

        execlp(
            stockfish_path.c_str(),
            stockfish_path.c_str(),
            static_cast<char*>(nullptr)
        );

        _exit(127);
    }

    // Parent does not use these ends.
    close(parent_to_child[0]);
    close(child_to_parent[1]);

    FILE* stockfish_input = fdopen(parent_to_child[1], "w");
    FILE* stockfish_output = fdopen(child_to_parent[0], "r");

    if (stockfish_input == nullptr || stockfish_output == nullptr) {
        throw std::runtime_error("fdopen failed");
    }

    std::string position_command;

    if (fen.empty() || fen == "startpos")
        position_command = "position startpos";
    else
        position_command = "position fen " + fen;

    if (!moves.empty()) {
        position_command += " moves";

        for (const std::string& move : moves)
            position_command += " " + move;
    }

    SendCommand(stockfish_input, position_command);
    SendCommand(
        stockfish_input,
        "go perft " + std::to_string(depth)
    );

    PerftDivideResult result;
    char buffer[4096];

    while (std::fgets(buffer, sizeof(buffer), stockfish_output)) {
        const std::string line = Trim(buffer);

        if (line.empty())
            continue;
        if (line.starts_with("Nodes searched:"))
            break;

        const size_t colon = line.find(":");
        if (colon == std::string::npos)
            continue;

        const std::string move = Trim(line.substr(0, colon));
        const std::string nodes = Trim(line.substr(colon + 1));

        if ((move.size() == 4 || move.size() == 5) && !nodes.empty())
            result[move] = std::stoull(nodes);
    }

    SendCommand(stockfish_input, "quit");
    std::fclose(stockfish_input);
    std::fclose(stockfish_output);

    int status = 0;
    waitpid(child_pid, &status, 0);

    return result;
}

PerftDivideResult GetLightknightPerftDivide(
    lightknight::Board& board,
    int depth
) {
    if (depth < 1)
        throw std::invalid_argument("Depth must be at least 1");

    std::vector<lightknight::Move> moves;
    moves.reserve(256);

    lightknight::movegen::GenerateMoves(board, moves);
    PerftDivideResult result;

    for (const lightknight::Move move : moves) {
        lightknight::UndoMoveInfo undo{};

        board.MakeMove(move, undo);
        const uint64_t nodes = lightknight::Perft(board, depth - 1);
        board.UnmakeMove(move, undo);

        std::ostringstream move_stream;
        move_stream << move;

        result[move_stream.str()] = nodes;
    }

    return result;
}

bool ComparePerftDivideResults(
    const PerftDivideResult& lightknight,
    const PerftDivideResult& stockfish
) {
    std::set<std::string> all_moves;

    for (const auto& [move, nodes] : lightknight)
        all_moves.insert(move);

    for (const auto& [move, nodes] : stockfish)
        all_moves.insert(move);

    std::uint64_t lightknight_total = 0;
    std::uint64_t stockfish_total = 0;

    for (const auto& [move, nodes] : lightknight)
        lightknight_total += nodes;

    for (const auto& [move, nodes] : stockfish)
        stockfish_total += nodes;

    bool equal = true;

    std::cout
        << std::left
        << std::setw(10) << "Move"
        << std::setw(18) << "LightKnight"
        << std::setw(18) << "Stockfish"
        << "Difference\n"
        << std::string(60, '-')
        << '\n';

    for (const std::string& move : all_moves) {
        const auto lightknight_it = lightknight.find(move);
        const auto stockfish_it = stockfish.find(move);

        std::cout << std::left << std::setw(10) << move;

        if (lightknight_it == lightknight.end()) {
            std::cout
                << std::setw(18) << "<missing>"
                << std::setw(18) << stockfish_it->second
                << "<missing in LightKnight>\n";

            equal = false;
            continue;
        }

        if (stockfish_it == stockfish.end()) {
            std::cout
                << std::setw(18) << lightknight_it->second
                << std::setw(18) << "<missing>"
                << "<extra in LightKnight>\n";

            equal = false;
            continue;
        }

        const std::uint64_t lightknight_nodes =
            lightknight_it->second;

        const std::uint64_t stockfish_nodes =
            stockfish_it->second;

        std::cout
            << std::setw(18) << lightknight_nodes
            << std::setw(18) << stockfish_nodes;

        if (lightknight_nodes == stockfish_nodes) {
            std::cout << "0\n";
        }
        else if (lightknight_nodes > stockfish_nodes) {
            std::cout
                << '+'
                << lightknight_nodes - stockfish_nodes
                << '\n';

            equal = false;
        }
        else {
            std::cout
                << '-'
                << stockfish_nodes - lightknight_nodes
                << '\n';

            equal = false;
        }
    }

    std::cout
        << std::string(60, '-')
        << "\nLightKnight total: " << lightknight_total
        << "\nStockfish total:   " << stockfish_total
        << "\nDifference:        ";

    if (lightknight_total >= stockfish_total)
        std::cout << '+' << lightknight_total - stockfish_total;
    else
        std::cout << '-' << stockfish_total - lightknight_total;

    std::cout
        << "\nResult:            "
        << (equal ? "MATCH" : "MISMATCH")
        << '\n';

    return equal;
}

bool TraceFirstPerftDiscrepancy(
    lightknight::Board& board,
    const std::string& stockfish_path,
    const std::string& original_fen,
    int depth
) {
    if (depth < 1) {
        throw std::invalid_argument(
            "Depth must be at least 1"
        );
    }

    // Compute
    std::vector<std::string> line;
    const auto discrepancy =
        TraceFirstPerftDiscrepancyImpl(
            board,
            stockfish_path,
            original_fen,
            depth,
            line
        );

    // Print.
    if (!discrepancy) {
        std::cout << "No discrepancy found.\n";
        return false;
    }
    std::cout
        << "\nFinal discrepancy\n"
        << "Line:         ";

    PrintLine(discrepancy->line);

    if (!discrepancy->line.empty())
        std::cout << ' ';

    std::cout
        << discrepancy->move
        << "\nDepth:        "
        << discrepancy->depth
        << "\nMove:         "
        << discrepancy->move
        << "\nLightKnight:  ";

    if (discrepancy->lightknight_nodes)
        std::cout << *discrepancy->lightknight_nodes;
    else
        std::cout << "<missing>";

    std::cout << "\nStockfish:    ";

    if (discrepancy->stockfish_nodes)
        std::cout << *discrepancy->stockfish_nodes;
    else
        std::cout << "<missing>";

    std::cout << '\n';

    return true;
}

int main(int argc, char* argv[]) {
    // Parse arguments.
    bool trace_mode = false;
    int argument_offset = 1;

    if (argc >= 2 && std::string(argv[1]) == "--trace") {
        trace_mode = true;
        argument_offset = 2;
    }

    if (argc != argument_offset + 2) {
        std::cerr
            << "Usage:\n"
            << "  " << argv[0]
            << " [--trace] \"<fen>\" <depth>\n";

        return 1;
    }

    const std::string fen =
        argv[argument_offset];

    const int depth =
        std::stoi(argv[argument_offset + 1]);

    // Path to stockfish.
    const std::string stockfish_path =
        "/usr/games/stockfish";

    lightknight::Board board(fen);

    // Trace mode.
    if (trace_mode) {
        TraceFirstPerftDiscrepancy(
            board,
            stockfish_path,
            fen,
            depth
        );

        // Tracing found and displayed the bug successfully.
        return 0;
    }

    // No trace mode.
    const PerftDivideResult lightknight_result =
        GetLightknightPerftDivide(board, depth);

    const PerftDivideResult stockfish_result =
        GetStockfishPerftDivide(
            stockfish_path,
            fen,
            depth
        );

    const bool matches =
        ComparePerftDivideResults(
            lightknight_result,
            stockfish_result
        );

    return 0;
}