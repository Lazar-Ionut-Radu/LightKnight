// time_management.cc
#include "time_management.h"

#include <algorithm>
#include <limits>

namespace lightknight::time_management {
    int ComputeTimeLimitMs(
        std::optional<int> remaining_time_ms,
        std::optional<int> increment_ms,
        std::optional<int> moves_to_go
    ) {
        if (!remaining_time_ms.has_value())
            return kMaxSearchTimeMs;

        const int remaining_time = *remaining_time_ms;
        const int inc = increment_ms.has_value() ? *increment_ms : 0;
        
        if (moves_to_go.has_value()) {
            return remaining_time / std::max(*moves_to_go, 1) + inc / 2;
        }
        
        // https://www.chessprogramming.org/Time_Management
        return remaining_time / 20 + inc / 2;
    }
} // namespace lightknight::time_management

