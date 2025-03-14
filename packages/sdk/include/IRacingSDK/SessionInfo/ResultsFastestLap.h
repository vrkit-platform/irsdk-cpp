#pragma once

#include <string>

namespace IRacingSDK::SessionInfo
{
    struct ResultsFastestLap
    {
        std::int32_t carIdx;
        std::int32_t fastestLap;
        float fastestTime;


    };
}