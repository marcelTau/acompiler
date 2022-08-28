#pragma once

#include <vector>
#include <unordered_map>
#include "token.h"

struct ScannerError : public std::exception {
    ScannerError(const std::string& message, TokenPosition pos) {
        msg = fmt::format("ERROR: {} at [{}, {}]", message, pos.line, pos.column).c_str();
    }
    [[nodiscard]] const char *what() const noexcept override {
        return msg.c_str();
    }
private:
    std::string msg;
};

class Scanner {
public:
    using TokenList = std::vector<Token>;
public:
    Scanner() = default;
    [[nodiscard]] TokenList scan(const std::string_view source);

private:
    void nextToken();
    [[nodiscard]] char advance();
    void addToken(const TokenType& type, const std::string_view literal = "");
    void newLine();

    [[nodiscard]] bool isAtEnd();
    [[nodiscard]] char peek();
    [[nodiscard]] char peekNext();
    [[nodiscard]] bool expect(char expected);

    void string();
    void number();
    void identifier();
    std::string_view getCurrentLiteral();
private:
    TokenList m_tokens {};
    std::string_view m_source;
    std::size_t m_current { 0 };
    std::size_t m_start { 0 };

    TokenPosition m_position { 1, 0 };
    const std::unordered_map<std::string_view, TokenType> m_keywords = {
        { "and", TokenType::And },
        { "else", TokenType::Else },
        { "false", TokenType::False },
        { "for", TokenType::For },
        { "fun", TokenType::Fun },
        { "if", TokenType::If },
        { "nuffin", TokenType::Nuffin },
        { "or", TokenType::Or },
        { "print", TokenType::Print },
        { "return", TokenType::Return },
        { "true", TokenType::True },
        { "let", TokenType::Let },
        { "while", TokenType::While },
        { "then", TokenType::Then },
        { "do", TokenType::Do },
        { "end", TokenType::End },
    };
};
