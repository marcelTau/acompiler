#pragma once

#include "parser.h"
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

struct Resolver 
    : public Expressions::ExpressionVisitor, 
             Statements::StatementVisitor 
{

        Resolver(std::unordered_map<Expression *, std::size_t>& locals)
            : locals(locals)
        {
        }
        
        void resolve(const StatementList& statements) {
            for (const auto& statement : statements) {
                resolve_statement(statement);
            }
        }

        void resolve_statement(const std::unique_ptr<Statement>& statement) {
            statement->accept(*this);
        }

        void resolve_expression(const std::unique_ptr<Expression>& expression) {
            expression->accept(*this);
        }

        void resolve_function(Function& function) {
            const auto enclosing_function = current_function;
            current_function = FunctionType::Function; // @todo refactor and pass as argument if classes are implemented
            begin_scope();

            for (const auto& param : function.params) {
                declare(param);
                define(param);
            }

            resolve(function.body);
            end_scope();
            current_function = enclosing_function;
        }

        void declare(const Token& name) {
            if (scopes.empty()) {
                return;
            }
            if (scopes[scopes.size() - 1].contains(name.lexeme)) {
                spdlog::error(fmt::format("Variable '{}' already exists in this scope.", name.lexeme));
                assert(false);
            }

            scopes[scopes.size() - 1].insert({ name.lexeme, false });
        }

        void define(const Token& name) {
            if (scopes.empty()) {
                return;
            }
            scopes[scopes.size() - 1].insert({ name.lexeme, true });
        }

        void begin_scope() {
            scopes.emplace_back();
        }

        void end_scope() {
            scopes.pop_back();
        }

        // ====================================================================
        // Statements
        // ====================================================================

        void visit(VariableDefinition& statement) override {
            declare(statement.name);

            if (statement.initializer) {
                resolve_expression(statement.initializer);
            }

            define(statement.name);
        }

        void visit(ExpressionStatement& statement) override {
            assert(false && "TODO resolver");
        }

        void visit(Print& statement) override {}
        void visit(Function& statement) override {
            declare(statement.name);
            define(statement.name);
            resolve_function(statement);
        }
        void visit(Return& statement) override {
            if (current_function == FunctionType::None) {
                spdlog::error(fmt::format("Can't return outside of function."));
            }

            if (statement.value) {
                resolve_expression(statement.value);
            }
        }

        // ====================================================================
        // Expressions
        // ====================================================================

        void visit(Assignment& expression) override {}
        void visit(BinaryOperator& expression) override {
            resolve_expression(expression.lhs);
            resolve_expression(expression.rhs);
        }
        void visit(Number& expression) override {}
        void visit(Bool& expression) override {}

        void visit(Variable& expression) override {
            if (!scopes.empty() && scopes[scopes.size() - 1].at(expression.name.lexeme) == false) {
                spdlog::error(fmt::format("Can't read variable '{}' in it's own initializer", expression.name));
                assert(false && "Variable");
            } else {
                resolve_local(&expression, expression.name);
            }
        }

        void resolve_local(Expression* expr, const Token& name) {
            std::size_t depth = 0;

            for (const auto& map : scopes | std::views::reverse) {
                if (map.contains(name.lexeme)) {
                    locals.insert({ expr, depth });
                    return;
                }
                depth++;
            }
        }

        void visit(Unary& expression) override {
            resolve_expression(expression.rhs);
        } 

    private:
        std::vector<std::unordered_map<std::string_view, bool>> scopes;
        FunctionType current_function { FunctionType::None };
        std::unordered_map<Expression *, std::size_t>& locals;
};

} // namespace Resolver
