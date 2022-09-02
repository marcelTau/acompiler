#pragma once

#include "environment.h"
#include "parser.h"
#include <fstream>
#include <bitset>

struct Emitter : public Expressions::ExpressionVisitor, Statements::StatementVisitor {
    using StatementList = std::vector<std::unique_ptr<Statements::Statement>>;
    Emitter(std::string filepath);
    void emit(const StatementList& statements);

private:
    // ------------------------------------------------------------------------
    // Statements
    // ------------------------------------------------------------------------
    void visit(Statements::VariableDefinition&  statement) override;
    void visit(Statements::ExpressionStatement& statement) override;
    void visit(Statements::Print&               statement) override;
    void visit(Statements::Return&              statement) override;
    void visit(Statements::Function&            statement) override;
    void visit(Statements::IfStatement&         statement) override;
    void visit(Statements::Block&               statement) override;

    // ------------------------------------------------------------------------
    // Expressions
    // ------------------------------------------------------------------------
    void visit(Expressions::Assignment&         expression) override;
    void visit(Expressions::BinaryOperator&     expression) override;
    void visit(Expressions::Number&             expression) override;
    void visit(Expressions::Bool&               expression) override;
    void visit(Expressions::Variable&           expression) override;
    void visit(Expressions::Unary&              expression) override;
    void visit(Expressions::Logical&            expression) override;
    void visit(Expressions::FunctionCall&       expression) override;

    // ------------------------------------------------------------------------
    // Helper Functions
    // ------------------------------------------------------------------------
    [[nodiscard]] ValueVariant lookup_variable(const Expressions::Variable& var);
    [[nodiscard]] ValueVariant lookup_variable(const Expressions::Assignment& var);
    [[nodiscard]] std::string getLabelName();
    void emit_line(std::string_view line, std::string_view comment = "");
    void emit_statement(const std::unique_ptr<Statements::Statement>& statement);


private:
    std::string filepath;
    std::stringstream output {};
    Environment<ValueVariant> environment;
    Environment<ValueVariant> globals;
    int current_function_stack_offset { 0 };
    int label_counter { 0 };
};
