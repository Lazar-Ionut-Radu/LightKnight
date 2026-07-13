#ifndef LIGHTKNIGHT_PERFT_DEBUG_H
#define LIGHTKNIGHT_PERFT_DEBUG_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>

namespace lightknight {
class Board;
};

using PerftDivideResult = std::unordered_map<std::string, uint64_t>;

PerftDivideResult GetLightknightPerftDivide(
    lightknight::Board& board,
    int depth
);

struct PerftDiscrepancy {
    std::vector<std::string> line;
    int depth;
    std::string move;
    std::optional<std::uint64_t> lightknight_nodes;
    std::optional<std::uint64_t> stockfish_nodes;
};

PerftDivideResult GetStockfishPerftDivide(
    const std::string& stockfish_path,
    const std::string& fen,
    int depth,
    const std::vector<std::string>& moves = {}
);

bool ComparePerftDivideResults(
    const PerftDivideResult& lightknight,
    const PerftDivideResult& stockfish
);

bool TraceFirstPerftDiscrepancy(
    lightknight::Board& board,
    const std::string& stockfish_path,
    const std::string& original_fen,
    int depth
);

#endif // LIGHTKNIGHT_PERFT_DEBUG_H