#pragma once

#include "Radio.h"
#include <string>
#include <vector>

namespace IRacingSDK::SessionInfo
{
    struct RadioInfo
    {
        std::int32_t selectedRadioNum;
        std::vector<Radio> radios;


    };
}