#pragma once

#include "parser.h"

struct Resolver
    : public Expressions::ExpressionVisitor, 
             Statements::StatementVisitor 
{
private:
    using VariableName = std::string_view;
    using StatementList = std::vector<std::unique_ptr<Statements::Statement>>;

    enum class FunctionType {
        None,
        Function,
    };

    enum class State {
        Declared,
        Defined,
        Undefined,
    };

public:
    Resolver();
    //Resolver() = default;

    // ====================================================================
    // Resolves
    // ====================================================================
    void resolve(const StatementList& statements);
    void resolve(const std::unique_ptr<Expressions::Expression>& expression);
    void resolve(Statements::Function& function);
    void resolve(const std::unique_ptr<Statements::Statement>& statement);
    void resolve(Expressions::Variable& value, const Token& name);
    void resolve(Expressions::Assignment& value, const Token& name);
    void resolve(Expressions::FunctionCall& value, const Token& name);

    // ====================================================================
    // Statements
    // ====================================================================
    void visit(Statements::VariableDefinition&  statement) override;
    void visit(Statements::ExpressionStatement& statement) override;
    void visit(Statements::Print&               statement) override;
    void visit(Statements::Function&            statement) override;
    void visit(Statements::Return&              statement) override;
    void visit(Statements::IfStatement&         statement) override;
    void visit(Statements::Block&               statement) override;

    // ====================================================================
    // Expressions
    // ====================================================================
    void visit(Expressions::Assignment&         expression) override;
    void visit(Expressions::Number&             expression) override;
    void visit(Expressions::Bool&               expression) override;
    void visit(Expressions::BinaryOperator&     expression) override;
    void visit(Expressions::Variable&           expression) override;
    void visit(Expressions::Unary&              expression) override;
    void visit(Expressions::Logical&            expression) override;
    void visit(Expressions::FunctionCall&       expression) override;

    // ====================================================================
    // Helper functions
    // ====================================================================
    void error(const Token& name, std::string_view message);
    void declare(const Token& name);
    void define(const Token& name);
    void begin_scope();
    void end_scope();

private:
    std::vector<std::unordered_map<VariableName, State>> scopes;
    FunctionType current_function { FunctionType::None };
};
