#ifndef LIGHTKNIGHT_PAWN_HASH_H
#define LIGHTKNIGHT_PAWN_HASH_H

#include <cstdint>
#include <cstddef>
#include <vector>

#include "types.h"
namespace lightknight::eval {

    struct PawnHashEntry {
        uint64_t zobrist = 0;
        uint64_t passed_pawns_bb[kNumColors]{0};
        uint64_t isolated_pawns_bb[kNumColors]{0};
        uint64_t connected_pawns_bb[kNumColors]{0};
        uint64_t protected_pawns_bb[kNumColors]{0};
        int pawn_eval[2]{0};
        bool valid = false;
    };

    class PawnHash {
    private:
        std::vector<PawnHashEntry> _entries;
        
        size_t Index(uint64_t zobrist) const;

    public:
        explicit PawnHash(size_t size_kb) {
            this->ResizeKB(size_kb);
        }

        void ResizeKB(size_t size_kb);
        void ResizeCounts(size_t size);
        void Clear();
        
        PawnHashEntry& operator[](uint64_t zobrist);
        const PawnHashEntry& operator[](uint64_t zobrist) const;

        PawnHashEntry* Probe(uint64_t zobrist);
        const PawnHashEntry* Probe(uint64_t zobrist) const;

        size_t Size() const;

        void Store(uint64_t zobrist, PawnHashEntry& entry);
    };

} // namespace lightknight::eval

#endif // LIGHTKNIGHT_PAWN_HASH_H