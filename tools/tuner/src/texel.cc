// tools/tuner/src/texel.cc
#include "texel.h"
#include "eval.h"

#include <cmath>

namespace lightknight::tuner {
    double TexelLoss(
        const PositionDataset& dataset,
        const parameters::EngineParameters& params,
        double k
    ) {
        double loss = 0.0;
        int total_positions = 0;

        for (const PositionResult& position : dataset) {
            // Evaluate the position.
            int evaluation = eval::Evaluate(position.board, params);

            // Convert evaluation to side-to-move perspective.
            if (position.board.turn == Color::kBlack)
                evaluation = -evaluation;

            // A scaled softmax gets us a prediction in [0, 1]
            const double prediction = 1.0 / (1.0 + std::exp(-k * evaluation));

            // Compute the loss terms coming from wins, draws and losses. It's just the
            // L2 loss
            loss += position.wins * (prediction - 1.0) * (prediction - 1.0);
            loss += position.draws * (prediction - 0.5) * (prediction - 0.5);
            loss += position.losses * prediction * prediction;
            
            total_positions += position.wins + position.draws + position.losses;
        }

        return loss / total_positions;
    }

}  // namespace lightknight::tuner