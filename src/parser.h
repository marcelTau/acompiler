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

template <typename Value>
struct Environment;


/// @todo ugly hacky stuff, try to fix this pls
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


namespace Statements {
    using Expressions::Expression;

    struct VariableDefinition;
    struct ExpressionStatement;
    struct Print;
    struct Function;
    struct Return;

    struct StatementVisitor {
        virtual void visit(VariableDefinition& statement) = 0;
        virtual void visit(ExpressionStatement& statement) = 0;
        virtual void visit(Print& statement) = 0;
        virtual void visit(Function& statement) = 0;
        virtual void visit(Return& statement) = 0;
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
        VariableDefinition() = default;
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
            //this->initializer.swap(other.initializer);
            //if (auto *p = dynamic_cast<
            //this->initializer = std::make_unique(other.initializer.get());
        }
        

        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final {
            return fmt::format(
                    "{0:>{w}}VariableDefinition:\n"
                    "{0:>{w}}  .name = {2}\n"
                    "{0:>{w}}  .initializer =\n{3}\n"
                    "{0:>{w}}  .datatype = {4}\n",
                    "",  // dummy argument for padding
                    fmt::arg("w", offset),
                    name,
                    initializer ? initializer->to_string(offset + 4) : "nullptr",
                    datatype.to_string()
            );
        }

        Token name;
        std::unique_ptr<Expression> initializer;
        DataType datatype;
        std::size_t offset { 8 };
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
            return fmt::format("PrintStatement: .expression {{ {} }}", expression->to_string());
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
                    "{0:>{w}}  .body = \n{5}\n",
                    "",  // dummy argument for padding
                    fmt::arg("w", offset),
                    return_datatype.to_string(),
                    name,
                    fmt::join(params, ", "),
                    ss.str()
            );
        }

        Token name;
        std::vector<Token> params;
        std::vector<std::unique_ptr<Statement>> body;
        std::size_t stack_size { 0 };
        DataType return_datatype;
        //std::shared_ptr<Environment> environment;
    };

    struct Return : public StatementAcceptor<Return> {
        Return(std::unique_ptr<Expression> value)
            : value(std::move(value))
        {
        }

        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final {
            //return fmt::format("Return: .value {{ {} }}", value ? value->to_string() : "nullptr");
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
        Assignment(Token name, std::unique_ptr<Expression> value)
            : name(name)
            , value(std::move(value))
        {
        }

        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final {
            return fmt::format(
                    "{0:>{w}}Assignment:\n"
                    "{0:>{w}}  .name = {2}\n"
                    "{0:>{w}}  .value =\n{3}\n"
                    "{0:>{w}}  .scope_distance = {4}\n",
                    "",  // dummy argument for padding
                    fmt::arg("w", offset),
                    name,
                    value ? value->to_string(offset + 4) : "nullptr",
                    scope_distance
            );
        }

        Token name;
        std::unique_ptr<Expression> value;
        int scope_distance { -1 };
    };

    struct BinaryOperator : public ExpressionAcceptor<BinaryOperator> {
        BinaryOperator(std::unique_ptr<Expression> lhs, Token operator_type, std::unique_ptr<Expression> rhs) 
            : lhs{std::move(lhs)}
            , operator_type(operator_type)
            , rhs{std::move(rhs)}
        {
            // @todo the datatype should be set in the typechecker and not in the parser,
            // since the parser does not know how to resolve local variables to
            // their actual datatype
        }

        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final {
            return fmt::format(
                    "{0:>{w}}BinaryOperator:\n"
                    "{0:>{w}}  .datatype = {2}\n"
                    "{0:>{w}}  .lhs =\n{3}\n"
                    "{0:>{w}}  .operator = {4}\n"
                    "{0:>{w}}  .rhs =\n{5}\n",
                    "",  // dummy argument for padding
                    fmt::arg("w", offset),
                    datatype.to_string(),
                    lhs->to_string(offset + 4), 
                    operator_type, 
                    rhs->to_string(offset + 4)
            );
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
            this->datatype = availableDataTypes.at("Int");
        }

        Number(Number&& other) noexcept {
            this->datatype = other.datatype;
        }

        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final {
            //return fmt::format("NumberExpression({}): .value {{ {} }}", datatype.to_string(), value);
            return fmt::format(
                    "{0:>{w}}NumberExpression:\n"
                    "{0:>{w}}  .datatype = {2}\n"
                    "{0:>{w}}  .value = {3}",
                    "",  // dummy argument for padding
                    fmt::arg("w", offset),
                    datatype.to_string(),
                    value
            );
        }

        int value;
    };

    struct Bool : public ExpressionAcceptor<Bool> {
        Bool(std::string_view sv)
        {
            value = (sv == "true");
            datatype = availableDataTypes.at("Bool");
        }

        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final {
            //return fmt::format("BoolExpression({}): .value {{ {} }}", datatype.to_string(), value ? "true" : "false");
            return fmt::format(
                    "{0:>{w}}BoolExpression:\n"
                    "{0:>{w}}  .datatype = {2}\n"
                    "{0:>{w}}  .value = {3}\n",
                    "",  // dummy argument for padding
                    fmt::arg("w", offset),
                    datatype.to_string(),
                    value ? "true" : "false"
            );
        }

        bool value;
    };

    struct Variable : public ExpressionAcceptor<Variable> {
        Variable(const Token& name)
            : name(name)
        {
            // @todo add datatype of variable here.
            // This will probably change when scopes are implemented and we can lookup the variable and get it's type
        }

        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final {
            //return fmt::format("VariableExpression({}): .name {{ {} }}", datatype.to_string(), name);
            return fmt::format(
                    "{0:>{w}}Variable:\n"
                    "{0:>{w}}  .name = {2}\n"
                    "{0:>{w}}  .datatype = {3}\n"
                    "{0:>{w}}  .scope_distance = {4}\n",
                    "",  // dummy argument for padding
                    fmt::arg("w", offset),
                    name,
                    datatype.to_string(),
                    scope_distance
            );
        }
        Token name;
        int scope_distance { -1 };
    };

    struct Unary : public ExpressionAcceptor<Unary> {
        Unary(Token operator_type, std::unique_ptr<Expression> rhs) 
            : operator_type(operator_type)
            , rhs(std::move(rhs))
        {
            datatype = this->rhs->datatype;
        }

        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final {
            //return fmt::format("UnaryExpression({}): .operator {{ {} }}, .rhs {{ {} }}", datatype.to_string(), operator_type, rhs->to_string());
            return fmt::format(
                    "{0:>{w}}UnaryExpression:\n"
                    "{0:>{w}}  .operator = {2}\n"
                    "{0:>{w}}  .rhs = {3}\n",
                    "",  // dummy argument for padding
                    fmt::arg("w", offset),
                    datatype.to_string(),
                    rhs->to_string(offset + 4)
            );
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
