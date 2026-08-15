#ifndef LIGHTKNIGHT_TUNER_TEXEL_H
#define LIGHTKNIGHT_TUNER_TEXEL_H

#include "params.h"
#include "position_dataset.h"

namespace lightknight::tuner {
    double TexelLoss(
        const PositionDataset& dataset,
        const parameters::EngineParameters& params,
        double k
    );
}

#endif // LIGHTKNIGHT_TUNER_TEXEL_H