#ifndef LIGHTKNIGHT_TRANSPOSITION_TABLE_H
#define LIGHTKNIGHT_TRANSPOSITION_TABLE_H

#include <cstddef>
#include <cstdint>
#include <vector>

#include "types.h"

namespace lightknight::search {
    enum class TTBound : uint8_t {
        Exact, // Corresponding to PV Nodes.
        Lower, // Corresponding to Cut Nodes.
        Upper  // Corresponding to All Nodes.
    };

    struct TTEntry {
        uint64_t zobrist = 0;
        int32_t score = 0;
        Move move{};
        TTBound bound = TTBound::Exact;
        uint8_t depth = -1;
        bool valid = false;
    };

    class TranspositionTable {
    private:
        std::vector<TTEntry> _entries;
        
        size_t Index(uint64_t zobrist) const;
        bool ShouldReplace(
            const TTEntry& old_entry,
            int new_depth,
            TTBound new_bound
        ) const;

    public:
        explicit TranspositionTable(size_t size_mb = 16) {
            this->ResizeMB(size_mb);
        }

        void ResizeMB(size_t size_mb);
        void ResizeCounts(size_t size);
        void Clear();

        TTEntry& operator[](uint64_t zobrist);
        const TTEntry& operator[](uint64_t zobrist) const;

        TTEntry* Probe(uint64_t zobrist);
        const TTEntry* Probe(uint64_t zobrist) const;

        size_t Size() const;

        void Store(
            uint64_t zobrist,
            int32_t score,
            uint8_t depth,
            Move move,
            TTBound bound
        );
    };
} // namespace lightknight::movegen

#endif // LIGHTKNIGHT_TRANSPOSITION_TABLE_H