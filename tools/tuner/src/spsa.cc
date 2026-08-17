// tools/tuner/src/spsa.cc
#include "spsa.h"
#include "params.h"
#include "texel.h"
#include "position_dataset.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

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
        double alpha,
        double gamma
    ) {
        parameters::EngineParameters current = params;
        auto parameter_info = parameters::GetParameterInfoList(current, true);

        std::uniform_int_distribution<int> bernoulli(0, 1);

        for (size_t iter = 0; iter < n_iters; ++iter) {
            // SPSA parameter schedules.
            const double a_k = a / std::pow(A + iter + 1.0, alpha);
            const double c_k = c / std::pow(iter + 1.0, gamma);

            // Create the two perturbed parameter sets.
            parameters::EngineParameters plus = current;
            parameters::EngineParameters minus = current;
            auto plus_info = parameters::GetParameterInfoList(plus, true);
            auto minus_info = parameters::GetParameterInfoList(minus, true);

            std::vector<int> delta(parameter_info.size());

            // Generate the SPSA perturbation.
            for (size_t i = 0; i < parameter_info.size(); ++i) {
                delta[i] = bernoulli(rng) ? 1 : -1;

                // Perturb the continuous values.
                plus_info[i].continuous_value =
                    std::clamp(
                        parameter_info[i].continuous_value + c_k * delta[i],
                        static_cast<double>(parameter_info[i].min),
                        static_cast<double>(parameter_info[i].max)
                    );

                minus_info[i].continuous_value =
                    std::clamp(
                        parameter_info[i].continuous_value - c_k * delta[i],
                        static_cast<double>(parameter_info[i].min),
                        static_cast<double>(parameter_info[i].max)
                    );

                // Convert the continuous values to the integer values actually used by the engine.
                *plus_info[i].value = std::clamp(
                    static_cast<int>(std::round(plus_info[i].continuous_value)),
                    plus_info[i].min,
                    plus_info[i].max
                );

                *minus_info[i].value = std::clamp(
                    static_cast<int>(std::round(minus_info[i].continuous_value)),
                    minus_info[i].min,
                    minus_info[i].max
                );
            }

            // Evaluate the two perturbed positions.
            const double loss_plus = TexelLoss(dataset, plus, texel_k);
            const double loss_minus = TexelLoss(dataset, minus, texel_k);
            const double loss_difference = loss_plus - loss_minus;

            // Common part of the SPSA gradient estimate.
            const double gradient_scale = loss_difference / (2.0 * c_k);

            // Update the continuous parameters.
            for (size_t i = 0; i < parameter_info.size(); ++i) {
                const double gradient = gradient_scale / delta[i];

                parameter_info[i].continuous_value -= a_k * gradient;

                // Keep the continuous parameter within its bounds.
                parameter_info[i].continuous_value =
                    std::clamp(
                        parameter_info[i].continuous_value,
                        static_cast<double>(parameter_info[i].min),
                        static_cast<double>(parameter_info[i].max)
                    );

                // Convert the continuous value to the integer value used by the engine.
                *parameter_info[i].value = std::clamp(
                    static_cast<int>(std::round(parameter_info[i].continuous_value)),
                    parameter_info[i].min,
                    parameter_info[i].max
                );
            }

            // Print progress.
            if ((iter + 1) % 10 == 0) {
                lightknight::parameters::SaveParameters(current, "tuned_params.csv");
                const double loss = TexelLoss(dataset, current, texel_k);

                std::cout
                    << "Iteration " << iter + 1
                    << ": loss = " << loss
                    << ", a_k = " << a_k
                    << ", c_k = " << c_k
                    << ", update scale = " << a_k * gradient_scale
                    << '\n';
            }
        }

        return current;
    }
} // namespace lightknight::tuner