#pragma once

#include "datatype.h"
#include "parser.h"
#include "token.h"
#include <spdlog/spdlog.h>
#include <variant>
#include <cassert>

using Value = std::variant<int, bool>;

struct Object {
    DataType type;
    Value value;
};



struct Environment {

    Environment() = default;
    Environment(Environment *enclosing_env)
        : enclosing { enclosing_env }
    {
    }
            
    void define(const std::string& name, const Object& value) {
        values[name] = value;
    }

    Object getAt(std::size_t distance, std::string_view name) {
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

    Object get(const Token& name) {
        try {
            return values.at(name.lexeme);
        } catch (std::out_of_range&) {
            assert(false && "TODO: Environment::get()");
        }
        assert(false);
        return Object{};
    }

    void assign(const Token& name, const Object& value) {
        if (values.contains(name.lexeme)) {
            values[name.lexeme] = value;
        } else if (enclosing != nullptr) {
            enclosing->assign(name, value);
        } else {
            spdlog::error(fmt::format("Undefined variable at '{}'", name));
            assert(false);
        }
    }

    void assignAt(std::size_t distance, const Token& name, const Object& value) {
        if (distance == 0) {
            values.insert({ name.lexeme, value });
        } else {
            assert(enclosing);
            enclosing->assignAt(distance - 1, name, value);
        }
    }

private:
    std::unordered_map<std::string_view, Object> values;
    Environment *enclosing { nullptr };
};
