//
// Created by jglanz on 8/27/2024.
//

#pragma once

#include <string>
#include <magic_enum/magic_enum.hpp>

namespace IRacingSDK::Utils {
    template<typename E>
    auto EnumName(E value) {
        return std::string{magic_enum::enum_name<E>(value)};
    }
}