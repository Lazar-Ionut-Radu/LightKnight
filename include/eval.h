#ifndef LIGHTKNIGHT_EVAL_H
#define LIGHTKNIGHT_EVAL_H

#include <cstddef>
#include "board.h"
#include "types.h"
#include "params.h"

namespace lightknight::eval {
    inline constexpr size_t kNumGamePhases = 2;
    enum GamePhase : uint8_t {
        kMG,
        kEG
    };

    // https://chessprogramming.org/Tapered_Eval
    // Weights for computing the game phase for tapered eval.
    inline constexpr int kGamePhaseWeights[lightknight::kNumPieces] = {
        0, 1, 1, 2, 4, 0, 0, 1, 1, 2, 4, 0, 0
    };
    inline constexpr int kTotalGamePhase = kGamePhaseWeights[Piece::kWhitePawn] * 16 
        + kGamePhaseWeights[Piece::kWhiteKnight] * 4 + kGamePhaseWeights[Piece::kWhiteBishop] * 4
        + kGamePhaseWeights[Piece::kWhiteRook] * 4 + kGamePhaseWeights[Piece::kWhiteQueen] * 2;

    // Tapered eval.
    int ComputeGamePhase(const Board& board);
    int ComputeWeightedEval(int phase, int mg_eval, int eg_eval);

    // Helpers.
    template <Piece piece_type>
    int GetPieceMobility(const Board& board, Square piece_sq);
    uint64_t PassedPawnsBB(const Board& board, Color color);
    uint64_t IsolatedPawnsBB(const Board& board, Color color);
    uint64_t ProtectedPawnsBB(const Board& board, Color color);
    uint64_t ConnectedPawnsBB(const Board& board, Color color);

    // From white's perspective.
    int EvaluateMaterial_(const Board& board, GamePhase game_phase, const parameters::EngineParameters& params);
    int EvaluateMaterial(const Board& board, int phase_weight, const parameters::EngineParameters& params);
    int EvaluatePieceSquare_(const Board& board, GamePhase game_phase, const parameters::EngineParameters& params);
    int EvaluatePieceSquare(const Board& board, int phase_weight, const parameters::EngineParameters& params);
    int EvaluateMobility_(const Board& board, GamePhase game_phase, const parameters::EngineParameters& params);
    int EvaluateMobility(const Board& board, int phase_weight, const parameters::EngineParameters& params);
    int EvaluatePawns_(const Board& board, GamePhase game_phase, const parameters::EngineParameters& params);
    int EvaluatePawns(const Board& board, int phase_weight, const parameters::EngineParameters& params);
    int EvaluateKings_(const Board& board, GamePhase game_phase, const parameters::EngineParameters& params);
    int EvaluateKings(const Board& board, int phase_weight, const parameters::EngineParameters& params);

    // From the perspective of the player whose turn it is.
    int Evaluate_(const Board& board, GamePhase game_phase, const parameters::EngineParameters& params);
    int Evaluate(const Board& board, const parameters::EngineParameters& params);
} // namespace lightknight::eval

#endif // LIGHTKNIGHT_EVAL_H
