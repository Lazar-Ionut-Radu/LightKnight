// params.c
#include <string>
#include <fstream>
#include <vector>

#include "params.h"

namespace lightknight::parameters {
    // TODO: write a tuner so I can find values of my own / not guessed probably totally wrong.

    EngineParameters::EngineParameters() :
        // ---------------------------------------------
        // ----------- Evaluation Parameters -----------
        // ---------------------------------------------

        // Piece values.
        // Credits: https://www.chessprogramming.org/Simplified_Evaluation_Function
        eval_piece_values{ 
            // Midgame
            {100,  320,  330,  500,  900},
            // Endgame
            {100,  320,  330,  500,  900}
        },

        // Piece-Square tables. Bonuses / Penalties for pieces on a particular square.
        // Credits: http://www.talkchess.com/forum3/viewtopic.php?f=2&t=68311&start=19
        eval_psqt {
            { // Midgame
                { // Pawn
                    0,   0,   0,   0,   0,   0,  0,   0,
                    -35,  -1, -20, -23, -15,  24, 38, -22,
                    -26,  -4,  -4, -10,   3,   3, 33, -12,
                    -27,  -2,  -5,  12,  17,   6, 10, -25,
                    -14,  13,   6,  21,  23,  12, 17, -23,
                    -6,   7,  26,  31,  65,  56, 25, -20,
                    98, 134,  61,  95,  68, 126, 34, -11,
                    0,   0,   0,   0,   0,   0,  0,   0,
                },
                { // Knight
                    -105, -21, -58, -33, -17, -28, -19,  -23,
                    -29, -53, -12,  -3,  -1,  18, -14,  -19,
                    -23,  -9,  12,  10,  19,  17,  25,  -16,
                    -13,   4,  16,  13,  28,  19,  21,   -8,
                    -9,  17,  19,  53,  37,  69,  18,   22,
                    -47,  60,  37,  65,  84, 129,  73,   44,
                    -73, -41,  72,  36,  23,  62,   7,  -17,
                    -167, -89, -34, -49,  61, -97, -15, -107,
                },
                { // Bishop
                    -33,  -3, -14, -21, -13, -12, -39, -21,
                    4,  15,  16,   0,   7,  21,  33,   1,
                    0,  15,  15,  15,  14,  27,  18,  10,
                    -6,  13,  13,  26,  34,  12,  10,   4,
                    -4,   5,  19,  50,  37,  37,   7,  -2,
                    -16,  37,  43,  40,  35,  50,  37,  -2,
                    -26,  16, -18, -13,  30,  59,  18, -47,
                    -29,   4, -82, -37, -25, -42,   7,  -8,
                },
                { // Rook
                    -19, -13,   1,  17, 16,  7, -37, -26,
                    -44, -16, -20,  -9, -1, 11,  -6, -71,
                    -45, -25, -16, -17,  3,  0,  -5, -33,
                    -36, -26, -12,  -1,  9, -7,   6, -23,
                    -24, -11,   7,  26, 24, 35,  -8, -20,
                    -5,  19,  26,  36, 17, 45,  61,  16,
                    27,  32,  58,  62, 80, 67,  26,  44,
                    32,  42,  32,  51, 63,  9,  31,  43,
                },
                { // Queen
                    -1, -18,  -9,  10, -15, -25, -31, -50,
                    -35,  -8,  11,   2,   8,  15,  -3,   1,
                    -14,   2, -11,  -2,  -5,   2,  14,   5,
                    -9, -26,  -9, -10,  -2,  -4,   3,  -3,
                    -27, -27, -16, -16,  -1,  17,  -2,   1,
                    -13, -17,   7,   8,  29,  56,  47,  57,
                    -24, -39,  -5,   1, -16,  57,  28,  54,
                    -28,   0,  29,  12,  59,  44,  43,  45,
                },
                { // King
                    -15,  36,  12, -54,   8, -28,  24,  14,
                    1,   7,  -8, -64, -43, -16,   9,   8,
                    -14, -14, -22, -46, -44, -30, -15, -27,
                    -49,  -1, -27, -39, -46, -44, -33, -51,
                    -17, -20, -12, -27, -30, -25, -14, -36,
                    -9,  24,   2, -16, -20,   6,  22, -22,
                    29,  -1, -20,  -7,  -8,  -4, -38, -29,
                    -65,  23,  16, -15, -56, -34,   2,  13,
                }
            },
            { // Endgame
                { // Pawn
                    0,   0,   0,   0,   0,   0,   0,   0,
                    13,   8,   8,  10,  13,   0,   2,  -7,
                    4,   7,  -6,   1,   0,  -5,  -1,  -8,
                    13,   9,  -3,  -7,  -7,  -8,   3,  -1,
                    32,  24,  13,   5,  -2,   4,  17,  17,
                    94, 100,  85,  67,  56,  53,  82,  84,
                    178, 173, 158, 134, 147, 132, 165, 187,
                    0,   0,   0,   0,   0,   0,   0,   0,
                },
                { // Knight
                    -29, -51, -23, -15, -22, -18, -50, -64,
                    -42, -20, -10,  -5,  -2, -20, -23, -44,
                    -23,  -3,  -1,  15,  10,  -3, -20, -22,
                    -18,  -6,  16,  25,  16,  17,   4, -18,
                    -17,   3,  22,  22,  22,  11,   8, -18,
                    -24, -20,  10,   9,  -1,  -9, -19, -41,
                    -25,  -8, -25,  -2,  -9, -25, -24, -52,
                    -58, -38, -13, -28, -31, -27, -63, -99,
                },
                { // Bishop
                    -23,  -9, -23,  -5, -9, -16,  -5, -17,
                    -14, -18,  -7,  -1,  4,  -9, -15, -27,
                    -12,  -3,   8,  10, 13,   3,  -7, -15,
                    -6,   3,  13,  19,  7,  10,  -3,  -9,
                    -3,   9,  12,   9, 14,  10,   3,   2,
                    2,  -8,   0,  -1, -2,   6,   0,   4,
                    -8,  -4,   7, -12, -3, -13,  -4, -14,
                    -14, -21, -11,  -8, -7,  -9, -17, -24,
                },
                { // Rook
                    -9,  2,  3, -1, -5, -13,   4, -20,
                    -6, -6,  0,  2, -9,  -9, -11,  -3,
                    -4,  0, -5, -1, -7, -12,  -8, -16,
                    3,  5,  8,  4, -5,  -6,  -8, -11,
                    4,  3, 13,  1,  2,   1,  -1,   2,
                    7,  7,  7,  5,  4,  -3,  -5,  -3,
                    11, 13, 13, 11, -3,   3,   8,   3,
                    13, 10, 18, 15, 12,  12,   8,   5,
                },
                { // Queen
                    -33, -28, -22, -43,  -5, -32, -20, -41,
                    -22, -23, -30, -16, -16, -23, -36, -32,
                    -16, -27,  15,   6,   9,  17,  10,   5,
                    -18,  28,  19,  47,  31,  34,  39,  23,
                    3,  22,  24,  45,  57,  40,  57,  36,
                    -20,   6,   9,  49,  47,  35,  19,   9,
                    -17,  20,  32,  41,  58,  25,  30,   0,
                    -9,  22,  22,  27,  27,  19,  10,  20,
                },
                { // King
                    -53, -34, -21, -11, -28, -14, -24, -43
                    -27, -11,   4,  13,  14,   4,  -5, -17,
                    -19,  -3,  11,  21,  23,  16,   7,  -9,
                    -18,  -4,  21,  24,  27,  23,   9, -11,
                    -8,  22,  24,  27,  26,  33,  26,   3,
                    10,  17,  23,  15,  20,  45,  44,  13,
                    -12,  17,  14,  17,  17,  38,  23,  11,
                    -74, -35, -18, -18, -11,  15,   4, -17,
                }
            }
        },

