#pragma once

#include <cstdint>
#include <cassert>
#include <string_view>
#include <source_location>
#include <variant>
#include "token.h"

enum class ErrorType : std::uint8_t {
    ParserError,
    ResolverError,
    None,
};

struct Error {
    ErrorType type { ErrorType::None };
    Token token;
    std::string msg;
};

/**
 * @brief `Result` is a wrapper around a std::variant that holds a good value or an error value.
 *        It provides a `rust-like` interface for errorhandling.
 */
template <typename T>
struct Result {
    Result(struct Error error) : value(error) {}
    Result(T &&value) : value(std::move(value)) {}

    static auto Error(ErrorType type, const Token& token, std::string_view msg) -> Result {
        std::string smsg(msg);
        struct Error e { .type = type, .token = token, .msg = smsg };
        return Result(e);
    }

    static auto Error(const struct Error& error) -> Result {
        return Result(error);
    }

    static auto ParseError(const Token& token, std::string_view msg) -> Result {
        std::string smsg(msg);
        struct Error e { .type = ErrorType::ParserError, .token = token, .msg = smsg };
        return Result(e);
    }

    constexpr operator bool() const {
        return !std::holds_alternative<struct Error>(value);
    }

    [[nodiscard]] struct Error get_err() const {
        return std::get<struct Error>(value);
    }

    T unwrap(/*const std::source_location& loc = std::source_location::current()*/) {
        if (std::holds_alternative<struct Error>(value)) {
            assert(false && "1");
        }
        return std::move(std::get<T>(value));
    }
    T unwrap(/*const std::source_location& loc = std::source_location::current()*/) const {
        if (std::holds_alternative<struct Error>(value)) {
            assert(false && "1");
        }
        return std::move(std::get<T>(value));
    }

private:
    std::variant<T, struct Error> value;
};
