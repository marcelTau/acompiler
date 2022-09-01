#include "emitter.h"
#include "register.h"

using namespace Statements;
using namespace Expressions;

std::bitset<15> Register::m_registers {};

Emitter::Emitter(std::string filepath)
    : filepath(std::move(filepath))
{
}

// ------------------------------------------------------------------------
// Statements
// ------------------------------------------------------------------------

void Emitter::Emitter::visit(IfStatement& statement) {
    spdlog::info(fmt::format("Emitter: {}", __PRETTY_FUNCTION__));

    // push result of condition on stack
    statement.condition->accept(*this);

    auto reg = Register();
    emit_line(fmt::format("  pop {:64}", reg), "take condition result from stack");
    emit_line(fmt::format("  cmp {:64}, 1", reg), "compare it to 1 to check if its true");

    auto false_label = getLabelName();
    emit_line(fmt::format("  jne {}", false_label), "jump to false label if not true");

    // then branch
    statement.then_branch->accept(*this);

    auto continue_label = getLabelName();
    emit_line(fmt::format("  jmp {}", continue_label), "jump over the bad branch");

    emit_line(fmt::format("{}:", false_label));

    if (statement.else_branch) {
        statement.else_branch->accept(*this);
    }

    emit_line(fmt::format("{}:", continue_label));
}

void Emitter::Emitter::visit(Block& statement) {
    spdlog::info(fmt::format("Emitter: {}", __PRETTY_FUNCTION__));

    // @todo maybe something to do here
    for (auto& s : statement.statements) {
        s->accept(*this);
    }
}