        // Bonuses / Penalties for pieces based on number of legal moves.
        eval_mobility {
            { // Midgame
                // Pawn
                {0},
                // Knight
                {-16, -12, -8, -4, 0, 4, 8, 12, 16},
                // Bishop
                {-21, -18, -15, -12, -9, -6, -3, 0, 3, 6, 9, 12, 15, 18},
                // Rook
                {-14, -12, -10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10, 12, 14, 16},
                // Queen
                {-14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}
            },
            { // Endgame
                // Pawn
                {0},
                // Knight
                {-16, -12, -8, -4, 0, 4, 8, 12, 16},
                // Bishop
                {-21, -18, -15, -12, -9, -6, -3, 0, 3, 6, 9, 12, 15, 18},
                // Rook
                {-28, -24, -20, -16, -12, -8, -4, 0, 4, 8, 12, 16, 20, 24, 28, 32},
                // Queen
                {-28, -26, -24, -22, -20, -18, -16, -14, -12, -10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26}
            }
        },

        // Tables for bonuses for passed pawns on a particular square.
        eval_passed_pawns {
            { // Midgame
                0,   0,   0,   0,   0,   0,   0,   0,
                20,  20,  20,  20,  20,  20,  20,  20,
                20,  20,  20,  20,  20,  20,  20,  20,
                30,  30,  30,  30,  30,  30,  30,  30,
                50,  50,  50,  50,  50,  50,  50,  50,
                75,  75,  75,  75,  75,  75,  75,  75,  
                100, 100, 100, 100, 100, 100, 100, 100,
                0,   0,   0,   0,   0,   0,   0,   0
            },
            { // Endgame
                0,   0,   0,   0,   0,   0,   0,   0,
                20,  20,  20,  20,  20,  20,  20,  20,
                20,  20,  20,  20,  20,  20,  20,  20,
                30,  30,  30,  30,  30,  30,  30,  30,
                50,  50,  50,  50,  50,  50,  50,  50,
                75,  75,  75,  75,  75,  75,  75,  75,  
                100, 100, 100, 100, 100, 100, 100, 100,
                0,   0,   0,   0,   0,   0,   0,   0
            },
        },

