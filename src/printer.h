#pragma once

#include <sstream>
#include "parser.h"

namespace Printer {

using namespace Statements;
using namespace Expressions;

using Expressions::Expression;
using Statements::Statement;

struct Printer : public Statements::StatementVisitor, Expressions::ExpressionVisitor  {

    void print(std::unique_ptr<Statement> expr) {
        expr->accept(*this);
        fmt::print("{}\n", output.str());
    }

    template <typename ...Exprs>
    auto parenthesize(const std::string_view& name, const Exprs& ...exprs) -> void {
        output << fmt::format("({}", name);
        ((output << fmt::format(" {}", exprs->to_string())), ...);
        output << ')';
    }

    /// -- Statements -- ///
    void visit(VariableDefinition& statement) override {
    }

    void visit(ExpressionStatement& statement) override {
    }

    void visit(Assignment& statement) override {
    }


    /// -- Expressions -- ///
    void visit(BinaryOperator& expression) override {
        parenthesize(expression.operator_type.lexeme, expression.lhs, expression.rhs);
    }

    void visit(Number& expression) override {
        output << expression.to_string();
    }

private:
    std::stringstream output{};
};

}
