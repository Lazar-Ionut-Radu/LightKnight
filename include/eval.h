#ifndef LIGHTKNIGHT_EVAL_H
#define LIGHTKNIGHT_EVAL_H

#include <cstddef>
#include "board.h"
#include "types.h"

namespace lightknight::eval {
    int kPieceValues[lightknight::kNumPieces] = {
        +100, +300, +320, +500, +900, 0,
        -100, -300, -320, -500, -900, 0,
        0
    };

    int EvaluateMaterial(lightknight::Board& board);
    int Evaluate(lightknight::Board& board);

} // namespace lightknight::eval

#endif // LIGHTKNIGHT_EVAL_H
