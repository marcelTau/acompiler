#pragma once

#include "environment.h"
#include "parser.h"
#include <fstream>
#include <bitset>

namespace Emitter {

enum Register {
    RAX,
    RBX,
    RCX,
    RDX,
    RSP,
    RBP,
    RSI,
    RDI,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
    MAX_COUNT,
};

static constexpr std::array registerNames {
    "rax",
    "rbx",
    "rcx",
    "rdx",
    "rsp",
    "rbp",
    "rsi",
    "rdi",
    "r8",
    "r9",
    "r10",
    "r11",
    "r12",
    "r13",
    "r14",
    "r15",
    "MAX_COUNT",
};

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

    // ------------------------------------------------------------------------
    // Expressions
    // ------------------------------------------------------------------------
    void visit(Expressions::Assignment&         expression) override;
    void visit(Expressions::BinaryOperator&     expression) override;
    void visit(Expressions::Number&             expression) override;
    void visit(Expressions::Bool&               expression) override;
    void visit(Expressions::Variable&           expression) override;
    void visit(Expressions::Unary&              expression) override;

    // ------------------------------------------------------------------------
    // Helper Functions
    // ------------------------------------------------------------------------
    [[nodiscard]] ValueVariant lookup_variable(const Expressions::Variable& var);
    [[nodiscard]] ValueVariant lookup_variable(const Expressions::Assignment& var);
    void emit_line(std::string_view line, std::string_view comment = "");
    void emit_statement(const std::unique_ptr<Statements::Statement>& statement);

private:
    [[nodiscard]] std::size_t getNextFreeRegister();

    std::string filepath;
    std::bitset<Register::MAX_COUNT> m_registers {};
    std::stringstream output {};
    Environment<ValueVariant> environment;
    Environment<ValueVariant> globals;
    int current_function_stack_offset { 0 };
};

} // namespace Emitter
