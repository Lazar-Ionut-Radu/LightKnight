// tools/tuner/src/spsa.cc
#include "spsa.h"
#include "params.h"
#include "texel.h"
#include "position_dataset.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>
#include <iostream>

namespace lightknight::tuner {
    parameters::EngineParameters SPSA(
        const PositionDataset& dataset,
        const parameters::EngineParameters& params,
        size_t n_iters,
        double texel_k,
        double a,
        double c,
        std::mt19937& rng,
        double alpha,
        double gamma
    ) {
        parameters::EngineParameters current = params;
        auto parameter_info = parameters::GetParameterInfoList(current, true);

        std::uniform_int_distribution<int> bernoulli(0, 1);

        for (size_t iter = 0; iter < n_iters; ++iter) {
            // Compute scheduling for the params.
            const double a_k = a / std::pow(iter + 1.0, alpha);
            const double c_k = c / std::pow(iter + 1.0, gamma);

            // Get the 2 samples to tests
            parameters::EngineParameters plus = current;
            parameters::EngineParameters minus = current;
            auto plus_info = parameters::GetParameterInfoList(plus, true);
            auto minus_info = parameters::GetParameterInfoList(minus, true);

            std::vector<int> delta(parameter_info.size());
            for (size_t i = 0; i < parameter_info.size(); ++i) {
                delta[i] = bernoulli(rng) ? 1 : -1;

                *plus_info[i].value =
                    std::clamp(
                        static_cast<int>(*plus_info[i].value + c_k * delta[i]),
                        plus_info[i].min,
                        plus_info[i].max
                    );

                *minus_info[i].value =
                    std::clamp(
                        static_cast<int>(*minus_info[i].value - c_k * delta[i]),
                        minus_info[i].min,
                        minus_info[i].max
                    );
            }

            // Compute the loss
            const double loss_plus = TexelLoss(dataset, plus, texel_k);
            const double loss_minus = TexelLoss(dataset, minus, texel_k);
            const double loss_difference = loss_plus - loss_minus;

            // Compute the gradient and update parameters.
            for (size_t i = 0; i < parameter_info.size(); ++i) {
                const double gradient = loss_difference / (2.0 * c_k * delta[i]);
                const int old_value = *parameter_info[i].value;
                int new_value = static_cast<int>(std::round(old_value - a_k * gradient));

                new_value = std::clamp(
                    new_value,
                    parameter_info[i].min,
                    parameter_info[i].max
                );

                *parameter_info[i].value = new_value;
            }

            // Print some info once in a while.
            if ((iter + 1) % 10 == 0) {
                const double loss = TexelLoss(dataset, current, texel_k);

                std::cout
                    << "Iteration " << iter + 1
                    << ": loss = " << loss
                    << '\n';
            }
        }

        return current;
    }
} // namespace lightknight::tuner