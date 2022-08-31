#pragma once

#include "environment.h"
#include "parser.h"
#include "error.h"
#include <spdlog/spdlog.h>
#include <ranges>

namespace Resovler {
using namespace Statements;
using namespace Expressions;

using Expressions::Expression;
using Statements::Statement;
using StatementList = std::vector<std::unique_ptr<Statement>>;

enum class FunctionType {
    None,
    Function,
};

enum class State {
    Declared,
    Defined,
    Undefined,
};

struct Resolver
    : public Expressions::ExpressionVisitor, 
             Statements::StatementVisitor 
{
    Resolver() = default;

    void error(const Token& name, std::string_view message) {
        std::string smsg(message);
        throw Error { 
            .type = ErrorType::ResolverError, 
            .token = name, 
            .msg = smsg
        };
    }

    /// When a new variable is declared
    void declare(const Token& name) {
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

    /// When a variable has a value now
    void define(const Token& name) {
        spdlog::info(fmt::format("Resolver: ##{}## {}", scopes.size(), __PRETTY_FUNCTION__));
        if (scopes.empty()) {
            return;
        }

        // change the State of the to Defined
        scopes[scopes.size() - 1][name.lexeme] = State::Defined;
    }

    void begin_scope() {
        spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
        scopes.emplace_back();
    }

    void end_scope() {
        spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
        scopes.pop_back();
    }

    // ====================================================================
    // Resolves
    // ====================================================================

    void resolve(const StatementList& statements) {
        spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));

        for (const auto& statement : statements) {
            resolve(statement);
        }
    }

    void resolve(const std::unique_ptr<Expression>& expression) {
        spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
        expression->accept(*this);
    }

    void resolve(Function& function) {
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

    void resolve(const std::unique_ptr<Statement>& statement) {
        spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
        statement->accept(*this);
    }

    void resolve(Variable& value, const Token& name) {
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

    void resolve(Assignment& value, const Token& name) {
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

    //void resolve(std::shared_ptr<FunctionDefinition> value, const Token& name) {
        //spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
        //std::size_t depth = 0;

        //// iterate over the scopes in reverse
        //for (const auto& map : scopes | std::views::reverse) {
            ////spdlog::info(fmt::format("-- Resolver: {}"));

            //// if the map contains the name of the variable, count the depth and add it to the locals
            //if (map.contains(name.lexeme)) {
                ////if (std::is_same_v<decltype(expr), 
                //locals.insert({ value, depth });
                //spdlog::info(fmt::format("-- Insert into locals with {}:{}", value->to_string(), depth));
                //return;
            //}
            //depth++;
        //}
    //}

    //void resolve(Expression* expr, const Token& name) {
        //spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
        //std::size_t depth = 0;

        //// iterate over the scopes in reverse
        //for (const auto& map : scopes | std::views::reverse) {
            ////spdlog::info(fmt::format("-- Resolver: {}"));

            //// if the map contains the name of the variable, count the depth and add it to the locals
            //if (map.contains(name.lexeme)) {
                ////if (std::is_same_v<decltype(expr), 
                //locals.insert({ , depth });
                //spdlog::info(fmt::format("-- Insert into locals with {}:{}", expr->to_string(), depth));
                //return;
            //}
            //depth++;
        //}
    //}


    // ====================================================================
    // Statements
    // ====================================================================

    /// When a variable is defined e.g 
    /// let x: Int;
    /// or
    /// let x: Int = 10;
    void visit(VariableDefinition& statement) override {
        declare(statement.name);

        // if the variable has an Initializer, we want to resolve this expression aswell
        if (statement.initializer) {
            resolve(statement.initializer);
        }

        define(statement.name);
    }

    void visit(ExpressionStatement& statement) override {
        spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
        resolve(statement.expression);
        //assert(false && "TODO resolver");
    }

    void visit(Print& statement) override {
        spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
    }

    
    /// When a new function is introduced to the scope
    void visit(Function& statement) override {
        spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
        declare(statement.name);
        define(statement.name);
        resolve(statement);
    }

    void visit(Return& statement) override {
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

    // ====================================================================
    // Expressions
    // ====================================================================

    void visit(Assignment& expression) override {
        spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
        resolve(expression.value);
        resolve(expression, expression.name);
        //self.resolve_expr(expr.value.clone())?;
        //self.resolve_local(wrapper, &expr.name);
    }


    void visit(Number& expression) override {
        spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
    }

    void visit(Bool& expression) override {
        spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
    }

    void visit(BinaryOperator& expression) override {
        spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));

        // resolve both sides
        resolve(expression.lhs);
        resolve(expression.rhs);
    }

    void visit(Variable& expression) override {
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

    void visit(Unary& expression) override {
        spdlog::info(fmt::format("Resolver: {}", __PRETTY_FUNCTION__));
        resolve(expression.rhs);
    }
private:
    using VariableName = std::string_view;

    std::vector<std::unordered_map<VariableName, State>> scopes;
    FunctionType current_function { FunctionType::None };
    //std::unordered_map<ValueVariant, std::size_t> locals;
};

} // namespace Resolver
