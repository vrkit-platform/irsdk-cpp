#pragma once

#include "ChassisCorner.h"
#include "Front.h"
#include "Rear.h"
#include <string>
namespace IRacingSDK::SessionInfo
{
    struct Chassis
    {
        Front front;
        ChassisCorner leftFront;
        ChassisCorner leftRear;
        ChassisCorner rightFront;
        ChassisCorner rightRear;
        Rear rear;


    };
}