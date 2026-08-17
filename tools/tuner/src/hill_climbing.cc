// tools/tuner/src/hill_climbing.cc
#include "hill_climbing.h"
#include "position_dataset.h"
#include "texel.h"
#include "params.h"

#include <vector>
#include <algorithm>
#include <random>
#include <cstddef>
#include <numeric>

namespace lightknight::tuner {
    std::vector<int> GetRandomShuffle(size_t size, std::mt19937& rng) {
        std::vector<int> indices(size);
        std::iota(indices.begin(), indices.end(), 0);
        std::shuffle(indices.begin(), indices.end(), rng);

        return indices;
    }

    parameters::EngineParameters GetRandomParameters(std::mt19937& rng) {
        parameters::EngineParameters new_parameters{};
        auto parameters_vector = parameters::GetParameterInfoList(new_parameters, true);
        
        for (auto& parameter : parameters_vector) {
            std::uniform_int_distribution<int> distribution(parameter.min, parameter.max);
            *parameter.value = distribution(rng);
        }

        return new_parameters;
    }

    parameters::EngineParameters StochasticHillClimbing(
        const PositionDataset& dataset,
        const parameters::EngineParameters& params,
        const std::vector<int>& offsets,
        std::mt19937& rng,
        size_t n_iters,
        size_t n_restarts,
        double texel_k,
        bool maximize
    ) {
        if (offsets.empty() || n_restarts == 0)
            return params;

        // Keep the best parameters found across all restarts.
        parameters::EngineParameters best_params = params;
        double best_value = TexelLoss(dataset, params, texel_k);

        // Run the specified number of hill climbing searches.
        for (size_t restart = 0; restart < n_restarts; ++restart) {
            // For the first run, keep those same parameters, otherwise randomize them.
            parameters::EngineParameters curr_params;
            if (restart > 0)
                curr_params = GetRandomParameters(rng);
            else
                curr_params = params;

            // Get the list of parameters. 
            auto params_vector = parameters::GetParameterInfoList(curr_params, true);

            // Compute the current value.
            double curr_value = TexelLoss(dataset, curr_params, texel_k);
            
            // Printing some info.
            std::cout << "Restart " << restart + 1 << "/" << n_restarts
                      << ": starting loss = " << curr_value << "\n";
                
            // Num of neighbouring states to this one.
            const size_t num_offsets = offsets.size();
            const size_t num_neighbours = params_vector.size() * num_offsets;

            // Run for the specified number of iterations.
            size_t n_iterations_done = 0;
            for (size_t iter = 0; iter < n_iters; ++iter) {
                bool improved = false;
                
                // Get a randomized list of indices with which we choose a neighbour.
                // Each index uniquely identifies:
                //
                //     parameter_index = index / num_offsets
                //     offset_index    = index % num_offsets
                //
                std::vector<int> indices = GetRandomShuffle(num_neighbours, rng);
                
                // Iterate through the neighbours.
                for (size_t neighbour = 0; neighbour < num_neighbours; ++neighbour) {
                    // Find the change needed to be done for this neighbour.
                    size_t neighbour_idx = indices[neighbour];
                    size_t param_idx = neighbour_idx / num_offsets;
                    size_t offset_idx = neighbour_idx % num_offsets;
                    int param_old_value = *params_vector[param_idx].value;
                    int param_new_value = *params_vector[param_idx].value + offsets[offset_idx];

                    // Check it keeps the param within its bounds.
                    if (param_new_value > params_vector[param_idx].max ||
                        param_new_value < params_vector[param_idx].min)
                        continue;
                    
                    // Apply the change to the parameter, so we move to the neighbour state.
                    *params_vector[param_idx].value = param_new_value;

                    // If this state is better, move to it immediately.
                    double neighbour_value = TexelLoss(dataset, curr_params, texel_k);
                    bool is_better = maximize 
                        ? (neighbour_value > curr_value)
                        : (neighbour_value < curr_value);
                    
                    if (is_better) {
                        curr_value = neighbour_value;
                        improved = true;
                        break;
                    }
                    
                    // Undo the change, moving back to the state this iteration began on.
                    *params_vector[param_idx].value = param_old_value;
                }
                n_iterations_done++;

                // If no neighbour is better than stop this run.
                if (!improved) {
                    break;
                }

                // Print some info.
                if (n_iterations_done % 10 == 0) {
                    std::cout << "Iteration " << n_iterations_done
                              << ": current loss = " << curr_value << "\n";

                    parameters::SaveParameters(curr_params, "./tuned_params.csv");
                }
            }
            
            // Update the best parameters found if this run improved them
            bool is_better = maximize
                ? curr_value > best_value
                : curr_value < best_value;

            if (is_better) {
                best_value = curr_value;
                best_params = curr_params;
            }

            // Print some info
            std::cout << "Restart " << restart + 1 << "/" << n_restarts
                      << ": final loss = " << curr_value
                      << ", num iterations = " << n_iterations_done;
            if (is_better)
                std::cout << " (new best)";
            std::cout << "\n";
        }
        
        std::cout << "Best loss = " << best_value << '\n';
        return best_params;
    }

