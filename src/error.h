#pragma once

#include <cstdint>
#include <cassert>
#include <string_view>
#include <source_location>
#include "token.h"

enum class ErrorType : std::uint8_t {
    ParserError,
    None,
};

struct Error {
    ErrorType type { ErrorType::None };
    Token token;
    std::string_view msg;
};

template <typename T>
struct Result {
    Result(T &&value) : ok_value(std::move(value)) {}
    //Result(const T& value) : ok_value(value) {}
    Result(const Error& error) {
        error_value = error;
    }

    static Result Error(ErrorType type, const Token& token, std::string_view msg) {
        struct Error e { .type = type, .token = token, .msg = msg };
        return Result(e);
    }

    static Result Error(const struct Error& error) {
        return Result(error);
    }

    constexpr operator bool() const {
        // Default error type, NO ERROR
        return this->error_value.type == ErrorType::None;
    }

    [[nodiscard]] constexpr struct Error get_err() const {
        return error_value;
    }

    T unwrap(/*const std::source_location& loc = std::source_location::current()*/) {
        if (this->error_value.type != ErrorType::None) {
            assert(false && "1");
            //assert(false && fmt::format("Panic at unwrap at {} [{}, {}] ({})\n", loc.file_name(), loc.line(), loc.column(), loc.function_name()).c_str());
        }
        return std::move(ok_value);
    }
    T unwrap(/*const std::source_location& loc = std::source_location::current()*/) const {
        if (this->error_value.type != ErrorType::None) {
            assert(false && "1");
            //assert(false && fmt::format("Panic at unwrap at {} [{}, {}] ({})\n", loc.file_name(), loc.line(), loc.column(), loc.function_name()).c_str());
        }
        return std::move(ok_value);
    }

    //constexpr T unwrap() const {
        //if (not *this) {
            //assert(false && "Panic at unwrap()");
        //}
        //return std::move(ok_value);
    //}

private:
    T ok_value;
    struct Error error_value;
};
