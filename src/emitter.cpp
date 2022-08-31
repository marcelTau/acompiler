#include "emitter.h"

using namespace Statements;
using namespace Expressions;

Emitter::Emitter(std::string filepath)
    : filepath(std::move(filepath))
{
}

void Emitter::Emitter::visit(IfStatement& /* statement */) {
    spdlog::info(fmt::format("Emitter: {}", __PRETTY_FUNCTION__));
    assert(false && "If statement");
}

void Emitter::Emitter::visit(Block& /* statement */) {
    spdlog::info(fmt::format("Emitter: {}", __PRETTY_FUNCTION__));
    assert(false && "Block statement");
}

void Emitter::Emitter::visit(VariableDefinition& statement) {
    spdlog::info(fmt::format("Emitter: {}", __PRETTY_FUNCTION__));

    // push initializer value on stack
    statement.initializer->accept(*this);

    current_function_stack_offset += (int)statement.initializer->datatype.size;
    statement.offset = current_function_stack_offset;

    auto idx1 = getNextFreeRegister();
    emit_line(fmt::format("  pop {}", registerNames[idx1]), "pop initializer into register");
    emit_line(fmt::format("  mov QWORD [rbp - {:#x}], {}", statement.offset, registerNames[idx1]), "store initializer on stack");

    environment.define(statement.name.getLexeme(), std::make_shared<VariableDefinition>(statement));

    // cleanup registers
    m_registers.flip(idx1);
}

void Emitter::visit(ExpressionStatement& statement) {
    spdlog::info(fmt::format("Emitter: {}", __PRETTY_FUNCTION__));
    statement.expression->accept(*this);
}

void Emitter::visit(Print&  /*statement*/) {
    spdlog::info(fmt::format("Emitter: {}", __PRETTY_FUNCTION__));
}

void Emitter::visit(Return& statement) {
    spdlog::info(fmt::format("Emitter: {}", __PRETTY_FUNCTION__));
    statement.value->accept(*this);
    emit_line("  pop rax", "pop result of binary expression into rax");
}

void Emitter::visit(Function& statement) {
    spdlog::info(fmt::format("Emitter: {}", __PRETTY_FUNCTION__));
    // @todo pretty print the params of the function as comment above
    emit_line("");
    emit_line(fmt::format("{}:", statement.name.lexeme), "User defined function");
    emit_line("  push rbp", "save rbp since it is callee saved");
    emit_line("  mov rbp, rsp", "setup rbp to use it as the base pointer");
    emit_line(fmt::format("  sub rsp, 0x18", statement.stack_size), fmt::format("reserve {} bytes on the stack", statement.stack_size));

    // @todo add size of params here
    current_function_stack_offset = 0;

    for (const auto& s : statement.body) {
        s->accept(*this);
    }

    emit_line("  mov rsp, rbp", "restore rsp (cleanup stack)");
    emit_line("  pop rbp", "get rbp back, since it is callee saved");
    emit_line("  ret");
}

void Emitter::visit(Assignment& expression) {
    spdlog::info(fmt::format("Emitter: {}", __PRETTY_FUNCTION__));
    auto var = lookup_variable(expression);

    if (std::holds_alternative<std::shared_ptr<VariableDefinition>>(var)) {
        std::size_t offset = std::get<std::shared_ptr<VariableDefinition>>(var)->offset;

        // put value on the stack
        expression.value->accept(*this);

        auto idx1 = getNextFreeRegister();
        emit_line(fmt::format("  pop {}", registerNames[idx1]), "take new value from stack");

        // assign new value to variable
        emit_line(fmt::format("  mov QWORD [rbp - {:#x}], {}", offset, registerNames[idx1]), "move new value into old position in stack");
        m_registers.flip(idx1);
    }
    spdlog::info(fmt::format("Done: Emitter: {}", __PRETTY_FUNCTION__));
}

void Emitter::visit(BinaryOperator& expression) {
    spdlog::info(fmt::format("Emitter: {}", __PRETTY_FUNCTION__));
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
}

void Emitter::visit(Number& expression) {
    spdlog::info(fmt::format("Emitter: {}", __PRETTY_FUNCTION__));
    emit_line(fmt::format("  push {}", expression.value), "push number value on stack");
}

void Emitter::visit(Bool& expression) {
    spdlog::info(fmt::format("Emitter: {}", __PRETTY_FUNCTION__));
    emit_line(fmt::format("  push {}", expression.value ? 1 : 0), "push bool value on stack");
}

void Emitter::visit(Variable& expression) {
    spdlog::info(fmt::format("Emitter: {}", __PRETTY_FUNCTION__));
    // make lookup to get the right variable with the correct offset
    auto var = lookup_variable(expression);

    if (std::holds_alternative<std::shared_ptr<VariableDefinition>>(var)) {
        std::size_t offset = std::get<std::shared_ptr<VariableDefinition>>(var)->offset;

        // variable is not on the stack yet
        if (offset == -1) {
            std::get<std::shared_ptr<VariableDefinition>>(var)->initializer->accept(*this);
        } else {
            auto idx1 = getNextFreeRegister();
            emit_line(fmt::format("  mov QWORD {}, [rbp - {:#x}]", registerNames[idx1], offset), "take value of variable out of stack");
            emit_line(fmt::format("  push {}", registerNames[idx1]), "push in onto the stack");
            //emit_line(fmt::format("  mov rax, {}", registerNames[idx1]), "put it in rax");
            m_registers.flip(idx1);
        }
    } else {
        assert(false && "kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk");
    }
}

void Emitter::visit(Unary& /* expression */) {
    spdlog::info(fmt::format("Emitter: {}", __PRETTY_FUNCTION__));
    assert(false && "unary");
}

void Emitter::visit(Logical& /* expression */) {
    spdlog::info(fmt::format("Emitter: {}", __PRETTY_FUNCTION__));
    assert(false && "logical");
}

// ----------------------------------------------------------------------------
// Helper Functions
// ----------------------------------------------------------------------------

ValueVariant Emitter::lookup_variable(const Variable& var) {
    spdlog::info(fmt::format("Emitter: {}", __PRETTY_FUNCTION__));

    try {
        const auto distance = var.scope_distance;
        auto found_value = environment.getAt(distance, var.name.lexeme);
        return found_value;
    } catch (std::out_of_range&) {
        assert(false && "no global env right now");
    } catch (...) {
        assert(false && "no global env right now .. something else");
    }
}

ValueVariant Emitter::lookup_variable(const Assignment& var) {
    spdlog::info(fmt::format("Emitter: {}", __PRETTY_FUNCTION__));
    try {
        const auto distance = var.scope_distance;
        auto found_value = environment.getAt(distance, var.name.lexeme);
        return found_value;
    } catch (std::out_of_range&) {
        assert(false && "no global env right now");
    } catch (...) {
        assert(false && "no global env right now .. something else");
    }
}

void Emitter::emit(const StatementList& statements) {
    spdlog::info(fmt::format("Emitter: {}", __PRETTY_FUNCTION__));
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
}

void Emitter::emit_line(std::string_view line, std::string_view comment) {
    output << fmt::format("{:30}; {}\n", line, comment);
}

void Emitter::emit_statement(const std::unique_ptr<Statement>& statement) {
    spdlog::info(fmt::format("Emitter: {}", __PRETTY_FUNCTION__));
    statement->accept(*this);
}

std::size_t Emitter::getNextFreeRegister() {
    for (std::size_t idx = Register::R8; idx <= Register::R15; ++idx) {
        if (!m_registers.test(idx)) {
            m_registers.set(idx);
            return idx;
        }
    }
    assert(false && "getNextFreeRegister");
}
