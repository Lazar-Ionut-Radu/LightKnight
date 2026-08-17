#ifndef LIGHTKNIGHT_TUNER_SPSA_H
#define LIGHTKNIGHT_TUNER_SPSA_H

#include <random>
#include "params.h"
#include "position_dataset.h"

namespace lightknight::tuner {
    parameters::EngineParameters SPSA(
        const PositionDataset& dataset,
        const parameters::EngineParameters& params,
        std::mt19937& rng,
        size_t n_iters,
        double texel_k,
        double A,
        double a,
        double c,
        double alpha = 0.602,
        double gamma = 0.101
    );
} // namespace lightknight::tuner

#endif // LIGHTKNIGHT_TUNER_SPSA_H