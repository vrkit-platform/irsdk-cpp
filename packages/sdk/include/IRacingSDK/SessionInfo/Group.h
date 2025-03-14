#pragma once

#include "Camera.h"
#include <string>
#include <vector>

namespace IRacingSDK::SessionInfo
{
    struct Group
    {
        std::int32_t groupNum;
        std::string groupName;
        std::vector<Camera> cameras;
        bool isScenic;


    };
}