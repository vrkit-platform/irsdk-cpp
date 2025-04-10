#pragma once

#include "BrakesDriveUnit.h"
#include "Chassis.h"
#include "TiresAero.h"
#include <string>

namespace IRacingSDK::SessionInfo
{
    struct CarSetup
    {
        std::int32_t updateCount;
        TiresAero tiresAero;
        Chassis chassis;
        BrakesDriveUnit brakesDriveUnit;


    };
}