    parameters::EngineParameters SteepestHillClimbing(
        const PositionDataset& dataset,
        const parameters::EngineParameters& params,
        const std::vector<int>& offsets,
        std::mt19937& rng,
        size_t n_iters,
        size_t n_restarts,
        double texel_k,
        bool maximize
    ) {
        if (offsets.empty() || n_restarts == 0)
            return params;

        // Keep the best parameters found across all restarts.
        parameters::EngineParameters best_params = params;
        double best_value = TexelLoss(dataset, params, texel_k);

        // Run the specified number of hill climbing searches.
        for (size_t restart = 0; restart < n_restarts; ++restart) {
             // For the first run, keep those same parameters, otherwise randomize them.
            parameters::EngineParameters curr_params;
            if (restart > 0)
                curr_params = GetRandomParameters(rng);
            else
                curr_params = params;

            // Get the list of parameters. 
            auto params_vector = parameters::GetParameterInfoList(curr_params, true);

            // Compute the current value.
            double curr_value = TexelLoss(dataset, curr_params, texel_k);
            
            // Printing some info.
            std::cout << "Restart " << restart + 1 << "/" << n_restarts
                      << ": starting loss = " << curr_value << "\n";

            // Num of neighbouring states to this one.
            const size_t num_offsets = offsets.size();
            const size_t num_neighbours = params_vector.size() * num_offsets;

            // Run for the specified number of iterations.
            size_t n_iterations_done = 0;
            for (size_t iter = 0; iter < n_iters; ++iter) {
                // Keep track of the best neighbour.
                parameters::EngineParameters best_neighbour_params = curr_params;
                double best_neighbour_value = curr_value;
                
                // Iterate through the neighbours
                bool improved = false;
                for (size_t neighbour = 0; neighbour < num_neighbours; ++neighbour) {
                    // Find the change needed to be done for this neighbour.
                    size_t param_idx = neighbour / num_offsets;
                    size_t offset_idx = neighbour % num_offsets;
                    int param_old_value = *params_vector[param_idx].value;
                    int param_new_value = *params_vector[param_idx].value + offsets[offset_idx];

                    // Check it keeps the param within its bounds.
                    if (param_new_value > params_vector[param_idx].max ||
                        param_new_value < params_vector[param_idx].min)
                        continue;
                        
                    // Apply the change to the parameter, so we move to the neighbour state.
                    *params_vector[param_idx].value = param_new_value;

                    // If this state is better, move to it immediately.
                    double neighbour_value = TexelLoss(dataset, curr_params, texel_k);
                    bool is_better = maximize 
                        ? (neighbour_value > curr_value)
                        : (neighbour_value < curr_value);
                        
                    if (is_better) {
                        best_neighbour_value = neighbour_value;
                        best_neighbour_params = curr_params;
                        improved = true;
                    }
                        
                    // Undo the change, moving back to the state this iteration began on.
                    *params_vector[param_idx].value = param_old_value;
                }
                n_iterations_done++;

                // If no neighbour is better than stop this run.
                if (!improved) {
                    break;
                }
                // Move to the best neighbour.
                else {
                    curr_params = best_neighbour_params;
                    curr_value = best_neighbour_value;
                }

                // Print some info.
                if (n_iterations_done % 10 == 0) {
                    std::cout << "Iteration " << n_iterations_done
                              << ": current loss = " << curr_value << "\n";

                    parameters::SaveParameters(curr_params, "./tuned_params.csv");
                }
            }

            // Update the best parameters found if this run improved them
            bool is_better = maximize
                ? curr_value > best_value
                : curr_value < best_value;

            if (is_better) {
                best_value = curr_value;
                best_params = curr_params;
            }

            // Print some info
            std::cout << "Restart " << restart + 1 << "/" << n_restarts
                      << ": final loss = " << curr_value
                      << ", num iterations = " << n_iterations_done;
            if (is_better)
                std::cout << " (new best)";
            std::cout << "\n";
        }
        
        std::cout << "Best loss = " << best_value << '\n';
        return best_params;
    }
} // namespace lightknight::tuner
