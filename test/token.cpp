#include <gtest/gtest.h>
#include <fmt/core.h>
#include "token.h"

TEST(token, correct_print_of_token_type) {
    Token token {
        .type { TokenType::For },
        .lexeme { "for" },
        .position { 0, 0 },
    };
    std::string expected = "Token (For, 'for', [0,0])";
    EXPECT_EQ(fmt::format("{}", token), expected);
}
