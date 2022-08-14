#pragma once

#include <vector>
#include <memory>
#include "token.h"

namespace Statements {
    struct VariableDefinition;
    struct ExpressionStatement;

    struct StatementVisitor {
        virtual void visit(VariableDefinition& statement) = 0;
        virtual void visit(ExpressionStatement& statement) = 0;

        virtual ~StatementVisitor() = default;
    };

    struct Statement {
        virtual ~Statement() = default;
        virtual void accept(StatementVisitor& visitor) = 0;
    };

    template<typename T>
    struct StatementAcceptor : public Statement {
        void accept(StatementVisitor& visitor) final {
            visitor.visit(static_cast<T&>(*this));
        }
    };

    struct VariableDefinition : public StatementAcceptor<VariableDefinition> {
    };
    struct ExpressionStatement : public StatementAcceptor<ExpressionStatement> {
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

    struct Expression {
        virtual ~Expression() = default;
        virtual void accept(ExpressionVisitor& visitor) = 0;
        
        // @todo datatype
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
    };
    struct Number : public ExpressionAcceptor<Number> {
    };

} // namespace Expressions

class Parser {
public:
    using TokenList = std::vector<Token>;
    using StatementList = std::vector<std::unique_ptr<Statements::Statement>>;

public:
    Parser() = default;

    void parse(const TokenList& tokens);

private:
    TokenList m_tokens;
    std::size_t m_current { 0 };
};
