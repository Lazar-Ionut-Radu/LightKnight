#ifndef LIGHTKNIGHT_PERFT_H
#define LIGHTKNIGHT_PERFT_H

#include <cstdint>

namespace lightknight {
    class Board;

    uint64_t Perft(Board& board, int depth, bool bulk_count = false);
} // namespace lightknight

#endif // LIGHTKNIGHT_PERFT_H