        // Tables for penalties for isolated pawns on a particular square.
        eval_isolated_pawns {
            { // Midgame
                0,   0,   0,   0,   0,   0,   0,   0,
                -8, -12, -14, -16, -16, -14, -12, -10,
                -8, -12, -14, -16, -16, -14, -12, -10,
                -10, -12, -14, -16, -16, -14, -12, -10,
                -10, -12, -14, -16, -16, -14, -12, -10,
                -10, -12, -14, -16, -16, -14, -12, -10,
                -10, -12, -14, -16, -16, -14, -12, -10,
                0,   0,   0,   0,   0,   0,   0,   0
            },
            { // Endgame
                0,   0,   0,   0,   0,   0,   0,   0,
                -8, -12, -14, -16, -16, -14, -12, -10,
                -8, -12, -14, -16, -16, -14, -12, -10,
                -10, -12, -14, -16, -16, -14, -12, -10,
                -10, -12, -14, -16, -16, -14, -12, -10,
                -10, -12, -14, -16, -16, -14, -12, -10,
                -10, -12, -14, -16, -16, -14, -12, -10,
                0,   0,   0,   0,   0,   0,   0,   0
            }
        },

        // Bonuses for protected pawns on a particular rank.
        eval_protected_pawns {
             // Midgame
            {
                0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,
                3,  3,  3,  3,  3,  3,  3,  3,
                5,  5,  5,  5,  5,  5,  5,  5,
                5,  5,  5,  5,  5,  5,  5,  5, 
                8,  8,  8,  8,  8,  8,  8,  8,
                10, 10, 10, 10, 10, 10, 10, 10,
                0,  0,  0,  0,  0,  0,  0,  0
            },
            // Endgame
            {
                0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,
                2,  2,  2,  2,  2,  2,  2,  2,
                4,  4,  4,  4,  4,  4,  4,  4,
                6,  6,  6,  6,  6,  6,  6,  6,
                10, 10, 10, 10, 10, 10, 10, 10,
                12, 12, 12, 12, 12, 12, 12, 12,
                0,  0,  0,  0,  0,  0,  0,  0
            }
        },
        
