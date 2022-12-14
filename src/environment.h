#pragma once

#include "datatype.h"
#include "token.h"
#include "parser.h"
#include <spdlog/spdlog.h>
#include <variant>
#include <cassert>
#include <unordered_map>

using ValueVariant = std::variant<
    std::shared_ptr<Statements::VariableDefinition>,
    std::shared_ptr<Expressions::Variable>,
    std::shared_ptr<Statements::Function>,
    std::shared_ptr<Expressions::FunctionCall>
>;

struct ValuePrintVisitor {
    std::string operator()(std::monostate&) { return "monostate"; }

    std::string operator()(auto& value) {
        return value->to_string();
    }
};

template <typename Value>
struct Environment {
    Environment() = default;
    Environment(std::shared_ptr<Environment> enclosing_env)
        : enclosing { enclosing_env }
    {
    }

    void define(const std::string& name, Value value) {
        spdlog::info(fmt::format("Environment: Define {} {}", name, std::visit(ValuePrintVisitor{}, value)));
        values[name] = value;
    }

    Value getAt(std::size_t distance, std::string_view name) {
        spdlog::info(fmt::format("Environment: get at {} {}", distance, name));
        if (distance == 0) {
            try {
                return values.at(std::string{name});
            } catch (std::out_of_range& e) {
                assert(false && "TODO: Environment::getAt");
            }
        }
        assert(enclosing);
        return enclosing->getAt(distance - 1, name);
    }

    Value get(const Token& name) {
        spdlog::info(fmt::format("Environment: get {}", name));
        try {
            return values.at(name.getLexeme());
        } catch (std::out_of_range&) {
            assert(false && "TODO: Environment::get()");
        }
        assert(false);
        return Value{};
    }

    void assign(const Token& name, Value value) {
        spdlog::info(fmt::format("Environment: assign {} {}", name, value));
        if (values.contains(name.getLexeme())) {
            values[name.getLexeme()] = value;
        } else if (enclosing != nullptr) {
            enclosing->assign(name, value);
        } else {
            spdlog::error(fmt::format("Undefined variable at '{}'", name));
            assert(false);
        }
    }

    void assignAt(std::size_t distance, const Token& name, Value value) {
        spdlog::info(fmt::format("Environment: assign at {} {} {}", distance, name, value));
        if (distance == 0) {
            values.insert({ name.getLexeme(), value });
        } else {
            assert(enclosing);
            enclosing->assignAt(distance - 1, name, value);
        }
    }

private:
    using VariableName = std::string;
    std::unordered_map<VariableName, Value> values;
    std::shared_ptr<Environment<Value>> enclosing { nullptr };
};
