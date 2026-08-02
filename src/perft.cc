// perft.cc
#include "perft.h"
#include "board.h"
#include "movegen.h"

#include <cassert>
#include <cstddef>
#include <vector>

namespace lightknight {
    namespace {
        uint64_t PerftImplementation(
            Board& board,
            int depth,
            size_t current_depth,
            bool bulk_count,
            std::vector<std::vector<Move>>& move_lists
        ) {
            if (depth == 0)
                return 1;

            std::vector<Move> &moves = move_lists[current_depth];
            moves.clear();

            const size_t move_count = lightknight::movegen::GenerateMoves<lightknight::movegen::MoveGenType::kAll>(board, moves);
            uint64_t nodes = 0;

            if (bulk_count && depth == 1)
                return move_count;

            for (size_t idx = 0; idx < move_count; ++idx) {
                const Move move = moves[idx];

                UndoMoveInfo undo{};

                board.MakeMove(move, undo);
                nodes += PerftImplementation(board, depth - 1, current_depth + 1, bulk_count, move_lists);
                board.UnmakeMove(move, undo);
            }

            return nodes;
        }
    } // namespace

    uint64_t Perft(Board& board, int depth, bool bulk_count) {
        if (depth < 0)
            throw std::invalid_argument("Perft depth cannot be negative");

        if (depth == 0)
            return 1;

        // Prealocate vectors to not do it million of times inside the search.
        std::vector<std::vector<Move>> move_lists(
            static_cast<size_t>(depth)
        );
        for (std::vector<Move>& moves : move_lists)
            moves.reserve(256);

        return PerftImplementation(board, depth, 0, bulk_count, move_lists);
    }
} // namespace lightknight