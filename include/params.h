#ifndef LIGHTKNIGHT_PARAMS_H
#define LIGHTKNIGHT_PARAMS_H

#include <vector>
#include <string>
#include <types.h>

namespace lightknight::parameters {
    struct EngineParameters {
        // Evaluation parameters
        int eval_piece_values[2][5];
        int eval_psqt[2][6][64];
        int eval_mobility[2][5][28];
        int eval_passed_pawns[2][64];
        int eval_isolated_pawns[2][64];
        int eval_protected_pawns[2][64];
        int eval_connected_pawns[2][64];
        int eval_doubled_pawns[2];
        int eval_tripled_pawns[2];
        int eval_king_pawn_shield[2][2];
        int eval_tempo[2];
        int eval_bishop_pair[2];

        // Transposition table
        int tt_size_mb;
        
        // Constructor with default values.
        EngineParameters();

        // Save parameters to a file
        void Save();
        void Load();
    };

    struct ParameterInfo {
        std::string name;
        int* value;
    };

    std::vector<ParameterInfo> GetParameterInfoList(const EngineParameters& params);

    void SaveParameters(const EngineParameters& params, const std::string& path);
    void LoadParameters(EngineParameters& params, const std::string& path);
} // namespace params

#endif // LIGHTKNIGHT_PARAMS_H