#pragma once

#include <vector>
#include "token.h"

class Scanner {
public:
    using TokenList = std::vector<Token>;
public:
    Scanner() = default;
    [[nodiscard]] TokenList scan(const std::string_view source);

private:
    [[nodiscard]] bool nextToken();
    [[nodiscard]] char advance() {
        m_position.column++;
        return m_source[m_current++];
    }

    Token addToken(const TokenType& type) {
        if (type == TokenType::Eof) {
            m_tokens.push_back(Token {
                .type { type },
                .lexeme { "" },
                .position { m_position },
            });
        } else {
            m_tokens.push_back(Token {
                .type { type },
                .lexeme { m_source.begin() + m_start, m_source.begin() + m_current },
                .position { m_position },
            });
        }
    }

    void newLine() {
        m_position.line++;
        m_position.column = 1;
    }

private:
    TokenList m_tokens {};
    std::string_view m_source;
    std::size_t m_current { 0 };
    std::size_t m_start { 0 };

    TokenPosition m_position { 1, 0 };
};
