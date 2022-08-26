#pragma once

#include <utility>
#include <vector>
#include <memory>
#include <optional>
#include "token.h"
#include "assert.h"

namespace Statements {
    struct VariableDefinition;
    struct ExpressionStatement;

    struct StatementVisitor {
        virtual void visit(VariableDefinition& statement) = 0;
        virtual void visit(ExpressionStatement& statement) = 0;

        virtual ~StatementVisitor() = default;
    };

    struct Statement {
        virtual ~Statement() = default;
        virtual void accept(StatementVisitor& visitor) = 0;
        virtual std::string to_string() = 0;
    };

    template<typename T>
    struct StatementAcceptor : public Statement {
        void accept(StatementVisitor& visitor) final {
            visitor.visit(static_cast<T&>(*this));
        }
    };

    struct VariableDefinition : public StatementAcceptor<VariableDefinition> {
        VariableDefinition(std::string_view name) : name(name) {}
        std::string name;

        std::string to_string() final {
            return fmt::format("VariableDefinition: .name = {}, ", name);
        }
    };
    struct ExpressionStatement : public StatementAcceptor<ExpressionStatement> {
    };
} // namespace Statements


namespace Expressions {
    struct Assignment;
    struct BinaryOperator;
    struct Number;

    struct ExpressionVisitor {
        virtual void visit(Assignment& expression) = 0;
        virtual void visit(BinaryOperator& expression) = 0;
        virtual void visit(Number& expression) = 0;
    };

    struct Expression {
        virtual ~Expression() = default;
        virtual void accept(ExpressionVisitor& visitor) = 0;
        
        // @todo datatype
    };

    template<typename T>
    struct ExpressionAcceptor : public Expression {
        void accept(ExpressionVisitor& visitor) final {
            visitor.visit(static_cast<T&>(*this));
        }
    };

    struct Assignment : public ExpressionAcceptor<Assignment> {
    };
    struct BinaryOperator : public ExpressionAcceptor<BinaryOperator> {
    };
    struct Number : public ExpressionAcceptor<Number> {
    };

} // namespace Expressions

class Parser {
public:
    using TokenList = std::vector<Token>;
    using StatementList = std::vector<std::unique_ptr<Statements::Statement>>;

    using Statement = std::unique_ptr<Statements::Statement>;

public:
    Parser() = default;

    StatementList parse(const TokenList& tokens) {
        StatementList statements;

        m_tokens = tokens;

        while (not isAtEnd()) {
            try {
                statements.push_back(declaration());
            } catch (std::exception& e) {
                fmt::print(stderr, "ERROR: (TODO): {}\n", e.what());
                return statements;
            } catch (...) {
                fmt::print(stderr, "ERROR: (TODO): ...\n");
                return statements;
            }
        }

        return statements;
    }

    [[nodiscard]] auto isAtEnd() -> bool { return peek().type == TokenType::Eof; }
    [[nodiscard]] auto peek() -> Token { return m_tokens[m_current]; }

    [[nodiscard]] auto declaration() -> Statement {
        if (check(TokenType::Let)) {
            advance();
            return varDeclaration();
        }
        throw "declaration";
    }

    [[nodiscard]] auto varDeclaration() -> Statement {
        const auto name = consume(TokenType::Identifier, "Expect variable name.");
        if (name.has_value()) {
            return std::make_unique<Statements::VariableDefinition>(name->lexeme);
        }
        throw;
    }

    [[nodiscard]] auto consume(const TokenType& ttype, std::string_view msg) -> std::optional<Token> {
        if (check(ttype)) {
            return advance();
        } else {
            fmt::print(stderr, "Error: {}", msg);
            return std::nullopt;
        }
    }

    [[nodiscard]] auto check(const TokenType& ttype) -> bool const {
        if (isAtEnd()) {
            return false;
        }
        return peek().type == ttype;
    }

    auto advance() -> Token {
        if (not isAtEnd()) {
            m_current++;
        }
        return previous();
    }

    [[nodiscard]] auto previous() -> Token {
        return m_tokens[m_current - 1];
    }

private:
    TokenList m_tokens;
    std::size_t m_current { 0 };
};
