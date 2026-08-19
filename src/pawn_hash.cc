// pawn_hash.cc
#include "pawn_hash.h"

namespace lightknight::eval {
    size_t PawnHash::Index(uint64_t zobrist) const {
        return zobrist % _entries.size();
    }

    PawnHashEntry& PawnHash::operator[](uint64_t zobrist) {
        return _entries[this->Index(zobrist)];
    }

    const PawnHashEntry& PawnHash::operator[](uint64_t zobrist) const {
        return _entries[this->Index(zobrist)];
    }

    PawnHashEntry* PawnHash::Probe(uint64_t zobrist) {
        PawnHashEntry& entry = (*this)[zobrist];
    
        if (!entry.valid || entry.zobrist != zobrist)
            return nullptr;
    
        return &entry;
    }

    const PawnHashEntry* PawnHash::Probe(uint64_t zobrist) const {
        const PawnHashEntry &entry = (*this)[zobrist];
    
        if (entry.valid && entry.zobrist == zobrist)
            return &entry;

        return nullptr;
    }
    
    void PawnHash::ResizeKB(size_t size_kb) {
        constexpr std::size_t bytes_per_kb = 1024ull;
        const size_t num_entries = size_kb * bytes_per_kb / sizeof(PawnHashEntry);

        this->_entries.assign(num_entries, PawnHashEntry{});
    }

    void PawnHash::ResizeCounts(size_t size) {
        this->_entries.assign(size, PawnHashEntry{});
    }

    void PawnHash::Clear() {
        std::fill(_entries.begin(), _entries.end(), PawnHashEntry{});
    }

    size_t PawnHash::Size() const {
        return _entries.size();
    }

    void PawnHash::Store(uint64_t zobrist, PawnHashEntry& entry) {
        PawnHashEntry& slot = (*this)[zobrist];

        slot = entry;
        slot.zobrist = zobrist;
        slot.valid = true;
    }

} // namespace lightknight::eval