        // Bonuses for connected pawns on a particular rank.
        eval_connected_pawns {
            // Midgame
            {
                0,  0,  0,  0,  0,  0,  0,  0,
                1,  1,  1,  1,  1,  1,  1,  1,
                2,  2,  2,  2,  2,  2,  2,  2,
                2,  2,  2,  2,  2,  2,  2,  2,
                3,  3,  3,  3,  3,  3,  3,  3,
                5,  5,  5,  5,  5,  5,  5,  5, 
                7,  7,  7,  7,  7,  7,  7,  7,
                0,  0,  0,  0,  0,  0,  0,  0
            },
            // Endgame
            {
                0,  0,  0,  0,  0,  0,  0,  0,
                1,  1,  1,  1,  1,  1,  1,  1,
                2,  2,  2,  2,  2,  2,  2,  2,
                2,  2,  2,  2,  2,  2,  2,  2,
                4,  4,  4,  4,  4,  4,  4,  4,
                7,  7,  7,  7,  7,  7,  7,  7,
                9,  9,  9,  9,  9,  9,  9,  9,
                0,  0,  0,  0,  0,  0,  0,  0
            },
        },

        // Penalty for doubled pawns.
        eval_doubled_pawns {-20, -30},
        
        // Penalty for tripled pawns.
        eval_tripled_pawns {-50, -70},
        
        // Bonuses for pawns in front of the king, 1 or 2 ranks forwards.
        eval_king_pawn_shield {
            // Midgame
            {8, 4},
            // Endgame
            {0, 0}
        },

        // Bonus for the side to move.
        eval_tempo {10, 10},

        // Bonus for having a bishop pair.
        eval_bishop_pair {23, 33},

        // ----------------------------------------------
        // ----------- Transposition Table --------------
        // ----------------------------------------------
        tt_size_mb {256} {};

