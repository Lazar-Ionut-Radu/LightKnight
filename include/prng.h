#ifndef LIGHTKNIGHT_PRNG_H
#define LIGHTKNIGHT_PRNG_H

#include <cstddef>
#include <cstdint>
#include <random>

namespace lightknight {
    // https://rosettacode.org/wiki/Pseudo-random_numbers/Splitmix64
    class SplitMix64 {
        private:
            uint64_t state_;
    
        public:
            explicit constexpr SplitMix64(uint64_t seed) : state_(seed) {}

            constexpr uint64_t next() {
                uint64_t z = (state_ += 0x9E3779B97F4A7C15ULL);
                z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
                z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
                return z ^ (z >> 31);
            }
    };
} //namespace lightknight

#endif //LIGHTKNIGHT_PRNG_H