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
