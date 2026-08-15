// tools/tuner/src/main.cc
#include <iostream>
#include <string>
#include <random>

#include "params.h"
#include "position_dataset.h"
#include "texel.h"
#include "hill_climbing.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr
            << "Usage: tuner <positions.csv> <params.csv>\n";
        return 1;
    }
    const std::string positions_path = argv[1];
    const std::string params_path = argv[2];

    // Load position dataset.
    const auto dataset = lightknight::tuner::LoadPositionDataset(positions_path);

    // Load initial params.
    lightknight::parameters::EngineParameters params{};
    lightknight::parameters::LoadParameters(params, params_path);

    // Rng
    std::mt19937 rng(42);

    // Params
    constexpr double texel_k = 0.005;
    
    
    lightknight::parameters::EngineParameters tuned_params;
    tuned_params = lightknight::tuner::StochasticHillClimbing(
        dataset,
        params,
        {-1, 1},
        rng,
        100,
        1,
        texel_k,
        false
    );
    lightknight::parameters::SaveParameters(tuned_params, "tuned_params.csv");

    return 0;
}