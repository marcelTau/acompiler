#pragma once

#include <utility>
#include <vector>
#include <memory>
#include <optional>
#include <charconv>
#include <cassert>
#include "token.h"

/// @todo ugly hacky stuff, try to fix this pls
namespace Expressions {
    struct ExpressionVisitor;
    struct Expression {
        virtual ~Expression() = default;
        virtual void accept(ExpressionVisitor& visitor) = 0;
        
        virtual std::string to_string() = 0;
        // @todo datatype
    };
} // namespace Expression


namespace Statements {
    using Expressions::Expression;

    struct VariableDefinition;
    struct ExpressionStatement;
    struct Assignment;

    struct StatementVisitor {
        virtual void visit(VariableDefinition& statement) = 0;
        virtual void visit(ExpressionStatement& statement) = 0;
        virtual void visit(Assignment& statement) = 0;

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
        VariableDefinition(std::string_view name, std::unique_ptr<Expression> initializer)
            : name(name)
            , initializer(std::move(initializer))
        {
        }

        std::string name;
        std::unique_ptr<Expression> initializer;

        std::string to_string() final {
            return fmt::format("VariableDefinition: .name = {}, .initializer {}", name, initializer ? initializer->to_string() : "nullptr");
        }
    };

    struct ExpressionStatement : public StatementAcceptor<ExpressionStatement> {
        std::string to_string() final {
            return "";
        }
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
        Number(std::string_view sv)
        {
            auto result = std::from_chars(sv.data(), sv.data() + sv.size(), value);
            if (result.ec == std::errc::invalid_argument) {
                assert(false && "TODO NUMBER");
            }
        }

        int value;

        std::string to_string() final {
            return fmt::format("NumberExpression: {}", value);
        }
    };

} // namespace Expressions

class Parser {
public:
    using TokenList = std::vector<Token>;
    using StatementList = std::vector<std::unique_ptr<Statements::Statement>>;

    using Statement = Statements::Statement;
    using UniqStatement = std::unique_ptr<Statement>;

    using Expression = Expressions::Expression;
    using UniqExpression = std::unique_ptr<Expression>;

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

    [[nodiscard]] auto declaration() -> UniqStatement {
        if (checkAndAdvance(TokenType::Let)) {
            return varDeclaration();
        }
        throw "declaration";
    }

    [[nodiscard]] auto varDeclaration() -> UniqStatement {
        const auto name = consume(TokenType::Identifier, "Expect variable name.");

        if (!name.has_value()) {
            throw;
        }

        UniqExpression initializer;

        if (checkAndAdvance(TokenType::Equal)) {
            initializer = expression();
        }

        std::ignore = consume(TokenType::Semicolon, "Expect ';' after variable declaration.");
        return std::make_unique<Statements::VariableDefinition>(name->lexeme, std::move(initializer));
    }

    [[nodiscard]] auto expression() -> UniqExpression {
        return assignment();
    }

    [[nodiscard]] auto assignment() -> UniqExpression {
        auto expr = primary(); // @todo change this

        if (checkAndAdvance(TokenType::Equal)) {
            fmt::print("todo multi assignment");
        }
        return expr;
    }

    [[nodiscard]] auto primary() -> UniqExpression {
        if (checkAndAdvance(TokenType::Number)) {
            return std::make_unique<Expressions::Number>(previous().lexeme);
        }

        return nullptr;
    }

    [[nodiscard]] auto consume(const TokenType& ttype, std::string_view msg) -> std::optional<Token> {
        if (check(ttype)) {
            return advance();
        } else {
            fmt::print(stderr, "Error: {}", msg);
            return std::nullopt;
        }
    }

    [[nodiscard]] auto checkAndAdvance(const TokenType& ttype) -> bool const {
        const auto result = check(ttype);
        if (result) {
            advance();
        }
        return result;
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
