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
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, variable_assignment) {
    Scanner s;
    Parser p;
    auto tokens = s.scan("let a = 10;");
    auto stmts = p.parse(tokens);

    Parser::StatementList expected;
    auto number = std::make_unique<Expressions::Number>("10");
    expected.push_back(std::make_unique<Statements::VariableDefinition>("a", std::move(number)));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, variable_assignment_with_plus_expression) {
    Scanner s;
    Parser p;
    auto tokens = s.scan("let a = 10 + 20;");
    auto stmts = p.parse(tokens);

    Parser::StatementList expected;

    auto lhs = std::make_unique<Expressions::Number>("10");
    Token token = Token({ .type = TokenType::Plus, .lexeme = "+", .position = { .line = 1, .column = 12 } });
    auto rhs = std::make_unique<Expressions::Number>("20");

    auto initializer = std::make_unique<Expressions::BinaryOperator>(std::move(lhs), token, std::move(rhs));
    expected.push_back(std::make_unique<Statements::VariableDefinition>("a", std::move(initializer)));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, variable_assignment_with_minus_expression) {
    Scanner s;
    Parser p;
    auto tokens = s.scan("let a = 10 - 20;");
    auto stmts = p.parse(tokens);

    Parser::StatementList expected;

    auto lhs = std::make_unique<Expressions::Number>("10");
    Token token = Token({ .type = TokenType::Minus, .lexeme = "-", .position = { .line = 1, .column = 12 } });
    auto rhs = std::make_unique<Expressions::Number>("20");

    auto initializer = std::make_unique<Expressions::BinaryOperator>(std::move(lhs), token, std::move(rhs));
    expected.push_back(std::make_unique<Statements::VariableDefinition>("a", std::move(initializer)));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, variable_assignment_with_recurring_plus_expressions) {
    Scanner s;
    Parser p;
    auto tokens = s.scan("let a = 10 + 20 + 30;");
    auto stmts = p.parse(tokens);

    Parser::StatementList expected;

    auto lhs = std::make_unique<Expressions::Number>("10");
    Token token = Token({ .type = TokenType::Plus, .lexeme = "+", .position = { .line = 1, .column = 12 } });
    auto rhs = std::make_unique<Expressions::Number>("20");
    Token token2 = Token({ .type = TokenType::Plus, .lexeme = "+", .position = { .line = 1, .column = 17 } });
    auto rrhs = std::make_unique<Expressions::Number>("30");

    auto initializer = std::make_unique<Expressions::BinaryOperator>(std::move(lhs), token, std::move(rhs));
    auto initializer2 = std::make_unique<Expressions::BinaryOperator>(std::move(initializer), token2, std::move(rrhs));
    expected.push_back(std::make_unique<Statements::VariableDefinition>("a", std::move(initializer2)));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, variable_assignment_with_recurring_mixed_plus_minus_expressions) {
    Scanner s;
    Parser p;
    auto tokens = s.scan("let a = 10 + 20 - 30;");
    auto stmts = p.parse(tokens);

    Parser::StatementList expected;

    auto lhs = std::make_unique<Expressions::Number>("10");
    Token token = Token({ .type = TokenType::Plus, .lexeme = "+", .position = { .line = 1, .column = 12 } });
    auto rhs = std::make_unique<Expressions::Number>("20");
    Token token2 = Token({ .type = TokenType::Minus, .lexeme = "-", .position = { .line = 1, .column = 17 } });
    auto rrhs = std::make_unique<Expressions::Number>("30");

    auto initializer = std::make_unique<Expressions::BinaryOperator>(std::move(lhs), token, std::move(rhs));
    auto initializer2 = std::make_unique<Expressions::BinaryOperator>(std::move(initializer), token2, std::move(rrhs));

    expected.push_back(std::make_unique<Statements::VariableDefinition>("a", std::move(initializer2)));

    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, variable_assignment_with_multiplication_expression) {
    Scanner s;
    Parser p;
    auto tokens = s.scan("let a = 10 * 20;");
    auto stmts = p.parse(tokens);

    Parser::StatementList expected;

    auto lhs = std::make_unique<Expressions::Number>("10");
    Token token = Token({ .type = TokenType::Star, .lexeme = "*", .position = { .line = 1, .column = 12 } });
    auto rhs = std::make_unique<Expressions::Number>("20");

    auto initializer = std::make_unique<Expressions::BinaryOperator>(std::move(lhs), token, std::move(rhs));
    expected.push_back(std::make_unique<Statements::VariableDefinition>("a", std::move(initializer)));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, variable_assignment_with_division_expression) {
    Scanner s;
    Parser p;
    auto tokens = s.scan("let a = 10 / 20;");
    auto stmts = p.parse(tokens);

    Parser::StatementList expected;

    auto lhs = std::make_unique<Expressions::Number>("10");
    Token token = Token({ .type = TokenType::Slash, .lexeme = "/", .position = { .line = 1, .column = 12 } });
    auto rhs = std::make_unique<Expressions::Number>("20");

    auto initializer = std::make_unique<Expressions::BinaryOperator>(std::move(lhs), token, std::move(rhs));
    expected.push_back(std::make_unique<Statements::VariableDefinition>("a", std::move(initializer)));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, variable_assignment_with_recurring_mixed_star_slash_expressions) {
    Scanner s;
    Parser p;
    auto tokens = s.scan("let a = 10 * 20 / 30;");
    auto stmts = p.parse(tokens);

    Parser::StatementList expected;

    auto lhs = std::make_unique<Expressions::Number>("10");
    Token token = Token({ .type = TokenType::Star, .lexeme = "*", .position = { .line = 1, .column = 12 } });
    auto rhs = std::make_unique<Expressions::Number>("20");
    Token token2 = Token({ .type = TokenType::Slash, .lexeme = "/", .position = { .line = 1, .column = 17 } });
    auto rrhs = std::make_unique<Expressions::Number>("30");

    auto initializer = std::make_unique<Expressions::BinaryOperator>(std::move(lhs), token, std::move(rhs));
    auto initializer2 = std::make_unique<Expressions::BinaryOperator>(std::move(initializer), token2, std::move(rrhs));

    expected.push_back(std::make_unique<Statements::VariableDefinition>("a", std::move(initializer2)));

    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, variable_assignment_with_recurring_precedence_expressions) {
    Scanner s;
    Parser p;
    auto tokens = s.scan("let a = 10 + 20 * 30;");
    auto stmts = p.parse(tokens);

    Parser::StatementList expected;

    auto lhs = std::make_unique<Expressions::Number>("10");
    Token token = Token({ .type = TokenType::Plus, .lexeme = "+", .position = { .line = 1, .column = 12 } });
    auto rhs = std::make_unique<Expressions::Number>("20");
    Token token2 = Token({ .type = TokenType::Star, .lexeme = "*", .position = { .line = 1, .column = 17 } });
    auto rrhs = std::make_unique<Expressions::Number>("30");

    // Structure:
    //    +
    // 10   *
    //    20 30

    auto initializer = std::make_unique<Expressions::BinaryOperator>(std::move(rhs), token2, std::move(rrhs));
    auto initializer2 = std::make_unique<Expressions::BinaryOperator>(std::move(lhs), token, std::move(initializer));

    expected.push_back(std::make_unique<Statements::VariableDefinition>("a", std::move(initializer2)));

    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, variable_assignment_with_unary_minus) {
    Scanner s;
    Parser p;
    auto tokens = s.scan("let a = -10;");
    auto stmts = p.parse(tokens);

    Parser::StatementList expected;
    auto number = std::make_unique<Expressions::Number>("10");
    auto token = Token({ .type = TokenType::Minus, .lexeme = "-", .position { .line = 1, .column = 9 } });
    auto unary = std::make_unique<Expressions::Unary>(token, std::move(number));
    expected.push_back(std::make_unique<Statements::VariableDefinition>("a", std::move(unary)));

    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, variable_assignment_with_unary_negation_false) {
    Scanner s;
    Parser p;
    auto tokens = s.scan("let a = !false;");
    auto stmts = p.parse(tokens);

    Parser::StatementList expected;
    auto bool_expr = std::make_unique<Expressions::Bool>("false");
    auto token = Token({ .type = TokenType::Bang, .lexeme = "!", .position { .line = 1, .column = 9 } });
    auto unary = std::make_unique<Expressions::Unary>(token, std::move(bool_expr));
    expected.push_back(std::make_unique<Statements::VariableDefinition>("a", std::move(unary)));

    fmt::print(stderr, "#{} {}#", stmts, expected);
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, variable_assignment_with_unary_negation_true) {
    Scanner s;
    Parser p;
    auto tokens = s.scan("let a = !true;");
    auto stmts = p.parse(tokens);

    Parser::StatementList expected;
    auto bool_expr = std::make_unique<Expressions::Bool>("true");
    auto token = Token({ .type = TokenType::Bang, .lexeme = "!", .position { .line = 1, .column = 9 } });
    auto unary = std::make_unique<Expressions::Unary>(token, std::move(bool_expr));
    expected.push_back(std::make_unique<Statements::VariableDefinition>("a", std::move(unary)));

    fmt::print(stderr, "#{} {}#", stmts, expected);
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, variable_assignment_with_double_unary_negation_true) {
    Scanner s;
    Parser p;
    auto tokens = s.scan("let a = !!true;");
    auto stmts = p.parse(tokens);

    Parser::StatementList expected;

    auto bool_expr = std::make_unique<Expressions::Bool>("true");
    auto token = Token({ .type = TokenType::Bang, .lexeme = "!", .position { .line = 1, .column = 9 } });
    auto token_inner = Token({ .type = TokenType::Bang, .lexeme = "!", .position { .line = 1, .column = 10 } });
    auto unary_inner = std::make_unique<Expressions::Unary>(token_inner, std::move(bool_expr));
    auto unary = std::make_unique<Expressions::Unary>(token, std::move(unary_inner));
    expected.push_back(std::make_unique<Statements::VariableDefinition>("a", std::move(unary)));

    fmt::print(stderr, "#{} {}#", stmts, expected);
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}
