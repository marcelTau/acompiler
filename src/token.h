#pragma once

#include <string_view>
#include <string>
#include <fmt/core.h>

static constexpr const char *const TokenTypeNames[] = {
    "LeftParen",
    "RightParen",
    "Comma",
    "Dot",
    "Minus",
    "Plus",
    "Semicolon",
    "Slash",
    "Star",
    "Bang",
    "BangEqual",
    "Equal",
    "EqualEqual",
    "Greater",
    "GreaterEqual",
    "Less",
    "LessEqual",
    "Identifier",
    "String",
    "Number",
    "And",
    "Else",
    "False",
    "For",
    "Fun",
    "If",
    "Nil",
    "Or",
    "Print",
    "Return",
    "True",
    "Var",
    "While",
    "Error",
    "Eof",
};

enum class TokenType : std::uint8_t {
    // Single-character tokens.
    LeftParen,
    RightParen,
    Comma,
    Dot,
    Minus,
    Plus,
    Semicolon,
    Slash,
    Star,

    // One or two character tokens.
    Bang,
    BangEqual,
    Equal,
    EqualEqual,
    Greater,
    GreaterEqual,
    Less,
    LessEqual,
    // Literals.
    Identifier,
    String,
    Number,
    // Keywords.
    And,            // 'and'
    Else,
    False,
    For,
    Fun,
    If,
    Nil,
    Or,             // 'or'
    Print,
    Return,
    True,
    Var,
    While,

    Error,
    Eof
};

template<>
struct fmt::formatter<TokenType> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx);

    template<typename FormatContext>
    auto format(const TokenType& type, FormatContext& ctx);
};

template<typename ParseContext>
constexpr auto fmt::formatter<TokenType>::parse(ParseContext& ctx) {
    return ctx.begin();
}

template<typename FormatContext>
auto fmt::formatter<TokenType>::format(const TokenType& type, FormatContext& ctx) {
    return fmt::format_to(ctx.out(), "{}", TokenTypeNames[static_cast<std::size_t>(type)]);
}

struct TokenPosition {
    int line;
    int column;

    bool operator==(const TokenPosition& other) const {
        return line == other.line && column == other.column;
    }
};

struct Token {
    TokenType type;
    std::string_view lexeme;
    TokenPosition position;

    bool operator==(const Token& other) const {
        return type == other.type && lexeme == other.lexeme && position == other.position;
    }
};

template<>
struct fmt::formatter<Token> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx);

    template<typename FormatContext>
    auto format(const Token& token, FormatContext& ctx);
};

template<typename ParseContext>
constexpr auto fmt::formatter<Token>::parse(ParseContext& ctx) {
    return ctx.begin();
}

template<typename FormatContext>
auto fmt::formatter<Token>::format(const Token& token, FormatContext& ctx) {
    return fmt::format_to(ctx.out(), "Token ({}, '{}', [{},{}])", token.type, token.lexeme, token.position.line, token.position.column);
}
