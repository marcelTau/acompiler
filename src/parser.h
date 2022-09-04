#pragma once

#include <utility>
#include <vector>
#include <memory>
#include <optional>
#include <sstream>
#include "fmt/format.h"
#include "fmt/ranges.h"
#include "token.h"
#include "error.h"
#include "datatype.h"
#include "expressions.h"

template <typename Value>
struct Environment;


/// @todo ugly hacky stuff, try to fix this pls
/*
namespace Expressions {
    struct ExpressionVisitor;
    struct Expression {
        Expression() = default;
        virtual ~Expression() = default;
        virtual void accept(ExpressionVisitor& visitor) = 0;
        [[nodiscard]] virtual std::string to_string(std::size_t offset = 0) const = 0;

        DataType datatype {};
    };
} // namespace Expression
*/

namespace Statements {
    using Expressions::Expression;

    struct VariableDefinition;
    struct ExpressionStatement;
    struct Print;
    struct Function;
    struct Return;
    struct IfStatement;
    struct Block;

    struct StatementVisitor {
        virtual void visit(VariableDefinition& statement) = 0;
        virtual void visit(ExpressionStatement& statement) = 0;
        virtual void visit(Print& statement) = 0;
        virtual void visit(Function& statement) = 0;
        virtual void visit(Return& statement) = 0;
        virtual void visit(IfStatement& statement) = 0;
        virtual void visit(Block& statement) = 0;
        virtual ~StatementVisitor() = default;
    };

    struct Statement {
        virtual ~Statement() = default;
        virtual void accept(StatementVisitor& visitor) = 0;
        [[nodiscard]] virtual std::string to_string(std::size_t offset = 0) const = 0;
    };

    template<typename T>
    struct StatementAcceptor : public Statement {
        void accept(StatementVisitor& visitor) final {
            visitor.visit(static_cast<T&>(*this));
        }
    };

    struct VariableDefinition : public StatementAcceptor<VariableDefinition> {
        //VariableDefinition() = default;
        VariableDefinition(const Token& name, std::unique_ptr<Expression> initializer, DataType data_type = {})
            : name(name)
            , initializer(std::move(initializer))
            , datatype(std::move(data_type))
        {
        }

        VariableDefinition(VariableDefinition& other)
            : name(other.name)
            , initializer { std::move(other.initializer) }
            , datatype(other.datatype)
            , offset(other.offset)
            , scope_distance(other.scope_distance)
        {
        }
        
        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final {
            return fmt::format(
                    "{0:>{w}}VariableDefinition:\n"
                    "{0:>{w}}  .name = {2}\n"
                    "{0:>{w}}  .initializer =\n{3}\n"
                    "{0:>{w}}  .datatype = {4}\n"
                    "{0:>{w}}  .scope_distance = {5}\n",
                    "",  // dummy argument for padding
                    fmt::arg("w", offset),
                    name,
                    initializer ? initializer->to_string(offset + 4) : "nullptr",
                    datatype.to_string(),
                    scope_distance
            );
        }

        Token name;
        std::unique_ptr<Expression> initializer;
        DataType datatype;
        int offset { -1 };
        int scope_distance { -1 };
    };

    struct ExpressionStatement : public StatementAcceptor<ExpressionStatement> {
        ExpressionStatement(std::unique_ptr<Expression> expr)
            : expression(std::move(expr))
        {
        }

        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final {
            return fmt::format(
                    "{0:>{w}}ExpressionStatement:\n"
                    "{0:>{w}}  .expression =\n{2}\n",
                    "",  // dummy argument for padding
                    fmt::arg("w", offset),
                    expression ? expression->to_string(offset + 4) : "nullptr"
            );
        }

        std::unique_ptr<Expression> expression;
    };

    struct Print : public StatementAcceptor<Print> {
        Print(std::unique_ptr<Expression> expr)
            : expression(std::move(expr))
        {
        }

        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final {
            return fmt::format(
                    "{0:>{w}}PrintStatement:\n"
                    "{0:>{w}}  .expression = {2}\n",
                    "",  // dummy argument for padding
                    fmt::arg("w", offset),
                    expression->to_string(offset + 4)
            );
        }

        std::unique_ptr<Expression> expression;
    };

