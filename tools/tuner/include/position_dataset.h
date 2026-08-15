#ifndef LIGHTKNIGHT_TUNER_POSITION_DATASET_H
#define LIGHTKNIGHT_TUNER_POSITION_DATASET_H

#include "board.h"

#include <vector>
#include <string>

namespace lightknight::tuner {
    struct PositionResult {
        Board board;
        int wins;
        int draws;
        int losses;
    };

    using PositionDataset = std::vector<PositionResult>;
    PositionDataset LoadPositionDataset(const std::string& path);
}; // namespace lightknight::tuner

#endif // LIGHTKNIGHT_TUNER_POSITION_DATASET_H