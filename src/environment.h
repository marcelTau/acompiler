#pragma once

#include "datatype.h"
#include "token.h"
#include "parser.h"
#include <spdlog/spdlog.h>
#include <variant>
#include <cassert>
#include <unordered_map>

using Value = std::variant<
    std::shared_ptr<Statements::VariableDefinition>,
    std::shared_ptr<Expressions::Variable>,
    std::shared_ptr<Statements::Function>
>;

struct ValuePrintVisitor {
    template <typename T>
    std::string operator()(const T& value) {
        return value->to_string();
    }
};


struct Environment {

    Environment() = default;
    Environment(std::shared_ptr<Environment> enclosing_env)
        : enclosing { enclosing_env }
    {
    }

    void define(const std::string& name, const Value& value) {
        values[name] = value;
    }

    Value getAt(std::size_t distance, std::string_view name) {
        if (distance == 0) {
            try {
                return values.at(name);
            } catch (std::out_of_range& e) {
                assert(false && "TODO: Environment::getAt");
            }
        }
        assert(enclosing);
        return enclosing->getAt(distance - 1, name);
    }

    Value get(const Token& name) {
        try {
            return values.at(name.lexeme);
        } catch (std::out_of_range&) {
            assert(false && "TODO: Environment::get()");
        }
        assert(false);
        return Value{};
    }

    void assign(const Token& name, const Value& value) {
        if (values.contains(name.lexeme)) {
            values[name.lexeme] = value;
        } else if (enclosing != nullptr) {
            enclosing->assign(name, value);
        } else {
            spdlog::error(fmt::format("Undefined variable at '{}'", name));
            assert(false);
        }
    }

    void assignAt(std::size_t distance, const Token& name, const Value& value) {
        if (distance == 0) {
            values.insert({ name.lexeme, value });
        } else {
            assert(enclosing);
            enclosing->assignAt(distance - 1, name, value);
        }
    }

private:
    using VariableName = std::string_view;
    std::unordered_map<VariableName, Value> values;
    std::shared_ptr<Environment> enclosing { nullptr };
};
