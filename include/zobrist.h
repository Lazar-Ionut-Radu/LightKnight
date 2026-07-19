#ifndef LIGHTKNIGHT_ZOBRIST_H
#define LIGHTKNIGHT_ZOBRIST_H

#include <cstddef>
#include <cstdint>
#include "prng.h"
#include "types.h"

namespace lightknight {
    // Zobrist hashes
    struct ZobristHashes {
        uint64_t piece_square[12][64];
        uint64_t turn;
        uint64_t castling[9];
        uint64_t en_passant[64];
    };

    consteval ZobristHashes GenerateZobristHashes(uint64_t seed) {
        // Pseudorandom numvber generator
        SplitMix64 prng(seed);
        ZobristHashes zh{};

        // Piece-square hashes
        for (auto& v : zh.piece_square)
            for (auto& e : v)
                e = prng.next();

        // Hash for the turn
        zh.turn = prng.next();
    
        // Castles
        for (uint64_t& i : zh.castling)
            i = prng.next();

        // En Passant Square
        for (uint64_t& i : zh.en_passant)
            i = prng.next();

        return zh;
    }

    // Zobrist hashes generated at compile time.
    static constexpr ZobristHashes zobrists = GenerateZobristHashes(0x42ull);
} // namespace lightknight

#endif // LIGHTKNIGHT_ZOBRIST_H