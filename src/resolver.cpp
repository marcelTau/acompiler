#include <spdlog/spdlog.h>
#include <ranges>
#include "resolver.h"
#include "error.h"

using namespace Statements;
using namespace Expressions;

// ====================================================================
// Resolves
// ====================================================================

void Resolver::resolve(const StatementList& statements) {
    spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));

    for (const auto& statement : statements) {
        resolve(statement);
    }
}

void Resolver::resolve(const std::unique_ptr<Expression>& expression) {
    spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
    expression->accept(*this);
}

void Resolver::resolve(Function& function) {
    const auto enclosing_function = current_function;

    current_function = FunctionType::Function;

    begin_scope();

    for (const auto& param : function.params) {
        declare(param);
        define(param);
    }

    resolve(function.body);

    end_scope();
    current_function = enclosing_function;
}

void Resolver::resolve(const std::unique_ptr<Statement>& statement) {
    spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
    statement->accept(*this);
}

void Resolver::resolve(Variable& value, const Token& name) {
    spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
    std::size_t depth = 0;

    // iterate over the scopes in reverse
    for (const auto& map : scopes | std::views::reverse) {
        // if the map contains the name of the variable, count the depth and add it to the locals
        if (map.contains(name.lexeme)) {
            // if we want to resolve a local just set the scope_distance value
            value.scope_distance = depth;
            return;
        }
        depth++;
    }
}

void Resolver::resolve(Assignment& value, const Token& name) {
    spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
    std::size_t depth = 0;

    // iterate over the scopes in reverse
    for (const auto& map : scopes | std::views::reverse) {
        // if the map contains the name of the variable, count the depth and add it to the locals
        if (map.contains(name.lexeme)) {
            // if we want to resolve a local just set the scope_distance value
            value.scope_distance = depth;
            return;
        }
        depth++;
    }
}

// ====================================================================
// Statements
// ====================================================================

void Resolver::visit(VariableDefinition& statement) {
    declare(statement.name);

    // if the variable has an Initializer, we want to resolve this expression aswell
    if (statement.initializer) {
        resolve(statement.initializer);
    }

    define(statement.name);
}

void Resolver::visit(ExpressionStatement& statement) {
    spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
    resolve(statement.expression);
}

void Resolver::visit(Print& statement) {
    spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
}

/// When a new function is introduced to the scope
void Resolver::visit(Function& statement) {
    spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
    declare(statement.name);
    define(statement.name);
    resolve(statement);
}

void Resolver::visit(Return& statement) {
    spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));

    // check if we are in a function
    if (current_function == FunctionType::None) {
        spdlog::error(fmt::format("Can't return outside of function."));
        error(Token{}, "Can't return outside of function.");
    }

    // resolve the value of the return statement
    if (statement.value) {
        resolve(statement.value);
    }
}

void Resolver::visit(IfStatement& statement) {
    spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
    // @todo maybe todo something here
    resolve(statement.condition);
    resolve(statement.then_branch);
    if (statement.else_branch) {
        resolve(statement.else_branch);
    }
}

void Resolver::visit(Block& statement) {
    spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
    // @todo maybe todo something here
    resolve(statement.statements);
}

// ====================================================================
// Expressions
// ====================================================================

void Resolver::visit(Assignment& expression) {
    spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
    resolve(expression.value);
    resolve(expression, expression.name);
}


void Resolver::visit(Number& expression) {
    spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
}

void Resolver::visit(Bool& expression) {
    spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
}

void Resolver::visit(BinaryOperator& expression) {
    spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));

    // resolve both sides
    resolve(expression.lhs);
    resolve(expression.rhs);
}

void Resolver::visit(Variable& expression) {
    spdlog::info(fmt::format("++ Resolver: {}", __PRETTY_FUNCTION__));

    const bool isNotEmpty = !scopes.empty();
    const bool isDeclared = scopes[scopes.size() - 1].at(expression.name.lexeme) == State::Declared;

    // If the variable is being used in its own initializer
    if (isNotEmpty && isDeclared) {
        error(expression.name, fmt::format("Can't read variable '{}' in it's own initializer.", expression.name.lexeme));
    } else {
        resolve(expression, expression.name);
    }
}

void Resolver::visit(Unary& expression) {
    spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
    resolve(expression.rhs);
}

void Resolver::visit(Logical& expression) {
    spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
    resolve(expression.lhs);
    resolve(expression.rhs);
}

// ====================================================================
// Helper functions
// ====================================================================

void Resolver::error(const Token& name, std::string_view message) {
    std::string smsg(message);
    throw Error { 
        .type = ErrorType::ResolverError, 
        .token = name, 
        .msg = smsg
    };
}

void Resolver::declare(const Token& name) {
    spdlog::info(fmt::format("Resolver: ##{}## {}", scopes.size(), __PRETTY_FUNCTION__));
    if (scopes.empty()) {
        return;
    }

    // check if the variable already exists in the current scope.
    // if so it means that we try to redefine this variable which is not allowed atm.
    if (scopes[scopes.size() - 1].contains(name.lexeme)) {
        spdlog::error(fmt::format("Variable '{}' already exists in this scope.", name.lexeme));
        error(name, fmt::format("Variable '{}' already exists in this scope.", name.lexeme));
    }

    // If it's not already in the current scope, we insert it
    // and set it's state to declared since it does not yet have a value.
    // This allows variables to be declared without an initializer e.g
    // let x: Int;
    scopes[scopes.size() - 1].insert({ name.lexeme, State::Declared }); // @fixme this is true atm since all variables have to be initialized on declaration

    spdlog::info("Inserted into scopes");
}

void Resolver::define(const Token& name) {
    spdlog::info(fmt::format("Resolver: ##{}## {}", scopes.size(), __PRETTY_FUNCTION__));
    if (scopes.empty()) {
        return;
    }

    // change the State of the to Defined
    scopes[scopes.size() - 1][name.lexeme] = State::Defined;
}

void Resolver::begin_scope() {
    spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
    scopes.emplace_back();
}

void Resolver::end_scope() {
    spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
    scopes.pop_back();
}
