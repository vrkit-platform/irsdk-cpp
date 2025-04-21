#include <cstdio>

#include <IRacingSDK/Client.h>
#include <spdlog/spdlog.h>

namespace IRacingSDK {
    using namespace Utils;

    Opt<std::shared_ptr<const VarDataHeader>> Client::getVarHeader(KnownVarName name) {
        return getVarHeader(std::string{KnownVarNameToStringView(name)});
    }

    std::optional<uint32_t> Client::getVarIdx(KnownVarName name) {
        return getVarIdx(std::string{KnownVarNameToStringView(name)});
    }

    std::optional<VarDataType> Client::getVarType(KnownVarName name) {
        return getVarType(std::string{KnownVarNameToStringView(name)});
    }

    std::optional<uint32_t> Client::getVarCount(KnownVarName name) {
        return getVarCount(std::string{KnownVarNameToStringView(name)});
    }

    std::optional<bool> Client::getVarBool(KnownVarName name, uint32_t entry) {
        return getVarBool(std::string{KnownVarNameToStringView(name)}, entry);
    }

    std::optional<int> Client::getVarInt(KnownVarName name, uint32_t entry) {
        return getVarInt(std::string{KnownVarNameToStringView(name)}, entry);
    }

    std::optional<float> Client::getVarFloat(KnownVarName name, uint32_t entry) {
        return getVarFloat(std::string{KnownVarNameToStringView(name)}, entry);
    }

    std::optional<double> Client::getVarDouble(KnownVarName name, uint32_t entry) {
        return getVarDouble(std::string{KnownVarNameToStringView(name)}, entry);
    }
} // namespace IRacingSDK
