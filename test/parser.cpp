#include <csignal>
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

    ss << '\n';

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

TEST(parser, variable_assignment) {
    Scanner s;
    Parser p;
    auto tokens = s.scan("let a = 10;");
    auto stmts = p.parse(tokens);

    Parser::StatementList expected;
    auto number = std::make_unique<Expressions::Number>("10");
    Token name { .type = TokenType::Identifier, .lexeme = "a", .position = { .line = 1, .column = 5 }, };
    expected.push_back(std::make_unique<Statements::VariableDefinition>(name, std::move(number)));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, variable_assignment_with_type_annotation_int) {
    Scanner s;
    Parser p;
    auto tokens = s.scan("let a: Int = 10;");
    auto stmts = p.parse(tokens);

    Parser::StatementList expected;
    auto dt = availableDataTypes.at("Int");
    auto number = std::make_unique<Expressions::Number>("10");
    Token name { .type = TokenType::Identifier, .lexeme = "a", .position = { .line = 1, .column = 5 }, };
    expected.push_back(std::make_unique<Statements::VariableDefinition>(name, std::move(number), dt));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, variable_assignment_with_type_annotation_bool) {
    Scanner s;
    Parser p;
    auto tokens = s.scan("let a: Bool = true;");
    auto stmts = p.parse(tokens);

    Parser::StatementList expected;
    auto dt = availableDataTypes.at("Bool");
    auto boolExpr = std::make_unique<Expressions::Bool>("true");
    Token name { .type = TokenType::Identifier, .lexeme = "a", .position = { .line = 1, .column = 5 }, };
    expected.push_back(std::make_unique<Statements::VariableDefinition>(name, std::move(boolExpr), dt));
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

    fmt::print(stderr, "{} {}", lhs->to_string(), rhs->to_string());

    auto initializer = std::make_unique<Expressions::BinaryOperator>(std::move(lhs), token, std::move(rhs));

    Token name { .type = TokenType::Identifier, .lexeme = "a", .position = { .line = 1, .column = 5 }, };
    expected.push_back(std::make_unique<Statements::VariableDefinition>(name, std::move(initializer)));
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
    Token name { .type = TokenType::Identifier, .lexeme = "a", .position = { .line = 1, .column = 5 }, };
    expected.push_back(std::make_unique<Statements::VariableDefinition>(name, std::move(initializer)));
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
    Token name { .type = TokenType::Identifier, .lexeme = "a", .position = { .line = 1, .column = 5 }, };
    expected.push_back(std::make_unique<Statements::VariableDefinition>(name, std::move(initializer2)));
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

    Token name { .type = TokenType::Identifier, .lexeme = "a", .position = { .line = 1, .column = 5 }, };
    expected.push_back(std::make_unique<Statements::VariableDefinition>(name, std::move(initializer2)));

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
    Token name { .type = TokenType::Identifier, .lexeme = "a", .position = { .line = 1, .column = 5 }, };
    expected.push_back(std::make_unique<Statements::VariableDefinition>(name, std::move(initializer)));
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
    Token name { .type = TokenType::Identifier, .lexeme = "a", .position = { .line = 1, .column = 5 }, };
    expected.push_back(std::make_unique<Statements::VariableDefinition>(name, std::move(initializer)));
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

    Token name { .type = TokenType::Identifier, .lexeme = "a", .position = { .line = 1, .column = 5 }, };
    expected.push_back(std::make_unique<Statements::VariableDefinition>(name, std::move(initializer2)));

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

    Token name { .type = TokenType::Identifier, .lexeme = "a", .position = { .line = 1, .column = 5 }, };
    expected.push_back(std::make_unique<Statements::VariableDefinition>(name, std::move(initializer2)));

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
    Token name { .type = TokenType::Identifier, .lexeme = "a", .position = { .line = 1, .column = 5 }, };
    expected.push_back(std::make_unique<Statements::VariableDefinition>(name, std::move(unary)));

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
    Token name { .type = TokenType::Identifier, .lexeme = "a", .position = { .line = 1, .column = 5 }, };
    expected.push_back(std::make_unique<Statements::VariableDefinition>(name, std::move(unary)));

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
    Token name { .type = TokenType::Identifier, .lexeme = "a", .position = { .line = 1, .column = 5 }, };
    expected.push_back(std::make_unique<Statements::VariableDefinition>(name, std::move(unary)));

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
    Token name { .type = TokenType::Identifier, .lexeme = "a", .position = { .line = 1, .column = 5 }, };
    expected.push_back(std::make_unique<Statements::VariableDefinition>(name, std::move(unary)));

    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, variable_assignment_with_variable_identifier) {
    Scanner s;
    Parser p;
    auto tokens = s.scan("let a = 10; let b = a");
    auto stmts = p.parse(tokens);

    Parser::StatementList expected;
    auto number = std::make_unique<Expressions::Number>("10");
    Token nameA { .type = TokenType::Identifier, .lexeme = "a", .position = { .line = 1, .column = 5 }, };
    expected.push_back(std::make_unique<Statements::VariableDefinition>(nameA, std::move(number)));

    Token nameB = { .type = TokenType::Identifier, .lexeme = "b", .position = { .line = 1, .column = 17 } };
    Token variable = { .type = TokenType::Identifier, .lexeme = "a", .position = { .line = 1, .column = 21 } };
    auto var = std::make_unique<Expressions::Variable>(variable);
    expected.push_back(std::make_unique<Statements::VariableDefinition>(nameB, std::move(var)));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, print_test_single_value) {
    Scanner s;
    Parser p;
    auto tokens = s.scan("print 10;");
    auto stmts = p.parse(tokens);

    Parser::StatementList expected;
    auto number = std::make_unique<Expressions::Number>("10");
    expected.push_back(std::make_unique<Statements::Print>(std::move(number)));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, print_test_complex) {
    Scanner s;
    Parser p;
    auto tokens = s.scan("print 10 + 2 * 3;");
    auto stmts = p.parse(tokens);

    //  print
    //       +
    //     10  *
    //        2 3

    Parser::StatementList expected;
    auto number10 = std::make_unique<Expressions::Number>("10");
    auto number2 = std::make_unique<Expressions::Number>("2");
    auto number3 = std::make_unique<Expressions::Number>("3");

    auto tokenPlus = Token({ .type = TokenType::Plus, .lexeme = "+", .position = { .line = 1, .column = 10 } });
    auto tokenStar = Token({ .type = TokenType::Star, .lexeme = "*", .position = { .line = 1, .column = 14 } });

    auto binaryStar = std::make_unique<Expressions::BinaryOperator>(std::move(number2), tokenStar, std::move(number3));
    auto binaryPlus = std::make_unique<Expressions::BinaryOperator>(std::move(number10), tokenPlus, std::move(binaryStar));

    expected.push_back(std::make_unique<Statements::Print>(std::move(binaryPlus)));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, empty_function) {
    Scanner s;
    Parser p;
    Parser::StatementList expected;
    auto tokens = s.scan("fun foo() -> Int end");
    auto stmts = p.parse(tokens);

    auto name = Token({ .type = TokenType::Identifier, .lexeme = "foo", .position = { .line = 1, .column = 7 } });
    auto returnType = availableDataTypes.at("Int");
    std::vector<std::unique_ptr<Statements::Statement>> params {};
    std::vector<std::unique_ptr<Statements::Statement>> body {};
    auto funcStmt = std::make_unique<Statements::Function>(name, std::move(params), std::move(body), returnType);

    expected.push_back(std::move(funcStmt));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, non_empty_function) {
    Scanner s;
    Parser p;
    Parser::StatementList expected;
    auto tokens = s.scan("fun foo() -> Int\nlet a = 10;\nend");
    auto stmts = p.parse(tokens);

    auto name = Token({ .type = TokenType::Identifier, .lexeme = "foo", .position = { .line = 1, .column = 7 } });
    std::vector<std::unique_ptr<Statements::Statement>> params {};
    std::vector<std::unique_ptr<Statements::Statement>> body {};

    auto returnType = availableDataTypes.at("Int");

    auto number = std::make_unique<Expressions::Number>("10");
    Token name2 = { .type = TokenType::Identifier, .lexeme = "a", .position = { .line = 2, .column = 6 }, };
    body.push_back(std::make_unique<Statements::VariableDefinition>(name2, std::move(number)));

    auto funcStmt = std::make_unique<Statements::Function>(name, std::move(params), std::move(body), returnType);

    expected.push_back(std::move(funcStmt));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, return_without_expression) {
    Scanner s;
    Parser p;
    Parser::StatementList expected;
    auto tokens = s.scan("return;");
    auto stmts = p.parse(tokens);

    std::unique_ptr<Expressions::Expression> value;
    auto returnStmt = std::make_unique<Statements::Return>(std::move(value));

    expected.push_back(std::move(returnStmt));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, return_with_simple_expression) {
    Scanner s;
    Parser p;
    Parser::StatementList expected;
    auto tokens = s.scan("return 8;");
    auto stmts = p.parse(tokens);

    auto value = std::make_unique<Expressions::Number>("8");
    auto returnStmt = std::make_unique<Statements::Return>(std::move(value));

    expected.push_back(std::move(returnStmt));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, return_with_complex_expression) {
    Scanner s;
    Parser p;
    Parser::StatementList expected;
    auto tokens = s.scan("return 1 + 2;");
    auto stmts = p.parse(tokens);

    auto number1 = std::make_unique<Expressions::Number>("1");
    auto number2 = std::make_unique<Expressions::Number>("2");
    Token tokenPlus = Token({ .type = TokenType::Plus, .lexeme = "+", .position = { .line = 1, .column = 10 } });

    auto value = std::make_unique<Expressions::BinaryOperator>(std::move(number1), tokenPlus, std::move(number2));

    auto returnStmt = std::make_unique<Statements::Return>(std::move(value));

    expected.push_back(std::move(returnStmt));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, new_variable_assignment_with_type_annotation_IMPORTANT) {
    Scanner s;
    Parser p;
    Parser::StatementList expected;
    auto tokens = s.scan("let x: Int = 10; x = 20;");
    auto stmts = p.parse(tokens);

    auto varName = Token{ .type = TokenType::Identifier, .lexeme = "x", .position = { .line = 1, .column = 5 }};
    auto varInitializer = std::make_unique<Expressions::Number>("10");
    auto datatype = DataType { .name = "Int", .size = 8 };

    auto varDefinition = std::make_unique<Statements::VariableDefinition>(varName, std::move(varInitializer), datatype);

    auto varName2 = Token{ .type = TokenType::Identifier, .lexeme = "x", .position = { .line = 1, .column = 18 }};
    auto value = std::make_unique<Expressions::Number>("20");
    auto expr = std::make_unique<Expressions::Assignment>(varName2, std::move(value));
    auto exprStatement = std::make_unique<Statements::ExpressionStatement>(std::move(expr));

    expected.push_back(std::move(varDefinition));
    expected.push_back(std::move(exprStatement));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, logical_or_simple) {
    Scanner s;
    Parser p;
    Parser::StatementList expected;
    auto tokens = s.scan("let x: Bool = 1 or 2;");
    auto stmts = p.parse(tokens);

    auto varName = Token{ .type = TokenType::Identifier, .lexeme = "x", .position = { .line = 1, .column = 5 }};

    auto lhs = std::make_unique<Expressions::Number>("1");
    auto op = Token{ .type = TokenType::Or, .lexeme = "or", .position = { .line = 1, .column = 18 }};
    auto rhs = std::make_unique<Expressions::Number>("2");
    auto varInitializer = std::make_unique<Expressions::Logical>(std::move(lhs), op, std::move(rhs));

    auto datatype = DataType { .name = "Bool", .size = 8 };
    auto varDefinition = std::make_unique<Statements::VariableDefinition>(varName, std::move(varInitializer), datatype);
    expected.push_back(std::move(varDefinition));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, logical_and_simple) {
    Scanner s;
    Parser p;
    Parser::StatementList expected;
    auto tokens = s.scan("let x: Bool = 1 and 2;");
    auto stmts = p.parse(tokens);

    auto varName = Token{ .type = TokenType::Identifier, .lexeme = "x", .position = { .line = 1, .column = 5 }};

    auto lhs = std::make_unique<Expressions::Number>("1");
    auto op = Token{ .type = TokenType::And, .lexeme = "and", .position = { .line = 1, .column = 19 }};
    auto rhs = std::make_unique<Expressions::Number>("2");
    auto varInitializer = std::make_unique<Expressions::Logical>(std::move(lhs), op, std::move(rhs));

    auto datatype = DataType { .name = "Bool", .size = 8 };
    auto varDefinition = std::make_unique<Statements::VariableDefinition>(varName, std::move(varInitializer), datatype);
    expected.push_back(std::move(varDefinition));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, logical_or_simple_multiple) {
    Scanner s;
    Parser p;
    Parser::StatementList expected;
    auto tokens = s.scan("let x: Bool = 1 or 2 or 3;");
    auto stmts = p.parse(tokens);

    auto varName = Token{ .type = TokenType::Identifier, .lexeme = "x", .position = { .line = 1, .column = 5 }};

    auto lhs = std::make_unique<Expressions::Number>("1");
    auto op = Token{ .type = TokenType::Or, .lexeme = "or", .position = { .line = 1, .column = 18 }};
    auto mid = std::make_unique<Expressions::Number>("2");

    auto op2 = Token{ .type = TokenType::Or, .lexeme = "or", .position = { .line = 1, .column = 23 }};
    auto rhs = std::make_unique<Expressions::Number>("3");

    auto lhsInitializer = std::make_unique<Expressions::Logical>(std::move(lhs), op, std::move(mid));
    auto varInitializer = std::make_unique<Expressions::Logical>(std::move(lhsInitializer), op2, std::move(rhs));

    auto datatype = DataType { .name = "Bool", .size = 8 };
    auto varDefinition = std::make_unique<Statements::VariableDefinition>(varName, std::move(varInitializer), datatype);
    expected.push_back(std::move(varDefinition));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, logical_and_simple_multiple) {
    Scanner s;
    Parser p;
    Parser::StatementList expected;
    auto tokens = s.scan("let x: Bool = 1 and 2 and 3;");
    auto stmts = p.parse(tokens);

    auto varName = Token{ .type = TokenType::Identifier, .lexeme = "x", .position = { .line = 1, .column = 5 }};

    auto lhs = std::make_unique<Expressions::Number>("1");
    auto op = Token{ .type = TokenType::And, .lexeme = "and", .position = { .line = 1, .column = 19 }};
    auto mid = std::make_unique<Expressions::Number>("2");

    auto op2 = Token{ .type = TokenType::And, .lexeme = "and", .position = { .line = 1, .column = 25 }};
    auto rhs = std::make_unique<Expressions::Number>("3");

    auto lhsInitializer = std::make_unique<Expressions::Logical>(std::move(lhs), op, std::move(mid));
    auto varInitializer = std::make_unique<Expressions::Logical>(std::move(lhsInitializer), op2, std::move(rhs));

    auto datatype = DataType { .name = "Bool", .size = 8 };
    auto varDefinition = std::make_unique<Statements::VariableDefinition>(varName, std::move(varInitializer), datatype);
    expected.push_back(std::move(varDefinition));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, logical_and_simple_multiple_mixed) {
    Scanner s;
    Parser p;
    Parser::StatementList expected;
    auto tokens = s.scan("let x: Bool = 1 and 2 or 3;");
    auto stmts = p.parse(tokens);

    auto varName = Token{ .type = TokenType::Identifier, .lexeme = "x", .position = { .line = 1, .column = 5 }};

    auto lhs = std::make_unique<Expressions::Number>("1");
    auto op = Token{ .type = TokenType::And, .lexeme = "and", .position = { .line = 1, .column = 19 }};
    auto mid = std::make_unique<Expressions::Number>("2");

    auto op2 = Token{ .type = TokenType::Or, .lexeme = "or", .position = { .line = 1, .column = 24 }};
    auto rhs = std::make_unique<Expressions::Number>("3");

    auto lhsInitializer = std::make_unique<Expressions::Logical>(std::move(lhs), op, std::move(mid));
    auto varInitializer = std::make_unique<Expressions::Logical>(std::move(lhsInitializer), op2, std::move(rhs));

    auto datatype = DataType { .name = "Bool", .size = 8 };
    auto varDefinition = std::make_unique<Statements::VariableDefinition>(varName, std::move(varInitializer), datatype);
    expected.push_back(std::move(varDefinition));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, equality_simple_equal_equal) {
    Scanner s;
    Parser p;
    Parser::StatementList expected;
    auto tokens = s.scan("let x: Bool = 1 == 2;");
    auto stmts = p.parse(tokens);

    auto varName = Token{ .type = TokenType::Identifier, .lexeme = "x", .position = { .line = 1, .column = 5 }};

    auto lhs = std::make_unique<Expressions::Number>("1");
    auto op = Token{ .type = TokenType::EqualEqual, .lexeme = "==", .position = { .line = 1, .column = 17 }};
    auto rhs = std::make_unique<Expressions::Number>("2");
    auto varInitializer = std::make_unique<Expressions::BinaryOperator>(std::move(lhs), op, std::move(rhs));

    auto datatype = DataType { .name = "Bool", .size = 8 };
    auto varDefinition = std::make_unique<Statements::VariableDefinition>(varName, std::move(varInitializer), datatype);
    expected.push_back(std::move(varDefinition));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, equality_simple_bang_equal) {
    Scanner s;
    Parser p;
    Parser::StatementList expected;
    auto tokens = s.scan("let x: Bool = 1 != 2 or true;");
    auto stmts = p.parse(tokens);

    auto varName = Token{ .type = TokenType::Identifier, .lexeme = "x", .position = { .line = 1, .column = 5 }};

    auto lhs = std::make_unique<Expressions::Number>("1");
    auto op = Token{ .type = TokenType::BangEqual, .lexeme = "!=", .position = { .line = 1, .column = 17 }};
    auto rhs = std::make_unique<Expressions::Number>("2");
    auto varInitializer = std::make_unique<Expressions::BinaryOperator>(std::move(lhs), op, std::move(rhs));

    auto rrhs = std::make_unique<Expressions::Bool>("true");
    auto op2 = Token{ .type = TokenType::Or, .lexeme = "or", .position = { .line = 1, .column = 22 }};
    auto varInitializer2 = std::make_unique<Expressions::Logical>(std::move(varInitializer), op2, std::move(rrhs));

    auto datatype = DataType { .name = "Bool", .size = 8 };
    auto varDefinition = std::make_unique<Statements::VariableDefinition>(varName, std::move(varInitializer2), datatype);
    expected.push_back(std::move(varDefinition));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, comparison_simple_less) {
    Scanner s;
    Parser p;
    Parser::StatementList expected;
    auto tokens = s.scan("let x: Bool = 1 < 2;");
    auto stmts = p.parse(tokens);

    auto varName = Token{ .type = TokenType::Identifier, .lexeme = "x", .position = { .line = 1, .column = 5 }};

    auto lhs = std::make_unique<Expressions::Number>("1");
    auto op = Token{ .type = TokenType::Less, .lexeme = "<", .position = { .line = 1, .column = 17 }};
    auto rhs = std::make_unique<Expressions::Number>("2");
    auto varInitializer = std::make_unique<Expressions::BinaryOperator>(std::move(lhs), op, std::move(rhs));

    auto datatype = DataType { .name = "Bool", .size = 8 };
    auto varDefinition = std::make_unique<Statements::VariableDefinition>(varName, std::move(varInitializer), datatype);
    expected.push_back(std::move(varDefinition));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, comparison_simple_greater) {
    Scanner s;
    Parser p;
    Parser::StatementList expected;
    auto tokens = s.scan("let x: Bool = 1 > 2;");
    auto stmts = p.parse(tokens);

    auto varName = Token{ .type = TokenType::Identifier, .lexeme = "x", .position = { .line = 1, .column = 5 }};

    auto lhs = std::make_unique<Expressions::Number>("1");
    auto op = Token{ .type = TokenType::Greater, .lexeme = ">", .position = { .line = 1, .column = 17 }};
    auto rhs = std::make_unique<Expressions::Number>("2");
    auto varInitializer = std::make_unique<Expressions::BinaryOperator>(std::move(lhs), op, std::move(rhs));

    auto datatype = DataType { .name = "Bool", .size = 8 };
    auto varDefinition = std::make_unique<Statements::VariableDefinition>(varName, std::move(varInitializer), datatype);
    expected.push_back(std::move(varDefinition));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, comparison_simple_greater_eq) {
    Scanner s;
    Parser p;
    Parser::StatementList expected;
    auto tokens = s.scan("let x: Bool = 1 >= 2;");
    auto stmts = p.parse(tokens);

    auto varName = Token{ .type = TokenType::Identifier, .lexeme = "x", .position = { .line = 1, .column = 5 }};

    auto lhs = std::make_unique<Expressions::Number>("1");
    auto op = Token{ .type = TokenType::GreaterEqual, .lexeme = ">=", .position = { .line = 1, .column = 17 }};
    auto rhs = std::make_unique<Expressions::Number>("2");
    auto varInitializer = std::make_unique<Expressions::BinaryOperator>(std::move(lhs), op, std::move(rhs));

    auto datatype = DataType { .name = "Bool", .size = 8 };
    auto varDefinition = std::make_unique<Statements::VariableDefinition>(varName, std::move(varInitializer), datatype);
    expected.push_back(std::move(varDefinition));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, simple_if_statement) {
    Scanner s;
    Parser p;
    Parser::StatementList expected;
    auto tokens = s.scan("if 1 == 1 then return 2; end");
    auto stmts = p.parse(tokens);

    auto lhs = std::make_unique<Expressions::Number>("1");
    auto op = Token{ .type = TokenType::EqualEqual, .lexeme = "==", .position = { .line = 1, .column = 6 }};
    auto rhs = std::make_unique<Expressions::Number>("1");

    auto condition = std::make_unique<Expressions::BinaryOperator>(std::move(lhs), op, std::move(rhs));

    auto returnValue = std::make_unique<Expressions::Number>("2");
    auto then_branch = std::make_unique<Statements::Return>(std::move(returnValue));

    std::vector<std::unique_ptr<Statements::Statement>> block_statements;

    block_statements.push_back(std::move(then_branch));
    auto then_block = std::make_unique<Statements::Block>(std::move(block_statements));

    auto ifStatement = std::make_unique<Statements::IfStatement>(std::move(condition), std::move(then_block), nullptr);

    expected.push_back(std::move(ifStatement));
    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, simple_print_statement) {
    Scanner s;
    Parser p;
    Parser::StatementList expected;
    auto tokens = s.scan("print 10;");
    auto stmts = p.parse(tokens);

    auto expression = std::make_unique<Expressions::Number>("10");
    auto printStmt = std::make_unique<Statements::Print>(std::move(expression));

    expected.push_back(std::move(printStmt));

    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}

