#pragma once

#include "environment.h"
#include "parser.h"
#include <fstream>
#include <bitset>

struct Emitter : public Expressions::ExpressionVisitor, Statements::StatementVisitor {
    using StatementList = std::vector<std::unique_ptr<Statements::Statement>>;
    Emitter(std::string filepath);
    void emit(const StatementList& statements);

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

    static constexpr std::array registerNames64 {
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

    static constexpr std::array registerNames8 {
        "al",
        "bl",
        "cl",
        "dl",
        "spl",
        "bpl",
        "sil",
        "dil",
        "r8b",
        "r9b",
        "r10b",
        "r11b",
        "r12b",
        "r13b",
        "r14b",
        "r15b",
        "MAX_COUNT",
    };

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
