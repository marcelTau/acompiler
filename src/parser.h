#pragma once

#include <utility>
#include <vector>
#include <memory>
#include <optional>
#include <charconv>
#include <cassert>
#include "token.h"
#include "error.h"

/// @todo ugly hacky stuff, try to fix this pls
namespace Expressions {
    struct ExpressionVisitor;
    struct Expression {
        virtual ~Expression() = default;
        virtual void accept(ExpressionVisitor& visitor) = 0;
        
        virtual std::string to_string() = 0;
        // @todo datatype
    };
} // namespace Expression


namespace Statements {
    using Expressions::Expression;

    struct VariableDefinition;
    struct ExpressionStatement;
    //struct Assignment;

    struct StatementVisitor {
        virtual void visit(VariableDefinition& statement) = 0;
        virtual void visit(ExpressionStatement& statement) = 0;
        //virtual void visit(Assignment& statement) = 0;

        virtual ~StatementVisitor() = default;
    };

    struct Statement {
        virtual ~Statement() = default;
        virtual void accept(StatementVisitor& visitor) = 0;
        virtual std::string to_string() = 0;
    };

    template<typename T>
    struct StatementAcceptor : public Statement {
        void accept(StatementVisitor& visitor) final {
            visitor.visit(static_cast<T&>(*this));
        }
    };

    struct VariableDefinition : public StatementAcceptor<VariableDefinition> {
        VariableDefinition(std::string_view name, std::unique_ptr<Expression> initializer)
            : name(name)
            , initializer(std::move(initializer))
        {
        }

        std::string name;
        std::unique_ptr<Expression> initializer;

        std::string to_string() final {
            return fmt::format("VariableDefinition: .name = {{ {} }}, .initializer {{ {} }}", name, initializer ? initializer->to_string() : "nullptr");
        }
    };

    struct ExpressionStatement : public StatementAcceptor<ExpressionStatement> {
        std::string to_string() final {
            return "ExpressionStatement";
        }
    };

} // namespace Statements


namespace Expressions {
    struct Assignment;
    struct BinaryOperator;
    struct Number;

    struct ExpressionVisitor {
        virtual void visit(Assignment& expression) = 0;
        virtual void visit(BinaryOperator& expression) = 0;
        virtual void visit(Number& expression) = 0;
    };

    template<typename T>
    struct ExpressionAcceptor : public Expression {
        void accept(ExpressionVisitor& visitor) final {
            visitor.visit(static_cast<T&>(*this));
        }
    };

    struct Assignment : public ExpressionAcceptor<Assignment> {
    };

    struct BinaryOperator : public ExpressionAcceptor<BinaryOperator> {
        BinaryOperator(std::unique_ptr<Expression> lhs, Token operator_type, std::unique_ptr<Expression> rhs) 
            : lhs(std::move(lhs))
            , operator_type(operator_type)
            , rhs(std::move(rhs))
        {}

        std::string to_string() final {
            return fmt::format("BinaryOperator: .lhs {{ {} }}, .operator {{ {} }}, .rhs {{ {} }}", lhs->to_string(), operator_type, rhs->to_string());
        }

        std::unique_ptr<Expression> lhs;
        Token operator_type;
        std::unique_ptr<Expression> rhs;
    };
    struct Number : public ExpressionAcceptor<Number> {
        Number(std::string_view sv)
        {
            auto result = std::from_chars(sv.data(), sv.data() + sv.size(), value);
            if (result.ec == std::errc::invalid_argument) {
                assert(false && "TODO NUMBER");
            }
        }

        int value;

        std::string to_string() final {
            return fmt::format("NumberExpression: .value {{ {} }}", value);
        }
    };

} // namespace Expressions

class Parser {
public:
    using TokenList = std::vector<Token>;
    using StatementList = std::vector<std::unique_ptr<Statements::Statement>>;

    using Statement = Statements::Statement;
    using UniqStatement = std::unique_ptr<Statement>;

    using Expression = Expressions::Expression;
    using UniqExpression = std::unique_ptr<Expression>;

public:
    Parser() = default;

    StatementList parse(const TokenList& tokens);

    /// --- Helper functions --- ///
    [[nodiscard]] auto isAtEnd() -> bool;
    [[nodiscard]] auto peek() -> Token;
    [[nodiscard]] auto consume(const TokenType& ttype, std::string_view msg) -> Result<Token>;

    template <typename ...Tokens>
    [[nodiscard]] auto checkAndAdvance(Tokens&& ...tokens) -> bool const;
    [[nodiscard]] auto check(const TokenType& ttype) -> bool const;

    auto advance() -> Token;
    [[nodiscard]] auto previous() -> Token;

    [[nodiscard]] auto errorStmt(const Token& token, std::string_view msg) -> Result<UniqStatement>;
    [[nodiscard]] auto errorExpr(const Token& token, std::string_view msg) -> Result<UniqExpression>;

    /// --- Parsing functions --- ///
    [[nodiscard]] auto declaration() -> Result<UniqStatement>;
    [[nodiscard]] auto varDeclaration() -> Result<UniqStatement>;
    [[nodiscard]] auto expression() -> Result<UniqExpression>;
    [[nodiscard]] auto assignment() -> Result<UniqExpression>;

    [[nodiscard]] auto term() -> Result<UniqExpression>;
    [[nodiscard]] auto factor() -> Result<UniqExpression>;
    [[nodiscard]] auto primary() -> Result<UniqExpression>;

private:
    TokenList m_tokens;
    std::size_t m_current { 0 };
    bool m_hasError { false };
};