void Emitter::Emitter::visit(VariableDefinition& statement) {
    spdlog::info(fmt::format("Emitter: {}", __PRETTY_FUNCTION__));

    // push initializer value on stack
    statement.initializer->accept(*this);

    // @todo maybe change back ? current_function_stack_offset += (int)statement.initializer->datatype.size;
    current_function_stack_offset += (int)statement.datatype.size;
    statement.offset = current_function_stack_offset;

    auto reg = Register();
    emit_line(fmt::format("  pop {:64}", reg), "pop initializer into register");
    emit_line(fmt::format("  mov QWORD [rbp - {:#x}], {:64}", statement.offset, reg), "store initializer on stack");

    environment.define(statement.name.getLexeme(), std::make_shared<VariableDefinition>(statement));
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

// ------------------------------------------------------------------------
// Expressions
// ------------------------------------------------------------------------

void Emitter::visit(Assignment& expression) {
    spdlog::info(fmt::format("Emitter: {}", __PRETTY_FUNCTION__));
    auto var = lookup_variable(expression);

    if (std::holds_alternative<std::shared_ptr<VariableDefinition>>(var)) {
        std::size_t offset = std::get<std::shared_ptr<VariableDefinition>>(var)->offset;

        // put value on the stack
        expression.value->accept(*this);

        auto reg = Register();
        emit_line(fmt::format("  pop {:64}", reg), "take new value from stack");

        // assign new value to variable
        emit_line(fmt::format("  mov QWORD [rbp - {:#x}], {:64}", offset, reg), "move new value into old position in stack");
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
            auto reg1 = Register();
            auto reg2 = Register();

            emit_line(fmt::format("  pop {:64}", reg1), "take value from stack into first free register");
            emit_line(fmt::format("  pop {:64}", reg2), "take second value from stack");
            emit_line(fmt::format("  add {:64}, {:64}", reg1, reg2));
            emit_line(fmt::format("  push {:64}", reg1));
            break;
        }
        case TokenType::Minus: {
            auto reg1 = Register();
            auto reg2 = Register();
            emit_line(fmt::format("  pop {:64}", reg1), "take value from stack into first free register");
            emit_line(fmt::format("  pop {:64}", reg2), "take second value from stack");
            emit_line(fmt::format("  sub {:64}, {:64}", reg2, reg1));
            emit_line(fmt::format("  push {:64}", reg2));
            break;
        }
        case TokenType::Star: {
            auto reg = Register();
            emit_line(fmt::format("  pop {:64}", reg), "take value from stack into first free register");
            emit_line(fmt::format("  pop rax"), "take second value from stack");
            emit_line(fmt::format("  mul {:64}", reg));
            emit_line(fmt::format("  push rax"));
            break;
        }
        case TokenType::Slash: {
            auto reg = Register();
            emit_line("  xor rdx, rdx", "set rdx to 0 so it does not mess with the division");
            emit_line(fmt::format("  pop {:64}", reg), "take value from stack into first free register");
            emit_line(fmt::format("  pop rax"), "take second value from stack");
            emit_line(fmt::format("  div {:64}", reg));
            emit_line(fmt::format("  push rax"));
            break;
        }
        case TokenType::EqualEqual: {
            auto reg1 = Register();
            auto reg2 = Register();
            emit_line(fmt::format("  pop {:64}", reg1), "take value from stack into first free register");
            emit_line(fmt::format("  pop {:64}", reg2), "take value from stack into first free register");
            emit_line(fmt::format("  cmp {:64}, {:64}", reg1, reg2), "do the comparison");
            emit_line(fmt::format("  sete {:8}", reg1), "Sets register to 1 if comparison is equal");
            emit_line(fmt::format("  push {:64}", reg1), "push result of comparion on stack");
            break;
        }
        case TokenType::BangEqual: {
            auto reg1 = Register();
            auto reg2 = Register();
            emit_line(fmt::format("  pop {:64}", reg1), "take value from stack into first free register");
            emit_line(fmt::format("  pop {:64}", reg2), "take value from stack into first free register");
            emit_line(fmt::format("  cmp {:64}, {:64}", reg1, reg2), "do the comparison");
            emit_line(fmt::format("  setne {:8}", reg1), "Sets register to 1 if comparison is not equal");
            emit_line(fmt::format("  push {:64}", reg1), "push result of comparion on stack");
            break;
        }
        case TokenType::Less: {
            auto reg1 = Register();
            auto reg2 = Register();
            emit_line(fmt::format("  pop {:64}", reg2), "take value from stack into first free register");
            emit_line(fmt::format("  pop {:64}", reg1), "take value from stack into first free register");
            emit_line(fmt::format("  cmp {:64}, {:64}", reg1, reg2), "do the comparison");
            emit_line(fmt::format("  setl {:8}", reg1), "Sets register to 1 if comparison is lower");
            emit_line(fmt::format("  push {:64}", reg1), "push result of comparion on stack");
            break;
        }
        case TokenType::LessEqual: {
            auto reg1 = Register();
            auto reg2 = Register();
            emit_line(fmt::format("  pop {:64}", reg2), "take value from stack into first free register");
            emit_line(fmt::format("  pop {:64}", reg1), "take value from stack into first free register");
            emit_line(fmt::format("  cmp {:64}, {:64}", reg1, reg2), "do the comparison");
            emit_line(fmt::format("  setle {:8}", reg1), "Sets register to 1 if comparison is lower");
            emit_line(fmt::format("  push {:64}", reg1), "push result of comparion on stack");
            break;
        }
        case TokenType::Greater: {
            auto reg1 = Register();
            auto reg2 = Register();
            emit_line(fmt::format("  pop {:64}", reg2), "take value from stack into first free register");
            emit_line(fmt::format("  pop {:64}", reg1), "take value from stack into first free register");
            emit_line(fmt::format("  cmp {:64}, {:64}", reg1, reg2), "do the comparison");
            emit_line(fmt::format("  setg {:8}", reg1), "Sets register to 1 if comparison is lower");
            emit_line(fmt::format("  push {:64}", reg1), "push result of comparion on stack");
            break;
        }
        case TokenType::GreaterEqual: {
            auto reg1 = Register();
            auto reg2 = Register();
            emit_line(fmt::format("  pop {:64}", reg2), "take value from stack into first free register");
            emit_line(fmt::format("  pop {:64}", reg1), "take value from stack into first free register");
            emit_line(fmt::format("  cmp {:64}, {:64}", reg1, reg2), "do the comparison");
            emit_line(fmt::format("  setge {:8}", reg1), "Sets register to 1 if comparison is lower");
            emit_line(fmt::format("  push {:64}", reg1), "push result of comparion on stack");
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
            auto reg = Register();
            emit_line(fmt::format("  mov QWORD {:64}, [rbp - {:#x}]", reg, offset), "take value of variable out of stack");
            emit_line(fmt::format("  push {:64}", reg), "push in onto the stack");
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

std::string Emitter::getLabelName() {
    return ".L" + std::to_string(++label_counter);
}
