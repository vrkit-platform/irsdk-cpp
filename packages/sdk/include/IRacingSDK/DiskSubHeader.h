#pragma once

#include <magic_enum/magic_enum.hpp>
#include <tchar.h>
#include <windows.h>

#include "ErrorTypes.h"
#include "Types.h"

namespace IRacingSDK {

// sub header used when writing telemetry to disk
struct DiskSubHeader {
    time_t startDate{};
    double startTime{};
    double endTime{};
    uint32_t lapCount{};
    uint32_t sampleCount{};
};

constexpr auto DiskSubHeaderSize = sizeof(DiskSubHeader);

} // namespace IRacingSDK
