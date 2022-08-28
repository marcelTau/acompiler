#pragma once

#include "parser.h"
#include <fstream>
#include <sstream>
#include <bitset>

namespace Emitter {

using namespace Statements;
using namespace Expressions;

using Expressions::Expression;
using Statements::Statement;
using StatementList = std::vector<std::unique_ptr<Statement>>;

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

    Emitter() = default;

    void emit(const StatementList& statements) {
        emit_line("", "=== Auto-generated code ===");

        emit_line("%include \"asm/defines.inc\"", "bring syscall defines into scope");
        emit_line("section .text", "begin source code segment");
        emit_line("global _start", "make entrypoint function visible to linker");

        emit_line("_start:", "entrypoint function");
        emit_line("  push rbp", "store old rbp on stack");
        emit_line("  mov rbp, rsp", "store stack pointer in rbp so it can be used as the base pointer for the functions stack frame\n");

        emit_line("  call main", "user defined main function\n");

        emit_line("  mov rdi, rax", "set exit code to what main put in rax");
        emit_line("  mov rax, SYS_EXIT", "prepare to exit with code 0");
        emit_line("  syscall", "call exit");

        for (const auto &statement : statements) {
            emit_statement(statement);
        }

        std::ofstream file("testoutput.asm");
        file << output.str();
        file.close();

        fmt::print("Emitter done\n");
    }

private:
    void emit_line(std::string_view line, std::string_view comment = "") {
        output << fmt::format("{:30}; {}\n", line, comment);
    }

    void emit_statement(const std::unique_ptr<Statement>& statement) {
        statement->accept(*this);
    }

    void visit(VariableDefinition& statement) override {}
    void visit(ExpressionStatement& statement) override {}
    void visit(Print& statement)override {}

    void visit(Return& statement) override {
        statement.value->accept(*this);
        emit_line("  pop rax", "pop result of binary expression into rax");
    }

    void visit(Function& statement) override {
        // @todo pretty print the params of the function as comment above
        emit_line(fmt::format("\n{}:", statement.name.lexeme), "User defined function");
        emit_line("  push rbp", "save rbp since it is callee saved");
        emit_line("  mov rbp, rsp", "setup rbp to use it as the base pointer");
        emit_line(fmt::format("  sub rsp, {}", statement.stack_size), fmt::format("reserve {} bytes on the stack", statement.stack_size));

        for (const auto& s : statement.body) {
            s->accept(*this);
        }

        emit_line("  mov rsp, rbp", "restore rsp (cleanup stack)");
        emit_line("  pop rbp", "get rbp back, since it is callee saved");
        emit_line("  ret");
    }

    void visit(Assignment& expression) override {}
    void visit(BinaryOperator& expression) override {
        std::size_t firstIdx, secondIdx;
        std::string_view op;

        switch (expression.operator_type.type) {
            case TokenType::Plus: 
                op = "add";
                break;
            case TokenType::Minus: 
                op = "sub";
                break;
            default:
                fmt::print(stderr, "Emitter: {}", expression.operator_type);
                assert(false && "Emitter: BinaryOperator");
        };

        firstIdx = getNextFreeRegister();
        fmt::print(stderr, "Moving lhs into register {}", registerNames[firstIdx]);
        expression.lhs->accept(*this);

        secondIdx = getNextFreeRegister();
        fmt::print(stderr, "Moving rhs into register {}", registerNames[secondIdx]);
        expression.rhs->accept(*this);

        // do the binary operation
        emit_line(fmt::format("  {} {}, {}", op, registerNames[firstIdx], registerNames[secondIdx]));

        // push outcome of binary expression onto stack
        emit_line(fmt::format("  push {}", registerNames[firstIdx]));
    }

    // loads the value into the next free register r8-r15
    void visit(Number& expression) override {
        auto idx = getNextFreeRegister();
        emit_line(fmt::format("  mov {}, {}", registerNames[idx], expression.value));
        m_registers.set(idx);
    }

    void visit(Bool& expression) override {}
    void visit(Variable& expressi) override {}
    void visit(Unary& expression) override {}

private:
    std::size_t getNextFreeRegister() {
        for (std::size_t idx = Register::R8; idx <= Register::R15; ++idx) {
            if (!m_registers.test(idx)) {
                return idx;
            }
        }

        assert(false && "getNextFreeRegister");
    }

    std::bitset<Register::MAX_COUNT> m_registers {};
    std::stringstream output {};
};

} // namespace Emitter
