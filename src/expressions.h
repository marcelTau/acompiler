#pragma once

#include <memory>
#include "datatype.h"
#include "token.h"

namespace Expressions {
    struct Assignment;
    struct BinaryOperator;
    struct Number;
    struct Bool;
    struct Variable;
    struct Unary;
    struct Logical;
    struct FunctionCall;


    // ============================================================================
    // Declare the visitor
    // ============================================================================
    struct ExpressionVisitor {
        virtual void visit(Assignment& expression) = 0;
        virtual void visit(BinaryOperator& expression) = 0;
        virtual void visit(Number& expression) = 0;
        virtual void visit(Bool& expression) = 0;
        virtual void visit(Variable& expression) = 0;
        virtual void visit(Unary& expression) = 0;
        virtual void visit(Logical& expression) = 0;
        virtual void visit(FunctionCall& expression) = 0;
    };

    // ============================================================================
    // Base class of all Expressions
    // ============================================================================
    struct Expression {
        Expression() = default;
        virtual ~Expression() = default;
        virtual void accept(ExpressionVisitor& visitor) = 0;
        [[nodiscard]] virtual std::string to_string(std::size_t offset = 0) const = 0;

        DataType datatype {};
    };

    // ============================================================================
    // Shortcut to implement the accept method for all expression types at once
    // ============================================================================
    template<typename T>
    struct ExpressionAcceptor : public Expression {
        void accept(ExpressionVisitor& visitor) final {
            visitor.visit(static_cast<T&>(*this));
        }
    };

    struct Assignment : public ExpressionAcceptor<Assignment> {
        Assignment(Token name, std::unique_ptr<Expression> value);
        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final;

        Token name;
        std::unique_ptr<Expression> value;
        int scope_distance { -1 };
    };

    struct BinaryOperator : public ExpressionAcceptor<BinaryOperator> {
        BinaryOperator(std::unique_ptr<Expression> lhs, Token operator_type, std::unique_ptr<Expression> rhs);
        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final;

        std::unique_ptr<Expression> lhs;
        Token operator_type;
        std::unique_ptr<Expression> rhs;
    };

    struct Number : public ExpressionAcceptor<Number> {
        Number(std::string_view sv);
        Number(Number&& other) noexcept;
        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final;

        int value;
    };

    struct Bool : public ExpressionAcceptor<Bool> {
        Bool(std::string_view sv);
        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final;

        bool value;
    };

    struct Variable : public ExpressionAcceptor<Variable> {
        Variable(const Token& name);
        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final;

        Token name;
        int scope_distance { -1 };
    };

    struct Unary : public ExpressionAcceptor<Unary> {
        Unary(Token operator_type, std::unique_ptr<Expression> rhs);
        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final;

        Token operator_type;
        std::unique_ptr<Expression> rhs;
    };

    struct Logical : public ExpressionAcceptor<Logical> {
        Logical(std::unique_ptr<Expression> lhs, Token operator_type, std::unique_ptr<Expression> rhs);
        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final;

        std::unique_ptr<Expression> lhs;
        Token operator_type;
        std::unique_ptr<Expression> rhs;
    };

    struct FunctionCall : public ExpressionAcceptor<FunctionCall> {
        FunctionCall(const Token& name, std::unique_ptr<Expression> callee, std::vector<std::unique_ptr<Expression>> params);
        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final;

        Token name;
        std::unique_ptr<Expression> callee;
        std::vector<std::unique_ptr<Expression>> params;
        int scope_distance { -1 };
    };
    
} // namespace Expressions


