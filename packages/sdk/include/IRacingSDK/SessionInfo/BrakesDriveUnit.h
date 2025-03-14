#pragma once

#include "BrakeSpec.h"
#include "Engine.h"
#include "Fuel.h"
#include "GearRatio.h"
namespace IRacingSDK::SessionInfo
{
    struct BrakesDriveUnit
    {
        BrakeSpec brakeSpec;
        Fuel fuel;
        Engine engine;
        GearRatio gearRatios;


    };
}