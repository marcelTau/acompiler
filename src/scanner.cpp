#include "scanner.h"
#include <tuple>

Scanner::TokenList Scanner::scan(const std::string_view source) {
    m_source = source;
    TokenList tokens;

    while (m_current < m_source.size()) {
        m_start = m_current;
        
        try {
            nextToken();
        } catch (const ScannerError& e) {
            fmt::print(stderr, "{}\n", e.what());
            //std::exit(EXIT_FAILURE);
        }
    }
    addToken(TokenType::Eof);
    return m_tokens;
}

void Scanner::nextToken() {
    #define SINGLE_TOKEN(token_char, token_name) \
        case token_char: addToken(TokenType::token_name); break;

    #define NESTED_TOKEN(first, second, first_name, second_name) \
        case first: {\
            if (expect(second)) {\
                addToken(TokenType::first_name);\
            } else {\
                addToken(TokenType::second_name);\
            }\
            break;\
        }\


    const auto c = advance();

    switch (c) {
        SINGLE_TOKEN('(', LeftParen);
        SINGLE_TOKEN(')', RightParen);
        SINGLE_TOKEN(',', Comma);
        SINGLE_TOKEN('.', Dot);
        SINGLE_TOKEN('-', Minus);
        SINGLE_TOKEN('+', Plus);
        SINGLE_TOKEN(';', Semicolon);
        SINGLE_TOKEN('*', Star);
        NESTED_TOKEN('!', '=', BangEqual, Bang);
        NESTED_TOKEN('=', '=', EqualEqual, Equal);
        NESTED_TOKEN('<', '=', LessEqual, Less);
        NESTED_TOKEN('>', '=', GreaterEqual, Greater);

        // comments
        case '#': {
            while (not isAtEnd() && peek() != '\n') {
                std::ignore = advance();
            }
        };
        case ' ':
        case '\t':
        case '\r': break;
        case '\n': newLine(); break;
        case '"': {
            string();
            break;
        }
        default: {
            if (std::isdigit(c)) {
                number();
            }
            fmt::print(stderr, "Token not found: '{}'", c);
        };
    }
    #undef SINGLE_TOKEN
    #undef NESTED_TOKEN
}

void Scanner::string() {
    while (not isAtEnd() && peek() != '"') {
        if (peek() == '\n') {
            throw ScannerError("No support for multi-line strings.", m_position);
        }
        std::ignore = advance();
    }

    if (isAtEnd()) {
        throw ScannerError("Unterminated string.", m_position);
    }

    std::ignore = advance();

    const std::string_view literal = { m_source.begin() + m_start + 1, m_source.begin() + m_current - 1 };
    addToken(TokenType::String, literal);
}

void Scanner::number() {
    while (std::isdigit(peek())) {
        std::ignore = advance();
    }

    if (peek() == '.' && std::isdigit(peekNext())) {
        std::ignore = advance();
        while (std::isdigit(peek())) {
            std::ignore = advance();
        }
    }

    const std::string_view literal = { m_source.begin() + m_start, m_source.begin() + m_current };
    addToken(TokenType::Number, literal);
}





















