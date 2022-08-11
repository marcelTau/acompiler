#include "scanner.h"

Scanner::TokenList Scanner::scan(const std::string_view source) {
    m_source = source;
    TokenList tokens;

    while (m_current < m_source.size()) {
        m_start = m_current;
        
        if (nextToken()) {
            fmt::print(stderr, "Error occured during scanning.");
            std::exit(EXIT_FAILURE);
        }
    }
    addToken(TokenType::Eof);
    return m_tokens;
}

bool Scanner::nextToken() {
    const auto c = advance();

    switch (c) {
        case '(': addToken(TokenType::LeftParen); break;
        case '\n': newLine(); break;
        default: {
            fmt::print(stderr, "Token not found: {}", c);
            return true;
        };
    }

    return false;
}
