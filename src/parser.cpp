#include "parser.h"

Parser::StatementList Parser::parse(const TokenList& tokens) {
    StatementList statements;

    m_tokens = tokens;

    while (! isAtEnd()) {
        auto decl = declaration();

        if (!decl) {
            return statements;
        }

        statements.push_back(decl.unwrap());
    }

    return statements;
}

/// --- Helper functions --- ///
auto Parser::isAtEnd() -> bool {
    return peek().type == TokenType::Eof;
}

auto Parser::peek() -> Token {
    return m_tokens[m_current];
}

auto Parser::consume(const TokenType& ttype, std::string_view msg) -> Result<Token> {
    if (check(ttype)) {
        return Result<Token>(advance());
    } else {
        return Result<Token>::ParseError(peek(), msg);
    }
}

template <typename ...Tokens>
auto Parser::checkAndAdvance(Tokens&& ...tokens) -> bool const {
    bool found = (check(tokens) || ...);
    if (found) {
        advance();
    }
    return found;
}

auto Parser::check(const TokenType& ttype) -> bool const {
    if (isAtEnd()) {
        return false;
    }
    return peek().type == ttype;
}

auto Parser::advance() -> Token {
    if (not isAtEnd()) {
        m_current++;
    }
    return previous();
}

auto Parser::previous() -> Token {
    return m_tokens[m_current - 1];
}

auto Parser::errorStmt(const Token& token, std::string_view msg) -> Result<UniqStatement> {
    m_hasError = true;
    return Result<UniqStatement>::ParseError(token, msg);
}

auto Parser::errorExpr(const Token& token, std::string_view msg) -> Result<UniqExpression> {
    m_hasError = true;
    return Result<UniqExpression>::ParseError(token, msg);
}

/// --- Parsing functions --- ///
auto Parser::declaration() -> Result<UniqStatement> {
    if (checkAndAdvance(TokenType::Let)) {
        return varDeclaration();
    } else {
        return statement();
    }
    assert(false);
}

auto Parser::statement() -> Result<UniqStatement> {
    if (checkAndAdvance(TokenType::Print)) {
        return printStatement();
    }
    assert(false && "expression_statement()");
}

auto Parser::printStatement() -> Result<UniqStatement> {
    auto expr = expression();
    if (!expr) {
        return expr.get_err();
    }
    std::ignore = consume(TokenType::Semicolon, "Expect ';' after value.");

    auto printExpr = std::make_unique<Statements::Print>(expr.unwrap());
    return Result<UniqStatement>(std::move(printExpr));
}

auto Parser::varDeclaration() -> Result<UniqStatement> {
    const auto name = consume(TokenType::Identifier, "Expect variable name.");

    if (!name) {
        return Result<UniqStatement>::Error(name.get_err());
    }

    UniqExpression initializer;

    if (checkAndAdvance(TokenType::Equal)) {
        if (auto result = expression(); ! result) {
            initializer = nullptr;
        } else {
            initializer = result.unwrap();
        }
    }

    std::ignore = consume(TokenType::Semicolon, "Expect ';' after variable declaration.");

    auto varDefinition = std::make_unique<Statements::VariableDefinition>(name.unwrap().lexeme, std::move(initializer));
    return Result<UniqStatement>(std::move(varDefinition));
}

auto Parser::expression() -> Result<UniqExpression> {
    return assignment();
}

auto Parser::assignment() -> Result<UniqExpression> {
    auto expr = term();

    if (checkAndAdvance(TokenType::Equal)) {
        fmt::print("todo multi assignment");
    }
    return expr;
}

auto Parser::term() -> Result<UniqExpression> {
    auto expr = factor();
    if (!expr) {
        return expr.get_err();
    }

    auto expr_unwrapped = expr.unwrap();

    while (checkAndAdvance(TokenType::Plus, TokenType::Minus)) {
        auto op = previous();
        auto right = factor();

        if (!right) {
            return right.get_err();
        }
        expr_unwrapped = std::make_unique<Expressions::BinaryOperator>(std::move(expr_unwrapped), op, right.unwrap());
    }

    return expr_unwrapped;
}

auto Parser::factor() -> Result<UniqExpression> {
    auto expr = unary();
    if (!expr) {
        return Result<UniqExpression>::Error(expr.get_err());
    }

    auto expr_unwrapped = expr.unwrap();

    while (checkAndAdvance(TokenType::Star, TokenType::Slash)) {
        auto op = previous();
        auto right = unary();
        if (!right) {
            return right.get_err();
        }
        expr_unwrapped = std::make_unique<Expressions::BinaryOperator>(std::move(expr_unwrapped), op, right.unwrap());
    }

    return expr_unwrapped;
}

auto Parser::unary() -> Result<UniqExpression> {
    if (checkAndAdvance(TokenType::Bang, TokenType::Minus)) {
        auto op = previous();
        auto right = unary();

        if (!right) {
            return right.get_err();
        }
        auto unaryExpr = std::make_unique<Expressions::Unary>(op, right.unwrap());
        return Result<UniqExpression>(std::move(unaryExpr));
    } else {
        return primary(); // @todo change
    }
}

auto Parser::primary() -> Result<UniqExpression> {
    if (checkAndAdvance(TokenType::Number)) {
        auto number = std::make_unique<Expressions::Number>(previous().lexeme);
        return Result<UniqExpression>(std::move(number));
    }

    if (checkAndAdvance(TokenType::True, TokenType::False)) {
        auto bool_expression = std::make_unique<Expressions::Bool>(previous().lexeme);
        return Result<UniqExpression>(std::move(bool_expression));
    }

    if (checkAndAdvance(TokenType::Identifier)) {
        auto variable_expression = std::make_unique<Expressions::Variable>(previous().lexeme);
        return Result<UniqExpression>(std::move(variable_expression));
    }

    return errorExpr(peek(), "Expect expression.");
}

