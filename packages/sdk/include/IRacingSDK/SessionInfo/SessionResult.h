#pragma once

#include <string>

namespace IRacingSDK::SessionInfo
{
    struct SessionResult {
        std::int32_t position;
        std::int32_t classPosition;
        std::int32_t carIdx;
        std::int32_t fastestLap;
        float fastestTime;
    };
}