TEST(parser, print_statement_with_expression) {
    Scanner s;
    Parser p;
    Parser::StatementList expected;
    auto tokens = s.scan("print 10 * 3;");
    auto stmts = p.parse(tokens);

    auto lhs = std::make_unique<Expressions::Number>("10");
    auto op = Token{ .type = TokenType::Star, .lexeme = "*", .position = { .line = 1, .column = 10 }};
    auto rhs = std::make_unique<Expressions::Number>("3");

    auto expression = std::make_unique<Expressions::BinaryOperator>(std::move(lhs), op, std::move(rhs));
    auto printStmt = std::make_unique<Statements::Print>(std::move(expression));

    expected.push_back(std::move(printStmt));

    EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
}


//TEST(parser, if_with_else) {
    //Scanner s;
    //Parser p;
    //Parser::StatementList expected;
    //auto tokens = s.scan("if 1 == 1 then return 2; else return 3; end");
    //auto stmts = p.parse(tokens);

    //auto lhs = std::make_unique<Expressions::Number>("1");
    //auto op = Token{ .type = TokenType::EqualEqual, .lexeme = "==", .position = { .line = 1, .column = 6 }};
    //auto rhs = std::make_unique<Expressions::Number>("1");

    //auto condition = std::make_unique<Expressions::BinaryOperator>(std::move(lhs), op, std::move(rhs));

    //auto returnValue = std::make_unique<Expressions::Number>("2");
    //auto then_branch = std::make_unique<Statements::Return>(std::move(returnValue));

    //std::vector<std::unique_ptr<Statements::Statement>> block_statements;
    //block_statements.push_back(std::move(then_branch));
    //auto then_block = std::make_unique<Statements::Block>(std::move(block_statements));

    //auto returnValue2 = std::make_unique<Expressions::Number>("3");
    //auto else_branch = std::make_unique<Statements::Return>(std::move(returnValue2));

    //std::vector<std::unique_ptr<Statements::Statement>> block_statements2;
    //block_statements2.push_back(std::move(else_branch));
    //auto else_block = std::make_unique<Statements::Block>(std::move(block_statements2));

    //auto ifStatement = std::make_unique<Statements::IfStatement>(std::move(condition), std::move(then_block), std::move(else_block));

    //expected.push_back(std::move(ifStatement));
    //EXPECT_TRUE(is_same(stmts, expected)) << fmt::format("#{} {}#", stmts, expected);
//}