    struct Function : public StatementAcceptor<Function> {
        Function(Token name, std::vector<Token> params, std::vector<std::unique_ptr<Statement>> body, DataType return_datatype)
            : name(name)
            , params(std::move(params))
            , body(std::move(body))
            , return_datatype(std::move(return_datatype))
        {
            //stack_size = params.size() * 4; // @todo for now assume that every datatype use 4 bytes and don't care about local variables
        }

        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final {
            std::stringstream ss{};

            for (auto& statement : body) {
                ss << statement->to_string(offset + 4);
            }

            return fmt::format(
                    "{0:>{w}}FunctionStatement:\n"
                    "{0:>{w}}  .returntype = {2}\n"
                    "{0:>{w}}  .name = {3}\n"
                    "{0:>{w}}  .params = {4}\n"
                    "{0:>{w}}  .body = \n{5}\n"
                    "{0:>{w}}  .scope_distance = {6}\n",
                    "",  // dummy argument for padding
                    fmt::arg("w", offset),
                    return_datatype.to_string(),
                    name,
                    fmt::join(params, ", "),
                    ss.str(),
                    scope_distance
            );
        }

        Token name;
        std::vector<Token> params;
        std::vector<std::unique_ptr<Statement>> body;
        std::size_t stack_size { 0 };
        DataType return_datatype;
        int scope_distance { -1 };
    };

    struct Return : public StatementAcceptor<Return> {
        Return(std::unique_ptr<Expression> value)
            : value(std::move(value))
        {
        }

        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final {
            return fmt::format(
                    "{0:>{w}}Return:\n"
                    "{0:>{w}}  .value =\n{2}\n",
                    "",  // dummy argument for padding
                    fmt::arg("w", offset),
                    value ? value->to_string(offset + 4) : "nullptr"
            );
        }

        std::unique_ptr<Expression> value;
    };

    struct Block : public StatementAcceptor<Block> {
        Block(std::vector<std::unique_ptr<Statement>>  statements)
            : statements(std::move(statements))
        {}

        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final {
            std::stringstream ss;

            for (const auto& statement : statements) {
                ss << statement->to_string(offset + 4) << '\n';
            }
            return fmt::format(
                    "{0:>{w}}Block:\n"
                    "{0:>{w}}  .statements =\n{2}\n",
                    "",  // dummy argument for padding
                    fmt::arg("w", offset),
                    ss.str()
            );
        }

        std::vector<std::unique_ptr<Statement>> statements;
    };

    struct IfStatement : public StatementAcceptor<IfStatement> {
        IfStatement(std::unique_ptr<Expression> condition, std::unique_ptr<Statement> then_branch, std::unique_ptr<Statement> else_branch)
            : condition(std::move(condition))
            , then_branch(std::move(then_branch))
            , else_branch(std::move(else_branch))
        {}

        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final {
            return fmt::format(
                    "{0:>{w}}IfStatement:\n"
                    "{0:>{w}}  .condition =\n{2}\n"
                    "{0:>{w}}  .then_branch =\n{3}\n"
                    "{0:>{w}}  .else_branch =\n{4}\n",
                    "",  // dummy argument for padding
                    fmt::arg("w", offset),
                    condition ? condition->to_string(offset + 4) : "nullptr",
                    then_branch ? then_branch->to_string(offset + 4) : "nullptr",
                    else_branch ? else_branch->to_string(offset + 4) : "nullptr"
            );
        }

        std::unique_ptr<Expression> condition;
        std::unique_ptr<Statement> then_branch;
        std::unique_ptr<Statement> else_branch;
    };
} // namespace Statements

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

    /// --- Helper functions --- ///
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

    /// --- Parsing functions --- ///
    [[nodiscard]] auto declaration() -> Result<UniqStatement>;
    [[nodiscard]] auto varDeclaration() -> Result<UniqStatement>;
    [[nodiscard]] auto statement() -> Result<UniqStatement>;
    [[nodiscard]] auto ifStatement() -> Result<UniqStatement>;
    [[nodiscard]] auto returnStatement() -> Result<UniqStatement>;
    [[nodiscard]] auto expressionStatement() -> Result<UniqStatement>;
    [[nodiscard]] auto printStatement() -> Result<UniqStatement>;
    [[nodiscard]] auto function() -> Result<UniqStatement>;
    [[nodiscard]] auto block() -> Result<std::vector<UniqStatement>>;


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
