#ifndef LIGHTKNIGHT_PARAMS_H
#define LIGHTKNIGHT_PARAMS_H

#include <vector>
#include <string>
#include <types.h>

namespace lightknight::parameters {
    
    struct EvalParameters {
        int piece_values[2][5];
        int psqt[2][6][64];
        int mobility[2][5][28];
        int passed_pawns[2][64];
        int isolated_pawns[2][64];
        int protected_pawns[2][64];
        int connected_pawns[2][64];
        int doubled_pawns[2];
        int tripled_pawns[2];
        int king_pawn_shield[2][2];
        int tempo[2];
        int bishop_pair[2];

        // Constructor with default values.
        EvalParameters();
    };

    struct EngineParameters {
        EvalParameters eval = EvalParameters();

        int tt_size_mb;
        
        // Constructor with default values.
        EngineParameters();
    };

    struct ParameterInfo {
        std::string name;
        int* value;
    };

    struct ParsedParameterName {
        std::string name;
        std::vector<int> indices;
    };

    std::vector<ParameterInfo> GetEvalParameterInfoList(const EngineParameters& params);
    std::vector<ParameterInfo> GetParameterInfoList(const EngineParameters& params);
    ParsedParameterName ParseParameterName(const std::string& param_name);
    
    void SaveParameters(const EngineParameters& params, const std::string& path);
    void LoadParameters(EngineParameters& params, const std::string& path);
} // namespace params

#endif // LIGHTKNIGHT_PARAMS_H