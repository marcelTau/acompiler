#include "parser.h"
#include "fmt/ranges.h"
#include "spdlog/spdlog.h"

Parser::StatementList Parser::parse(const TokenList& tokens) {
    StatementList statements;

    m_tokens = tokens;

    spdlog::info(fmt::format("Start parsing with {} tokens", m_tokens.size()));

    while (! isAtEnd()) {
        auto decl = declaration();

        if (!decl) {
            spdlog::error(fmt::format("ParserError: {}", decl.get_err().msg));
            return statements;
        }

        statements.push_back(decl.unwrap());
    }

    spdlog::info(fmt::format("Finished parsing with {} statements", statements.size()));
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
    } else if (checkAndAdvance(TokenType::Fun)) {
        return function();
    } else {
        return statement();
    }
    assert(false);
}

auto Parser::function() -> Result<UniqStatement> {
    auto name = consume(TokenType::Identifier, "Expect function name.");
    std::ignore = consume(TokenType::LeftParen, "Expect '(' after function name.");

    std::vector<Token> params;
    if (!check(TokenType::RightParen)) {
        auto param = consume(TokenType::Identifier, "Expect parameter name.");
        if (!param) {
            return param.get_err();
        }
        params.push_back(param.unwrap());

        while (checkAndAdvance(TokenType::Comma)) {
            param = consume(TokenType::Identifier, "Expect parameter name.");
            if (!param) {
                return param.get_err();
            }
            params.push_back(param.unwrap());
        }
    }

    std::ignore = consume(TokenType::RightParen, "Expect ')' after parameters.");

    std::ignore = consume(TokenType::Arrow, "Expect '->' after function declaration.");

    auto return_datatype_token = consume(TokenType::Identifier, "Expect return value after '->'.");

    if (!return_datatype_token) {
        return return_datatype_token.get_err();
    }

    auto return_datatype_name = return_datatype_token.unwrap().lexeme;

    DataType datatype {};

    try {
        datatype = availableDataTypes.at(return_datatype_name);
    } catch (std::out_of_range&) {
        return Result<UniqStatement>::ParseError(return_datatype_token.unwrap(), 
                                                 fmt::format("Expect datatype after '->' found {}.", 
                                                 return_datatype_name));
    }

    auto block_stmts = block();

    if (!block_stmts) {
        return block_stmts.get_err();
    }

    auto function_stmt = std::make_unique<Statements::Function>(name.unwrap(), params, block_stmts.unwrap(), datatype);
    return Result<UniqStatement>(std::move(function_stmt));
}

auto Parser::block() -> Result<std::vector<UniqStatement>> {
    std::vector<UniqStatement> statements;

    // @todo this needs to change when there are other constructs that use the
    // end keyword such as 'if' or 'for'
    while (!check(TokenType::End) && !isAtEnd()) {
        auto stmt = declaration();
        if (!stmt) {
            return stmt.get_err();
        }
        statements.emplace_back(stmt.unwrap());
    }
    std::ignore = consume(TokenType::End, "Expect 'end' after block.");
    return statements;
}

auto Parser::statement() -> Result<UniqStatement> {
    if (checkAndAdvance(TokenType::Print)) {
        return printStatement();
    }
    if (checkAndAdvance(TokenType::Return)) {
        return returnStatement();
    }
    assert(false && "expression_statement()");
}

auto Parser::returnStatement() -> Result<UniqStatement> {
    std::unique_ptr<Expression> value;
    if (!check(TokenType::Semicolon)) {
        auto value_result = expression();
        if (!value_result) {
            return value_result.get_err();
        }
        value = value_result.unwrap();
    }

    std::ignore = consume(TokenType::Semicolon, "Expect ';' after return.");

    auto returnStatement = std::make_unique<Statements::Return>(std::move(value));
    return Result<UniqStatement>(std::move(returnStatement));
}

auto Parser::expressionStatement() -> Result<UniqStatement> {
    auto expr = expression();
    if (!expr) {
        return expr.get_err();
    }
    std::ignore = consume(TokenType::Semicolon, "Expect ';' after expression.");

    assert(false && "todo ddasdf");
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

    DataType datatype = {};

    if (checkAndAdvance(TokenType::Colon)) {
        auto datatype_token = consume(TokenType::Identifier, "Exptect datatype after ':'.");

        if (!datatype_token) {
            return datatype_token.get_err();
        }

        const auto datatype_name = datatype_token.unwrap().lexeme;

        try {
            datatype = availableDataTypes.at(datatype_name);
        } catch (std::out_of_range& e) {
            return Result<UniqStatement>::ParseError(datatype_token.unwrap(), 
                    fmt::format("Expect datatype after ':' found {}.", datatype_name));
        }
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

    auto varDefinition = std::make_unique<Statements::VariableDefinition>(name.unwrap(), std::move(initializer), datatype);
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
        auto variable_expression = std::make_unique<Expressions::Variable>(previous());
        return Result<UniqExpression>(std::move(variable_expression));
    }

    return errorExpr(peek(), "Expect expression.");
}

