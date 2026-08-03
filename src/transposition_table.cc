// transposition_table.cc
#include "transposition_table.h"

#include <vector> 
#include <cstdint>
#include <cstddef>
#include <algorithm>

namespace lightknight::search {
    size_t TranspositionTable::Index(uint64_t zobrist) const {
        return zobrist % _entries.size();
    }

    bool TranspositionTable::ShouldReplace(
        const TTEntry& old_entry,
        int new_depth,
        TTBound new_bound
    ) const {
        // If its empty
        if (!old_entry.valid)
            return true;

        // Compare depth
        if (new_depth > old_entry.depth)
            return true;

        if (new_depth < old_entry.depth)
            return false;

        // Depths equal.
        // New bound better.
        if (new_bound == TTBound::Exact)
            return true;

        // Both bounds inexact.
        if (old_entry.bound != TTBound::Exact)
            return true;

        return false;
    }

    void TranspositionTable::Store(
        uint64_t zobrist,
        int32_t score,
        uint8_t depth,
        Move move,
        TTBound bound
    ) {
        TTEntry& slot = (*this)[zobrist];

        if (!this->ShouldReplace(slot, depth, bound))
            return;

        slot = TTEntry{
            .zobrist = zobrist,
            .score = score,
            .move = move,
            .bound = bound,
            .depth = depth,
            .valid = true
        };
    }

    TTEntry& TranspositionTable::operator[](uint64_t zobrist) {
        return _entries[this->Index(zobrist)];
    }

    const TTEntry& TranspositionTable::operator[](uint64_t zobrist) const {
        return _entries[this->Index(zobrist)];
    }

    TTEntry* TranspositionTable::Probe(uint64_t zobrist) {
        TTEntry &entry = (*this)[zobrist];
    
        if (entry.valid && entry.zobrist == zobrist)
            return &entry;

        return nullptr;
    }

    const TTEntry* TranspositionTable::Probe(uint64_t zobrist) const {
        const TTEntry &entry = (*this)[zobrist];
    
        if (entry.valid && entry.zobrist == zobrist)
            return &entry;

        return nullptr;
    }
    
    void TranspositionTable::ResizeMB(size_t size_mb) {
        constexpr std::size_t bytes_per_mb = 1024ull * 1024ull;
        const size_t num_entries = size_mb * bytes_per_mb / sizeof(TTEntry);

        this->_entries.assign(num_entries, TTEntry{});
    }

    void TranspositionTable::ResizeCounts(size_t size) {
        this->_entries.assign(size, TTEntry{});
    }

    void TranspositionTable::Clear() {
        std::fill(_entries.begin(), _entries.end(), TTEntry{});
    }

    size_t TranspositionTable::Size() const {
        return _entries.size();
    }
} // namespace lightknight::movegen