    std::vector<ParameterInfo> GetParameterInfoList(const EngineParameters& params) {
        std::vector<ParameterInfo> parameters;

        // ------------ Evaluation ------------

        // Eval piece values
        for (int game_phase : {0, 1}) {
            for (int piece = 0; piece < 5; ++piece) {
                parameters.push_back({
                    "eval_piece_values_" + std::to_string(game_phase) + "_" + std::to_string(piece),
                    const_cast<int*>(&params.eval_piece_values[game_phase][piece])
                });
            }
        }

        // Eval psqt
        for (int game_phase : {0, 1}) {
            for (int piece = 0; piece < 6; ++piece) {
                for (int square = 0; square < 64; ++square) {
                    parameters.push_back({
                        "eval_psqt_" + std::to_string(game_phase) + "_" + std::to_string(piece) + "_" + std::to_string(square),
                        const_cast<int*>(&params.eval_psqt[game_phase][piece][square])
                    });
                }
            }
        }

        // Eval mobility
        for (int game_phase : {0, 1}) {
            for (int piece = 0; piece < 5; ++piece) {
                for (int mobility = 0; mobility < 28; ++mobility) {
                    // Don't care about pawns
                    if (piece == 0)
                        break;

                    // Knight max mobility = 8
                    if (piece == 1 && mobility > 8)
                        break;

                    // Bishop max mobility = 13
                    if (piece == 2 && mobility > 13)
                        break;

                    // Rook max mobility = 15
                    if (piece == 3 && mobility > 15)
                        break;

                    // Queen max mobility = 27
                    if (piece == 4 && mobility > 27)
                        break;

                    parameters.push_back({
                        "eval_mobility_" + std::to_string(game_phase) + "_" + std::to_string(piece) + "_" + std::to_string(mobility),
                        const_cast<int*>(&params.eval_mobility[game_phase][piece][mobility])
                    });
                }
            }
        }

        // Eval passed pawns
        for (int game_phase : {0, 1}) {
            for (int square = 0; square < 64; ++square) {
                parameters.push_back({
                    "eval_passed_pawns_" + std::to_string(game_phase) + "_" + std::to_string(square),
                    const_cast<int*>(&params.eval_passed_pawns[game_phase][square])
                });
            }
        }

        // Eval isolated pawns
        for (int game_phase : {0, 1}) {
            for (int square = 0; square < 64; ++square) {
                parameters.push_back({
                    "eval_isolated_pawns_" + std::to_string(game_phase) + "_" + std::to_string(square),
                    const_cast<int*>(&params.eval_isolated_pawns[game_phase][square])
                });
            }
        }

        // Eval protected pawns
        for (int game_phase : {0, 1}) {
            for (int square = 0; square < 64; ++square) {
                parameters.push_back({
                    "eval_protected_pawns_" + std::to_string(game_phase) + "_" + std::to_string(square),
                    const_cast<int*>(&params.eval_protected_pawns[game_phase][square])
                });
            }
        }

        // Eval connected pawns
        for (int game_phase : {0, 1}) {
            for (int square = 0; square < 64; ++square) {
                parameters.push_back({
                    "eval_connected_pawns_" + std::to_string(game_phase) + "_" + std::to_string(square),
                    const_cast<int*>(&params.eval_connected_pawns[game_phase][square])
                });
            }
        }

        // Eval doubled pawns
        for (int game_phase : {0, 1}) {
            parameters.push_back({
                "eval_doubled_pawn_" + std::to_string(game_phase),
                const_cast<int*>(&params.eval_doubled_pawns[game_phase])
            });
        }

        // Eval tripled pawns
        for (int game_phase : {0, 1}) {
            parameters.push_back({
                "eval_tripled_pawn_" + std::to_string(game_phase),
                const_cast<int*>(&params.eval_tripled_pawns[game_phase])
            });
        }

        // Eval king pawn shield
        for (int game_phase : {0, 1}) {
            for (int dist : {0, 1}) {
                parameters.push_back({
                    "eval_king_pawn_shield_" + std::to_string(game_phase) + "_" + std::to_string(dist),
                    const_cast<int*>(&params.eval_king_pawn_shield[game_phase][dist])
                });
            }
        }

        // Eval tempo
        for (int game_phase : {0, 1}) {
            parameters.push_back({
                "eval_tempo_" + std::to_string(game_phase),
                const_cast<int*>(&params.eval_tempo[game_phase])
            });
        }

        // Eval bishop pair
        for (int game_phase : {0, 1}) {
            parameters.push_back({
                "eval_bishop_pair_" + std::to_string(game_phase),
                const_cast<int*>(&params.eval_bishop_pair[game_phase])
            });
        }

        // ---------------- Transposition Table ----------------
        
        // Transposition table size.
        parameters.push_back({
            "tt_size_mb",
            const_cast<int*>(&params.tt_size_mb)
        });

        return parameters;
    }

    void SaveParameters(const EngineParameters& params, const std::string& path) {
        std::ofstream file(path);

        if (!file) {
            throw std::runtime_error("Could not open parameter file: " + path);
        }

        file << "name,value\n";

        for (const auto& parameter : GetParameterInfoList(params)) {
            file << parameter.name << ',' << *parameter.value << '\n';
        }
    }

    // TODO.
    void LoadParameters(EngineParameters& params, const std::string& path) {

    }
        
} // namespace parameters