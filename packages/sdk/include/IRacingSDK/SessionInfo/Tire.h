#pragma once

#include <string>

namespace IRacingSDK::SessionInfo
{
    struct Tire
    {
        float startingPressure;
        float lastHotPressure;
        float lastTempsOMI;
        float treadRemaining;
        float lastTempsIMO;


    };
}