#ifndef LIGHTKNIGHT_EVAL_H
#define LIGHTKNIGHT_EVAL_H

#include <cstddef>
#include "board.h"
#include "types.h"

namespace lightknight::eval {
    // From white's perspective.
    inline constexpr int kPieceValues[lightknight::kNumPieces] = {
        +100, +300, +320, +500, +900, 0,
        -100, -300, -320, -500, -900, 0,
        0
    };

    // From white's perspective
    int EvaluateMaterial(lightknight::Board& board);
    
    // From the perspective of the player whose turn it is.
    int Evaluate(lightknight::Board& board);
} // namespace lightknight::eval

#endif // LIGHTKNIGHT_EVAL_H
