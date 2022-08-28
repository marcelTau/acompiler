#pragma once

#include "parser.h"
#include <fstream>
#include <sstream>


/*
Default asm source code layout:
    
%include "defines.inc"      ; all syscalls defined etc.


section .text
global _start

_start:
    push rbp                ; store old rbp on stack
    mov rbp, rsp            ; store rsp in rbp to use it as base pointer of stackframe

    ;; call main            ; here we would call main. This function has to be defined in the sourcecode. Otherwise it will not work.

    mov rax, 60             ; exit with 0
    mov rdi, 0
    syscall
*/

namespace Emitter {

using namespace Statements;
using namespace Expressions;

using Expressions::Expression;
using Statements::Statement;
using StatementList = std::vector<std::unique_ptr<Statement>>;

struct Emitter : public Expressions::ExpressionVisitor, Statements::StatementVisitor {

    Emitter() = default;

    void emit(const StatementList& statements) {
        emit_line("", "=== Auto-generated code ===");

        emit_line("%include \"defines.inc\"", "bring syscall defines into scope");
        emit_line("section .text", "begin source code segment");
        emit_line("global _start", "make entrypoint function visible to linker");

        emit_line("_start:", "entrypoint function");
        emit_line("  push rbp", "store old rbp on stack");
        emit_line("  mov rbp, rsp", "store stack pointer in rbp so it can be used as the base pointer for the functions stack frame\n");
        emit_line("  call main", "user defined main function\n");
        emit_line("  mov rax, 60", "prepare to exit with code 0");
        emit_line("  mov rsi, 0", "set exit code 0");
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
        output << fmt::format("{:40}; {}\n", line, comment);
    }

    void emit_statement(const std::unique_ptr<Statement>& statement) {
        statement->accept(*this);
    }

    void visit(VariableDefinition& statement) override {}
    void visit(ExpressionStatement& statement) override {}
    void visit(Print& statement)override {}
    void visit(Function& statement) override {
        emit_line(fmt::format("{}:", statement.name.lexeme), "User defined function");
        emit_line("  push rbp");
        emit_line("  mov rbp, rsp");
        emit_line(fmt::format("  sub rsp, {}", statement.stack_size), fmt::format("reserve {} bytes on the stack", statement.stack_size));
        emit_line("  ret");

        //for (const auto& s : statement.body) {
            //s->accept(*this);
        //}
    }

    void visit(Assignment& expression) override {}
    void visit(BinaryOperator& expression) override {}
    void visit(Number& expression)override {}
    void visit(Bool& expression)override {}
    void visit(Variable& expressi) override {}
    void visit(Unary& expression)override {}

private:
    std::stringstream output {};
};

} // namespace Emitter
