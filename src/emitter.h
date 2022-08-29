#pragma once

#include "parser.h"
#include <fstream>
#include <spdlog/spdlog.h>
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

    Emitter(const std::string& filepath)
        : filepath(filepath)
    {
    }

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

        std::ofstream file(filepath);
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
        emit_line("");
        emit_line(fmt::format("{}:", statement.name.lexeme), "User defined function");
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
        // recursivly push lhs on stack
        expression.lhs->accept(*this);

        // recursivly push rhs on stack
        expression.rhs->accept(*this);

        switch (expression.operator_type.type) {
            case TokenType::Plus: {
                auto idx1 = getNextFreeRegister();
                emit_line(fmt::format("  pop {}", registerNames[idx1]), "take value from stack into first free register");

                auto idx2 = getNextFreeRegister();
                emit_line(fmt::format("  pop {}", registerNames[idx2]), "take second value from stack");

                // do the addition and push result on stack
                emit_line(fmt::format("  add {}, {}", registerNames[idx1], registerNames[idx2]));
                emit_line(fmt::format("  push {}", registerNames[idx1]));

                // clear the used registers
                m_registers.flip(idx1);
                m_registers.flip(idx2);
                break;
            }
            case TokenType::Minus: {
                auto idx1 = getNextFreeRegister();
                emit_line(fmt::format("  pop {}", registerNames[idx1]), "take value from stack into first free register");

                auto idx2 = getNextFreeRegister();
                emit_line(fmt::format("  pop {}", registerNames[idx2]), "take second value from stack");

                // do the subtraction and push result on stack
                emit_line(fmt::format("  sub {}, {}", registerNames[idx2], registerNames[idx1]));
                emit_line(fmt::format("  push {}", registerNames[idx2]));

                // clear the used registers
                m_registers.flip(idx1);
                m_registers.flip(idx2);
                break;
            }
            case TokenType::Star: {
                auto idx1 = getNextFreeRegister();
                emit_line(fmt::format("  pop {}", registerNames[idx1]), "take value from stack into first free register");

                emit_line(fmt::format("  pop rax"), "take second value from stack");

                emit_line(fmt::format("  mul {}", registerNames[idx1]));

                // @todo if i need to keep rax consistent, then push it in the beginning and pop it at the end, therefore mov <free_register>, rax before
                emit_line(fmt::format("  push rax"));

                // clear the used registers
                m_registers.flip(idx1);

                break;
            }
            case TokenType::Slash: {

                emit_line("  xor rdx, rdx", "set rdx to 0 so it does not mess with the division");
                auto idx1 = getNextFreeRegister();
                emit_line(fmt::format("  pop {}", registerNames[idx1]), "take value from stack into first free register");
                emit_line(fmt::format("  pop rax"), "take second value from stack");


                emit_line(fmt::format("  div {}", registerNames[idx1]));

                // @todo if i need to keep rax consistent, then push it in the beginning and pop it at the end, therefore mov <free_register>, rax before
                emit_line(fmt::format("  push rax"));

                // clear the used registers
                m_registers.flip(idx1);

                break;
            }
            default:
                fmt::print(stderr, "Emitter: {}", expression.operator_type);
                assert(false && "Emitter: BinaryOperator");
        };

        //if (op == "mul") {
            //firstIdx = getNextFreeRegister();
            //spdlog::info("Moving lhs into register {}", registerNames[firstIdx]);
            //expression.lhs->accept(*this);

            //emit_line(fmt::format("  mov rax, {}", registerNames[firstIdx]), "mul multiplies the value in register by rax");

            //secondIdx = getNextFreeRegister();
            //spdlog::info("Moving rhs into register {}", registerNames[secondIdx]);
            //expression.rhs->accept(*this);

            //emit_line(fmt::format("  {} {}", op, registerNames[secondIdx]));
            //emit_line(fmt::format("  push rax"), "result is already in rax");
        //} else {
            //firstIdx = getNextFreeRegister();
            //spdlog::info("Moving lhs into register {}", registerNames[firstIdx]);
            //expression.lhs->accept(*this);

            //secondIdx = getNextFreeRegister();
            //spdlog::info("Moving rhs into register {}", registerNames[secondIdx]);
            //expression.rhs->accept(*this);

            //emit_line(fmt::format("  {} {}, {}", op, registerNames[firstIdx], registerNames[secondIdx]));
            //emit_line(fmt::format("  push {}", registerNames[firstIdx]));
        //}
    }

    // loads the value into the next free register r8-r15
    void visit(Number& expression) override {
        //auto idx = getNextFreeRegister();
        //emit_line(fmt::format("  mov {}, {}", registerNames[idx], expression.value));
        
        emit_line(fmt::format("  push {}", expression.value), "push value on stack");

        //m_registers.set(idx);
    }

    void visit(Bool& expression) override {}
    void visit(Variable& expressi) override {}
    void visit(Unary& expression) override {}

private:
    std::size_t getNextFreeRegister() {
        for (std::size_t idx = Register::R8; idx <= Register::R15; ++idx) {
            if (!m_registers.test(idx)) {
                m_registers.set(idx);
                return idx;
            }
        }

        assert(false && "getNextFreeRegister");
    }

    std::string filepath;
    std::bitset<Register::MAX_COUNT> m_registers {};
    std::stringstream output {};
};

} // namespace Emitter
