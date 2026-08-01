#ifndef LIGHTKNIGHT_TIME_MANAGEMENT_H
#define LIGHTKNIGHT_TIME_MANAGEMENT_H

#include <optional>
#include <limits>
#include <algorithm>

namespace lightknight::time_management {

    constexpr int kMinSearchTimeMs = 10;
    constexpr int kMaxSearchTimeMs = std::numeric_limits<int>::max() / 2;

    int ComputeTimeLimitMs(
        std::optional<int> remaining_time_ms,
        std::optional<int> increment_ms,
        std::optional<int> moves_to_go
    );

} // namespace lightknight::time_management

#endif // LIGHTKNIGHT_TIME_MANAGEMENT_H