//
// Created by jglanz on 1/26/2024.
//

#pragma once
#include <fmt/core.h>
#include <stdexcept>
#include <expected>

namespace IRacingSDK {
    enum class ErrorCode : uint32_t {
        General,
        NotImplemented,
        NotFound
    };

    class GeneralError : public std::logic_error {
    public:
        template <typename E, typename... Args>
        static E create(ErrorCode code, fmt::format_string<Args...> fmt, Args&&... args) {
            return E(code, fmt::format(fmt, std::forward<Args>(args)...));
        }

        template <typename E>
        static E create(ErrorCode code, const std::string& msg) {
            return E(code, msg);
        }

        explicit GeneralError(ErrorCode code = ErrorCode::General, const std::string& msg = "") : std::logic_error(msg),
            code_(code) {};

        explicit GeneralError(const std::string& msg = "") : std::logic_error(msg), code_(ErrorCode::General) {};

        virtual ErrorCode code() {
            return code_;
        };
        
    protected:
        ErrorCode code_{ErrorCode::General};
    };

    class NotImplementedError : public GeneralError {
    public:
        explicit NotImplementedError(
            ErrorCode code = ErrorCode::NotImplemented,
            const std::string& msg = ""
        ) : GeneralError(code, msg) {};

        explicit NotImplementedError(const std::string& msg = "") : GeneralError(ErrorCode::NotImplemented, msg) {};
    };


    class NotFoundError : public GeneralError {
    public:
        explicit NotFoundError(ErrorCode code = ErrorCode::NotFound, const std::string& msg = "") : GeneralError(
            code,
            msg
        ) {};

        explicit NotFoundError(const std::string& msg = "") : GeneralError(ErrorCode::NotFound, msg) {};
    };

    template <typename V>
    using Expected = std::expected<V, GeneralError>;

    template <typename E, typename... T>
    auto MakeUnexpected(fmt::format_string<T...> fmt, T&&... args) {
        return std::unexpected<E>(E(fmt::format(fmt, args...)));
    }

    template <typename E, typename... T>
    auto MakeUnexpected(ErrorCode code, fmt::format_string<T...> fmt, T&&... args) {
        return std::unexpected<E>(E(code, fmt::format(fmt, args...)));
    };
} // namespace IRacingSDK
