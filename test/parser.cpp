#include <gtest/gtest.h>
#include "parser.h"
#include "scanner.h"

template<>
struct fmt::formatter<Parser::StatementList> {
    template<typename ParseContext>
    constexpr auto parse(ParseContext& ctx);

    template<typename FormatContext>
    auto format(const Parser::StatementList& tl, FormatContext& ctx);
};

template<typename ParseContext>
constexpr auto fmt::formatter<Parser::StatementList>::parse(ParseContext& ctx) {
    return ctx.begin();
}

template<typename FormatContext>
auto fmt::formatter<Parser::StatementList>::format(const Parser::StatementList& sl, FormatContext& ctx) {
    std::stringstream ss;

    for (const auto& statement : sl) {
        ss << statement->to_string() << '\n';
    }
    return fmt::format_to(ctx.out(), "StatementList: {}", ss.str());
}

[[nodiscard]] bool is_same(const Parser::StatementList& a, const Parser::StatementList& b) {
    if (a.size() != b.size()) {
        fmt::print(stderr, "StatementList.size() is different");
        return false;
    }

    for (int i = 0; i < a.size(); ++i) {
        if (a[i]->to_string() != b[i]->to_string()) {
            fmt::print(stderr, "{} and {} are not equal", a[i].get()->to_string(), b[i].get()->to_string());
            return false;
        }
    }

    return true;
}

TEST(parser, var_with_name) {
    Scanner s;
    Parser p;
    auto tokens = s.scan("let a;");
    auto stmts = p.parse(tokens);

    Parser::StatementList expected;
    expected.push_back(std::make_unique<Statements::VariableDefinition>("a", nullptr));
    EXPECT_TRUE(is_same(stmts, expected));

    if (HasFailure()) {
        fmt::print(stderr, "#{} {}#", stmts, expected);
    }
}

TEST(parser, variable_assignment) {
    Scanner s;
    Parser p;
    auto tokens = s.scan("let a = 10;");
    auto stmts = p.parse(tokens);

    Parser::StatementList expected;
    auto number = std::make_unique<Expressions::Number>("10");
    expected.push_back(std::make_unique<Statements::VariableDefinition>("a", std::move(number)));
    EXPECT_TRUE(is_same(stmts, expected));

    if (HasFailure()) {
        fmt::print(stderr, "#{} {}#", stmts, expected);
    }
}


















