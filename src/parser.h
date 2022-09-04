#pragma once

#include "expressions.h"
#include "statements.h"
#include "error.h"

class Parser {
public:
    using TokenList = std::vector<Token>;
    using StatementList = std::vector<std::unique_ptr<Statements::Statement>>;

    using Statement = Statements::Statement;
    using UniqStatement = std::unique_ptr<Statement>;

    using Expression = Expressions::Expression;
    using UniqExpression = std::unique_ptr<Expression>;

    enum struct BlockType {
        Function,
        IfStatement,
        Undefined,
    };

public:
    Parser() = default;

    StatementList parse(const TokenList& tokens);

    // ============================================================================
    // Helper Functions
    // ============================================================================
    [[nodiscard]] auto isAtEnd() -> bool;
    [[nodiscard]] auto peek() -> Token;
    [[nodiscard]] auto consume(const TokenType& ttype, const std::string& msg) -> Result<Token>;

    template <typename ...Tokens>
    [[nodiscard]] auto checkAndAdvance(Tokens&& ...tokens) -> bool const;
    [[nodiscard]] auto check(const TokenType& ttype) -> bool const;

    auto advance() -> Token;
    [[nodiscard]] auto previous() -> Token;

    [[nodiscard]] auto errorStmt(const Token& token, const std::string& msg) -> Result<UniqStatement>;
    [[nodiscard]] auto errorExpr(const Token& token, const std::string& msg) -> Result<UniqExpression>;

    // ============================================================================
    // Statements
    // ============================================================================
    [[nodiscard]] auto declaration() -> Result<UniqStatement>;
    [[nodiscard]] auto varDeclaration() -> Result<UniqStatement>;
    [[nodiscard]] auto statement() -> Result<UniqStatement>;
    [[nodiscard]] auto ifStatement() -> Result<UniqStatement>;
    [[nodiscard]] auto returnStatement() -> Result<UniqStatement>;
    [[nodiscard]] auto expressionStatement() -> Result<UniqStatement>;
    [[nodiscard]] auto printStatement() -> Result<UniqStatement>;
    [[nodiscard]] auto function() -> Result<UniqStatement>;
    [[nodiscard]] auto block() -> Result<std::vector<UniqStatement>>;


    // ============================================================================
    // Expressions
    // ============================================================================
    [[nodiscard]] auto expression() -> Result<UniqExpression>;
    [[nodiscard]] auto assignment() -> Result<UniqExpression>;
    [[nodiscard]] auto or_() -> Result<UniqExpression>;
    [[nodiscard]] auto and_() -> Result<UniqExpression>;
    [[nodiscard]] auto equality() -> Result<UniqExpression>;
    [[nodiscard]] auto comparison() -> Result<UniqExpression>;
    [[nodiscard]] auto term() -> Result<UniqExpression>;
    [[nodiscard]] auto factor() -> Result<UniqExpression>;
    [[nodiscard]] auto unary() -> Result<UniqExpression>;
    [[nodiscard]] auto call() -> Result<UniqExpression>;
    [[nodiscard]] auto primary() -> Result<UniqExpression>;

private:
    TokenList m_tokens;
    std::size_t m_current { 0 };
    bool m_hasError { false };
    int current_function_end_depth { 0 };
    BlockType current_block_type = BlockType::Undefined;
};
