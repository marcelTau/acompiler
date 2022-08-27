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
        return Result<Token>::Error(ErrorType::ParserError, peek(), msg);
    }
}

auto Parser::checkAndAdvance(const TokenType& ttype) -> bool const {
    const auto result = check(ttype);
    if (result) {
        advance();
    }
    return result;
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
    return Result<UniqStatement>::Error(ErrorType::ParserError, token, msg);
}

auto Parser::errorExpr(const Token& token, std::string_view msg) -> Result<UniqExpression> {
    m_hasError = true;
    return Result<UniqExpression>::Error(ErrorType::ParserError, token, msg);
}

/// --- Parsing functions --- ///
auto Parser::declaration() -> Result<UniqStatement> {
    if (checkAndAdvance(TokenType::Let)) {
        return varDeclaration();
    }
    assert(false);
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
    auto expr = primary(); // @todo change this

    if (checkAndAdvance(TokenType::Equal)) {
        fmt::print("todo multi assignment");
    }
    return expr;
}

auto Parser::primary() -> Result<UniqExpression> {
    if (checkAndAdvance(TokenType::Number)) {
        auto number = std::make_unique<Expressions::Number>(previous().lexeme);
        return Result<UniqExpression>(std::move(number));
    }
    return errorExpr(peek(), "Expect expression.");
}

