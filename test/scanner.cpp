#include <gtest/gtest.h>
#include "scanner.h"
#include <concepts>

template <typename ...Args>
Scanner::TokenList buildExpectedList(Args&& ...args) {
    fmt::print("{}", __PRETTY_FUNCTION__);
    Scanner::TokenList tokens { args... };
    return tokens;
}

template<>
struct fmt::formatter<Scanner::TokenList> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx);

    template<typename FormatContext>
    auto format(const Scanner::TokenList& tl, FormatContext& ctx);
};

template<typename ParseContext>
constexpr auto fmt::formatter<Scanner::TokenList>::parse(ParseContext& ctx) {
    return ctx.begin();
}

template<typename FormatContext>
auto fmt::formatter<Scanner::TokenList>::format(const Scanner::TokenList& tl, FormatContext& ctx) {
    std::stringstream ss;

    for (const auto& token : tl) {
        ss << fmt::format("{}", token);
    }
    return fmt::format_to(ctx.out(), "TokenList: {}", ss.str());
}


TEST(scanner, first_token) {
    Scanner s;
    auto tokens = s.scan("(");
    Scanner::TokenList expected = buildExpectedList(
        Token { .type { TokenType::LeftParen }, .lexeme { "(" }, .position { 1, 1 }, },
        Token { .type { TokenType::Eof }, .lexeme { "" }, .position { 1, 1 }, }
    );
    EXPECT_EQ(tokens, expected);
}

TEST(scanner, double_char_token) {
    Scanner s;
    auto tokens = s.scan("!=");
    Scanner::TokenList expected = buildExpectedList(
        Token { .type { TokenType::BangEqual }, .lexeme { "!=" }, .position { 1, 1 }, },
        Token { .type { TokenType::Eof }, .lexeme { "" }, .position { 1, 1 }, }
    );

    EXPECT_EQ(tokens, expected);
}

TEST(scanner, nested_tokens) {
    Scanner s;
    auto tokens = s.scan("==");
    Scanner::TokenList expected = buildExpectedList(
        Token { .type { TokenType::EqualEqual }, .lexeme { "==" }, .position { 1, 1 }, },
        Token { .type { TokenType::Eof }, .lexeme { "" }, .position { 1, 1 }, }
    );
    EXPECT_EQ(tokens, expected);
}

TEST(scanner, simple_string) {
    Scanner s;
    auto tokens = s.scan("\"hallo\"");
    Scanner::TokenList expected = buildExpectedList(
        Token { .type { TokenType::String }, .lexeme { "hallo" }, .position { 1, 7 }, },
        Token { .type { TokenType::Eof }, .lexeme { "" }, .position { 1, 7 }, }
    );
    fmt::print("\n{}\n{}\n", expected, tokens);
    EXPECT_EQ(tokens, expected);
}

TEST(scanner, unterminated_string) {
    Scanner s;
    testing::internal::CaptureStderr();
    auto tokens = s.scan("\"hallo");
    auto output = testing::internal::GetCapturedStderr();
    EXPECT_EQ(output, "ERROR: Unterminated string. at [1, 6]\n");
}

TEST(scanner, mutli_line_string_error) {
    Scanner s;
    testing::internal::CaptureStderr();
    auto tokens = s.scan("\"hallo\n");
    auto output = testing::internal::GetCapturedStderr();
    EXPECT_EQ(output, "ERROR: No support for multi-line strings. at [1, 6]\n");
}

TEST(scanner, comment) {
    Scanner s;
    auto tokens = s.scan("# this is a comment");
    Scanner::TokenList expected = buildExpectedList(
        Token { .type { TokenType::Eof }, .lexeme { "" }, .position { 1, 19 }, }
    );
    fmt::print("\n{}\n{}\n", expected, tokens);
    EXPECT_EQ(tokens, expected);
}

TEST(scanner, simple_number) {
    Scanner s;
    auto tokens = s.scan("10");
    Scanner::TokenList expected = buildExpectedList(
        Token { .type { TokenType::Number }, .lexeme { "10" }, .position { 1, 2 }, },
        Token { .type { TokenType::Eof }, .lexeme { "" }, .position { 1, 2 }, }
    );
    fmt::print("\n{}\n{}\n", expected, tokens);
    EXPECT_EQ(tokens, expected);
}

TEST(scanner, float_number) {
    Scanner s;
    auto tokens = s.scan("10.2");
    Scanner::TokenList expected = buildExpectedList(
        Token { .type { TokenType::Number }, .lexeme { "10.2" }, .position { 1, 4 }, },
        Token { .type { TokenType::Eof }, .lexeme { "" }, .position { 1, 4 }, }
    );
    fmt::print("\n{}\n{}\n", expected, tokens);
    EXPECT_EQ(tokens, expected);
}

TEST(scanner, keyword) {
    Scanner s;
    auto tokens = s.scan("then");
    Scanner::TokenList expected = buildExpectedList(
        Token { .type { TokenType::Then }, .lexeme { "then" }, .position { 1, 4 }, },
        Token { .type { TokenType::Eof }, .lexeme { "" }, .position { 1, 4 }, }
    );
    fmt::print("\n{}\n{}\n", expected, tokens);
    EXPECT_EQ(tokens, expected);
}

TEST(scanner, keyword2) {
    Scanner s;
    auto tokens = s.scan("nuffin");
    Scanner::TokenList expected = buildExpectedList(
        Token { .type { TokenType::Nuffin }, .lexeme { "nuffin" }, .position { 1, 6 }, },
        Token { .type { TokenType::Eof }, .lexeme { "" }, .position { 1, 6 }, }
    );
    fmt::print("\n{}\n{}\n", expected, tokens);
    EXPECT_EQ(tokens, expected);
}

TEST(scanner, whole_line_of_code) {
    Scanner s;
    auto tokens = s.scan("let x = 10;");
    Scanner::TokenList expected = buildExpectedList(
        Token { .type { TokenType::Let }, .lexeme { "let" }, .position { 1, 3 }, },
        Token { .type { TokenType::Identifier }, .lexeme { "x" }, .position { 1, 5 }, },
        Token { .type { TokenType::Equal }, .lexeme { "=" }, .position { 1, 7 }, },
        Token { .type { TokenType::Number }, .lexeme { "10" }, .position { 1, 10 }, },
        Token { .type { TokenType::Semicolon }, .lexeme { ";" }, .position { 1, 11 }, },
        Token { .type { TokenType::Eof }, .lexeme { "" }, .position { 1, 11 }, }
    );
    fmt::print("\n{}\n{}\n", expected, tokens);
    EXPECT_EQ(tokens, expected);
}
