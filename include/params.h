#ifndef LIGHTKNIGHT_PARAMS_H
#define LIGHTKNIGHT_PARAMS_H

#include <types.h>

namespace lightknight::params {
    struct EngineParameters {
        // Evaluation parameters
        int eval_piece_values[2][kNumPieces];
        int eval_psqt[2][6][kNumSquares];
        int eval_mobility[2][5][28];
        int eval_passed_pawns[2][lightknight::kNumSquares];
        int eval_isolated_pawns[2][lightknight::kNumSquares];
        int eval_doubled_pawns[2];
        int eval_tripled_pawns[2];
        int eval_protected_pawn[2][8];
        int eval_connected_pawn[2][8];
        int eval_king_pawn_shield[2][2];
        int eval_tempo[2];
        int eval_bishop_pair[2];

        // Transposition table
        int tt_size_mb;
        
        // Constructor with default values.
        EngineParameters();
    };
} // namespace params

#endif // LIGHTKNIGHT_PARAMS_H