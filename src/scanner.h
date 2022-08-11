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
    [[nodiscard]] char advance() {
        m_position.column++;
        return m_source[m_current++];
    }

    void addToken(const TokenType& type, const std::string_view literal = "") {
        fmt::print("{}: m_start = {}, m_current = {}\n", type, m_start, m_current);

        const std::string_view lexeme = literal != "" 
            ? literal
            : std::string_view { m_source.begin() + m_start, m_source.begin() + m_current };

        if (type == TokenType::Eof) {
            m_tokens.push_back(Token { .type { type }, .lexeme { "" }, .position { m_position } });
        } else {
            m_tokens.push_back(Token { 
                .type { type },
                .lexeme { lexeme },
                .position { m_position } });
        }
    }

    void newLine() {
        m_position.line++;
        m_position.column = 1;
    }

    [[nodiscard]] bool isAtEnd() {
        return m_current >= m_source.size();
    }

    [[nodiscard]] char peek() {
        if (isAtEnd()) {
            fmt::print(stderr, "peek failed");
            return '\0';
        }
        return m_source[m_current];
    }

    [[nodiscard]] char peekNext() {
        if (m_current + 1 >= m_source.size()) {
            return '\0';
        }
        return m_source[m_current + 1];
    }

    [[nodiscard]] bool expect(char expected) {
        if (isAtEnd()) {
            return false;
        }

        if (m_source[m_current] != expected) {
            return false;
        }

        m_current++;
        return true;
    }

    void string();
    void number();
    void identifier();

    std::string_view getCurrentLiteral() {
        return { m_source.begin() + m_start, m_source.begin() + m_current };
    }
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
