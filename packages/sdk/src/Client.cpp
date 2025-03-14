#include <cstdio>

#include <IRacingSDK/Client.h>
#include <spdlog/spdlog.h>

namespace IRacingSDK {
    using namespace Utils;

    Opt<const VarDataHeader*> Client::getVarHeader(KnownVarName name) {
        return getVarHeader(KnownVarNameToStringView(name));
    }

    std::optional<uint32_t> Client::getVarIdx(KnownVarName name) {
        return getVarIdx(KnownVarNameToStringView(name));
    }

    std::optional<VarDataType> Client::getVarType(KnownVarName name) {
        return getVarType(KnownVarNameToStringView(name));
    }

    std::optional<uint32_t> Client::getVarCount(KnownVarName name) {
        return getVarCount(KnownVarNameToStringView(name));
    }

    std::optional<bool> Client::getVarBool(KnownVarName name, uint32_t entry) {
        return getVarBool(KnownVarNameToStringView(name), entry);
    }

    std::optional<int> Client::getVarInt(KnownVarName name, uint32_t entry) {
        return getVarInt(KnownVarNameToStringView(name), entry);
    }

    std::optional<float> Client::getVarFloat(KnownVarName name, uint32_t entry) {
        return getVarFloat(KnownVarNameToStringView(name), entry);
    }

    std::optional<double> Client::getVarDouble(KnownVarName name, uint32_t entry) {
        return getVarDouble(KnownVarNameToStringView(name), entry);
    }
} // namespace IRacingSDK
