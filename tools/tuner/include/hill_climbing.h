#ifndef LIGHTKNIGHT_TUNER_HILL_CLIMBING_H
#define LIGHTKNIGHT_TUNER_HILL_CLIMBING_H

#include "params.h"
#include "position_dataset.h"

#include <cstddef>
#include <vector>
#include <random>

namespace lightknight::tuner {
    std::vector<int> GetRandomShuffle(size_t size, std::mt19937& rng);
    parameters::EngineParameters GetRandomParameters(std::mt19937& rng);

    parameters::EngineParameters StochasticHillClimbing(
        const PositionDataset& dataset,
        const parameters::EngineParameters& params,
        const std::vector<int>& offsets,
        std::mt19937& rng,
        size_t n_iters,
        size_t n_restarts,
        double texel_k,
        bool maximize = false
    );

    parameters::EngineParameters SteepestHillClimbing(
        const PositionDataset& dataset,
        const parameters::EngineParameters& params,
        const std::vector<int>& offsets,
        std::mt19937& rng,
        size_t n_iters,
        size_t n_restarts,
        double texel_k,
        bool maximize = false
    );

} // namespace lightknight::tuner

#endif // LIGHTNIGHT_TUNER_HILL_CLIMBING_H