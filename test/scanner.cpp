#include <gtest/gtest.h>
#include "scanner.h"

bool vecEq() {
}

TEST(scanner, first_token) {
    Scanner s;
    auto tokens = s.scan("(");
    Scanner::TokenList expected;
    expected.push_back(Token {
        .type { TokenType::LeftParen },
        .lexeme { "(" },
        .position { 1, 1 },
    });
    expected.push_back(Token {
        .type { TokenType::Eof },
        .lexeme { "" },
        .position { 1, 1 },
    });

    for (std::size_t i = 0; i < tokens.size(); ++i) {
        fmt::print("{} -- {}\n", tokens[i], expected[i]);
        EXPECT_TRUE(tokens[i] == expected[i]);
    }
}
