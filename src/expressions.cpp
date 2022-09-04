#include "expressions.h"
#include <cassert>
#include <charconv>

// ============================================================================
// Assignment
// ============================================================================
Expressions::Assignment::Assignment(Token name, std::unique_ptr<Expression> value)
    : name(name)
    , value(std::move(value))
{
}

std::string Expressions::Assignment::to_string(std::size_t offset) const {
    return fmt::format(
        "{0:>{w}}Assignment:\n"
        "{0:>{w}}  .name = {2}\n"
        "{0:>{w}}  .value =\n{3}\n"
        "{0:>{w}}  .scope_distance = {4}\n",
        "",  // dummy argument for padding
        fmt::arg("w", offset),
        name,
        value ? value->to_string(offset + 4) : "nullptr",
        scope_distance
    );
}

// ============================================================================
// BinaryOperator
// ============================================================================
Expressions::BinaryOperator::BinaryOperator(std::unique_ptr<Expression> lhs, Token operator_type, std::unique_ptr<Expression> rhs) 
    : lhs{std::move(lhs)}
    , operator_type(operator_type)
    , rhs{std::move(rhs)}
{
    // @todo the datatype should be set in the typechecker and not in the parser,
    // since the parser does not know how to resolve local variables to
    // their actual datatype
}

std::string Expressions::BinaryOperator::to_string(std::size_t offset) const {
    return fmt::format(
        "{0:>{w}}BinaryOperator:\n"
        "{0:>{w}}  .datatype = {2}\n"
        "{0:>{w}}  .lhs =\n{3}\n"
        "{0:>{w}}  .operator = {4}\n"
        "{0:>{w}}  .rhs =\n{5}\n",
        "",  // dummy argument for padding
        fmt::arg("w", offset),
        datatype.to_string(),
        lhs->to_string(offset + 4), 
        operator_type, 
        rhs->to_string(offset + 4)
    );
}

// ============================================================================
// Number
// ============================================================================
Expressions::Number::Number(std::string_view sv)
{
    auto result = std::from_chars(sv.data(), sv.data() + sv.size(), value);
    if (result.ec == std::errc::invalid_argument) {
        assert(false && "TODO NUMBER");
    }
    this->datatype = availableDataTypes.at("Int");
}

Expressions::Number::Number(Number&& other) noexcept {
    this->datatype = other.datatype;
}

std::string Expressions::Number::to_string(std::size_t offset) const {
    return fmt::format(
        "{0:>{w}}NumberExpression:\n"
        "{0:>{w}}  .datatype = {2}\n"
        "{0:>{w}}  .value = {3}",
        "",  // dummy argument for padding
        fmt::arg("w", offset),
        datatype.to_string(),
        value
    );
}

// ============================================================================
// Bool
// ============================================================================
Expressions::Bool::Bool(std::string_view sv)
{
    value = (sv == "true");
    datatype = availableDataTypes.at("Bool");
}

std::string Expressions::Bool::to_string(std::size_t offset) const {
    return fmt::format(
        "{0:>{w}}BoolExpression:\n"
        "{0:>{w}}  .datatype = {2}\n"
        "{0:>{w}}  .value = {3}\n",
        "",  // dummy argument for padding
        fmt::arg("w", offset),
        datatype.to_string(),
        value ? "true" : "false"
    );
}

// ============================================================================
// Variable
// ============================================================================
Expressions::Variable::Variable(const Token& name)
: name(name)
{
    // @todo add datatype of variable here.
    // This will probably change when scopes are implemented and we can lookup the variable and get it's type
}

std::string Expressions::Variable::to_string(std::size_t offset) const {
    return fmt::format(
        "{0:>{w}}Variable:\n"
        "{0:>{w}}  .name = {2}\n"
        "{0:>{w}}  .datatype = {3}\n"
        "{0:>{w}}  .scope_distance = {4}\n",
        "",  // dummy argument for padding
        fmt::arg("w", offset),
        name,
        datatype.to_string(),
        scope_distance
    );
}

// ============================================================================
// Unary
// ============================================================================
Expressions::Unary::Unary(Token operator_type, std::unique_ptr<Expression> rhs) 
    : operator_type(operator_type)
    , rhs(std::move(rhs))
{
    datatype = this->rhs->datatype;
}

std::string Expressions::Unary::to_string(std::size_t offset) const {
    return fmt::format(
        "{0:>{w}}UnaryExpression:\n"
        "{0:>{w}}  .operator = {2}\n"
        "{0:>{w}}  .rhs = \n{3}\n"
        "{0:>{w}}  .datatype = {4}\n",
        "",  // dummy argument for padding
        fmt::arg("w", offset),
        operator_type,
        rhs->to_string(offset + 4),
        datatype.to_string()
    );
}

// ============================================================================
// Logical
// ============================================================================
Expressions::Logical::Logical(std::unique_ptr<Expressions::Expression> lhs, 
                              Token operator_type, 
                              std::unique_ptr<Expressions::Expression> rhs) 
    : lhs(std::move(lhs))
    , operator_type(operator_type)
    , rhs(std::move(rhs))
{
    datatype = availableDataTypes["Bool"];
}

std::string Expressions::Logical::to_string(std::size_t offset) const {
    return fmt::format(
        "{0:>{w}}LogicalExpression:\n"
        "{0:>{w}}  .lhs = \n{2}\n"
        "{0:>{w}}  .operator = {3}\n"
        "{0:>{w}}  .rhs = \n{4}\n"
        "{0:>{w}}  .datatype = {5}\n",
        "",  // dummy argument for padding
        fmt::arg("w", offset),
        lhs->to_string(offset + 4),
        operator_type,
        rhs->to_string(offset + 4),
        datatype.to_string()
    );
}

// ============================================================================
// FunctionCall
// ============================================================================
Expressions::FunctionCall::FunctionCall(const Token& name, std::unique_ptr<Expression> callee, std::vector<std::unique_ptr<Expression>> params) 
    : name(name)
    , callee(std::move(callee))
    , params(std::move(params))
{
    //datatype = availableDataTypes["Bool"];
}

std::string Expressions::FunctionCall::to_string(std::size_t offset) const {
    return fmt::format(
        "{0:>{w}}FunctionCall:\n"
        "{0:>{w}}  .callee = \n{2}\n"
        "{0:>{w}}  .params = {3}\n"
        "{0:>{w}}  .scope_distance = {4}\n"
        "{0:>{w}}  .name = {5}\n",
        "",  // dummy argument for padding
        fmt::arg("w", offset),
        callee->to_string(offset + 4),
        "",
        scope_distance,
        name
    );
}
