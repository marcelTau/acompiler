#pragma once

#include <utility>
#include <vector>
#include <memory>
#include <optional>
#include <charconv>
#include <cassert>
#include <sstream>
#include "fmt/format.h"
#include "fmt/ranges.h"
#include "token.h"
#include "error.h"
#include "datatype.h"

/// @todo ugly hacky stuff, try to fix this pls
namespace Expressions {
    struct ExpressionVisitor;
    struct Expression {
        virtual ~Expression() = default;
        virtual void accept(ExpressionVisitor& visitor) = 0;
        virtual std::string to_string() = 0;

        DataType datatype {};
        // @todo datatype
    };
} // namespace Expression


namespace Statements {
    using Expressions::Expression;

    struct VariableDefinition;
    struct ExpressionStatement;
    struct Print;
    struct Function;
    struct Return;
    //struct Assignment;

    struct StatementVisitor {
        virtual void visit(VariableDefinition& statement) = 0;
        virtual void visit(ExpressionStatement& statement) = 0;
        virtual void visit(Print& statement) = 0;
        virtual void visit(Function& statement) = 0;
        virtual void visit(Return& statement) = 0;
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
        VariableDefinition(std::string_view name, std::unique_ptr<Expression> initializer, DataType data_type = {})
            : name(name)
            , initializer(std::move(initializer))
            , data_type(std::move(data_type))
        {
        }


        std::string to_string() final {
            return fmt::format("VariableDefinition: .name {{ {} }}, .initializer {{ {} }}, .dt {{ {} }}", 
                               name,
                               initializer ? initializer->to_string() : "nullptr",
                               data_type.to_string());
        }

        std::string name;
        std::unique_ptr<Expression> initializer;
        DataType data_type;
    };

    struct ExpressionStatement : public StatementAcceptor<ExpressionStatement> {
        std::string to_string() final {
            return "ExpressionStatement";
        }
    };

    struct Print : public StatementAcceptor<Print> {
        Print(std::unique_ptr<Expression> expr)
            : expression(std::move(expr))
        {
        }

        std::string to_string() final {
            return fmt::format("PrintStatement: .expression {{ {} }}", expression->to_string());
        }

        std::unique_ptr<Expression> expression;
    };

    struct Function : public StatementAcceptor<Function> {
        Function(Token name, std::vector<Token> params, std::vector<std::unique_ptr<Statement>> body)
            : name(name)
            , params(std::move(params))
            , body(std::move(body))
        {
            //stack_size = params.size() * 4; // @todo for now assume that every datatype use 4 bytes and don't care about local variables
        }

        std::string to_string() final {
            std::stringstream ss{};

            for (const auto& statement : body) {
                ss << " { " << statement->to_string() << " }, ";
            }

            return fmt::format(
                "FunctionStatement: .name {{ {} }}, .params {{ {} }}, .body {{ {} }}",
                name, fmt::join(params, ", "), ss.str());
        }

        Token name;
        std::vector<Token> params;
        std::vector<std::unique_ptr<Statement>> body;
        std::size_t stack_size { 0 };
    };

    struct Return : public StatementAcceptor<Return> {
        Return(std::unique_ptr<Expression> value)
            : value(std::move(value))
        {
        }

        std::string to_string() final {
            return fmt::format("Return: .value {{ {} }}", value ? value->to_string() : "nullptr");
        }

        std::unique_ptr<Expression> value;
    };

} // namespace Statements


namespace Expressions {
    struct Assignment;
    struct BinaryOperator;
    struct Number;
    struct Bool;
    struct Variable;
    struct Unary;

    struct ExpressionVisitor {
        virtual void visit(Assignment& expression) = 0;
        virtual void visit(BinaryOperator& expression) = 0;
        virtual void visit(Number& expression) = 0;
        virtual void visit(Bool& expression) = 0;
        virtual void visit(Variable& expression) = 0;
        virtual void visit(Unary& expression) = 0;
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
        {
        }

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
            datatype = availableDataTypes.at("Int");
        }

        std::string to_string() final {
            return fmt::format("NumberExpression: .value {{ {} }}", value);
        }

        int value;
    };

    struct Bool : public ExpressionAcceptor<Bool> {
        Bool(std::string_view sv)
        {
            value = (sv == "true");
            datatype = availableDataTypes.at("Bool");
        }

        std::string to_string() final {
            return fmt::format("BoolExpression: .value {{ {} }}", value ? "true" : "false");
        }

        bool value;
    };

    struct Variable : public ExpressionAcceptor<Variable> {
        Variable(std::string_view name)
            : name(name) {}

        std::string to_string() final {
            return fmt::format("VariableExpression: .name {{ {} }}", name);
        }
        std::string name;
    };

    struct Unary : public ExpressionAcceptor<Unary> {
        Unary(Token operator_type, std::unique_ptr<Expression> rhs) 
            : operator_type(operator_type)
            , rhs(std::move(rhs))
        {
        }

        std::string to_string() final {
            return fmt::format("UnaryExpression: .operator {{ {} }}, .rhs {{ {} }}", operator_type, rhs->to_string());
        }

        Token operator_type;
        std::unique_ptr<Expression> rhs;
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
    [[nodiscard]] auto statement() -> Result<UniqStatement>;
    [[nodiscard]] auto returnStatement() -> Result<UniqStatement>;
    [[nodiscard]] auto expressionStatement() -> Result<UniqStatement>;
    [[nodiscard]] auto printStatement() -> Result<UniqStatement>;
    [[nodiscard]] auto function() -> Result<UniqStatement>;
    [[nodiscard]] auto block() -> Result<std::vector<UniqStatement>>;


    [[nodiscard]] auto expression() -> Result<UniqExpression>;
    [[nodiscard]] auto assignment() -> Result<UniqExpression>;

    [[nodiscard]] auto term() -> Result<UniqExpression>;
    [[nodiscard]] auto factor() -> Result<UniqExpression>;
    [[nodiscard]] auto unary() -> Result<UniqExpression>;
    [[nodiscard]] auto primary() -> Result<UniqExpression>;

private:
    TokenList m_tokens;
    std::size_t m_current { 0 };
    bool m_hasError { false };
};
