#pragma once

#include <memory>

#include "datatype.h"
#include "token.h"

namespace Expressions {
    struct Expression;
}

namespace Statements {
    struct VariableDefinition;
    struct ExpressionStatement;
    struct Print;
    struct Function;
    struct Return;
    struct IfStatement;
    struct Block;

    // ============================================================================
    // Declare the visitor
    // ============================================================================
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

    // ============================================================================
    // Base class of all Statements
    // ============================================================================
    struct Statement {
        virtual ~Statement() = default;
        virtual void accept(StatementVisitor& visitor) = 0;
        [[nodiscard]] virtual std::string to_string(std::size_t offset = 0) const = 0;
    };

    // ============================================================================
    // Shortcut to implement the accept method for all expression types at once
    // ============================================================================
    template<typename T>
    struct StatementAcceptor : public Statement {
        void accept(StatementVisitor& visitor) final {
            visitor.visit(static_cast<T&>(*this));
        }
    };

    struct VariableDefinition : public StatementAcceptor<VariableDefinition> {
        VariableDefinition(const Token& name, std::unique_ptr<Expressions::Expression> initializer, DataType data_type = {});
        VariableDefinition(VariableDefinition& other);
        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final;

        Token name;
        std::unique_ptr<Expressions::Expression> initializer;
        DataType datatype;
        int offset { -1 };
        int scope_distance { -1 };
    };

    struct ExpressionStatement : public StatementAcceptor<ExpressionStatement> {
        ExpressionStatement(std::unique_ptr<Expressions::Expression> expr);
        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final;

        std::unique_ptr<Expressions::Expression> expression;
    };

    struct Print : public StatementAcceptor<Print> {
        Print(std::unique_ptr<Expressions::Expression> expr);
        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final;

        std::unique_ptr<Expressions::Expression> expression;
    };

    struct Function : public StatementAcceptor<Function> {
        Function(Token name, std::vector<Token> params, std::vector<std::unique_ptr<Statement>> body, DataType return_datatype);
        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final;

        Token name;
        std::vector<Token> params;
        std::vector<std::unique_ptr<Statement>> body;
        std::size_t stack_size { 0 };
        DataType return_datatype;
        int scope_distance { -1 };
    };

    struct Return : public StatementAcceptor<Return> {
        Return(std::unique_ptr<Expressions::Expression> value);
        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final;

        std::unique_ptr<Expressions::Expression> value;
    };

    struct Block : public StatementAcceptor<Block> {
        Block(std::vector<std::unique_ptr<Statement>> statements);
        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final;

        std::vector<std::unique_ptr<Statement>> statements;
    };

    struct IfStatement : public StatementAcceptor<IfStatement> {
        IfStatement(std::unique_ptr<Expressions::Expression> condition, std::unique_ptr<Block> then_branch, std::unique_ptr<Block> else_branch);
        [[nodiscard]] std::string to_string(std::size_t offset = 0) const final;

        std::unique_ptr<Expressions::Expression> condition;
        std::unique_ptr<Block> then_branch;
        std::unique_ptr<Block> else_branch;
    };
}; // namespace Statements
