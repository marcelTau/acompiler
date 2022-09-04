#include <sstream>

#include "expressions.h"
#include "fmt/format.h"
#include "statements.h"

// ============================================================================
// VariableDefinition
// ============================================================================
Statements::VariableDefinition::VariableDefinition(const Token& name, std::unique_ptr<Expressions::Expression> initializer, DataType data_type)
    : name(name)
    , initializer(std::move(initializer))
    , datatype(std::move(data_type))
{
}

Statements::VariableDefinition::VariableDefinition(VariableDefinition& other)
    : name(other.name)
    , initializer { std::move(other.initializer) }
    , datatype(other.datatype)
    , offset(other.offset)
    , scope_distance(other.scope_distance)
{
}
        
std::string Statements::VariableDefinition::to_string(std::size_t offset) const {
    return fmt::format(
        "{0:>{w}}VariableDefinition:\n"
        "{0:>{w}}  .name = {2}\n"
        "{0:>{w}}  .initializer =\n{3}\n"
        "{0:>{w}}  .datatype = {4}\n"
        "{0:>{w}}  .scope_distance = {5}\n",
        "",  // dummy argument for padding
        fmt::arg("w", offset),
        name,
        initializer ? initializer->to_string(offset + 4) : "nullptr",
        datatype.to_string(),
        scope_distance
    );
}

// ============================================================================
// ExpressionStatement
// ============================================================================
Statements::ExpressionStatement::ExpressionStatement(std::unique_ptr<Expressions::Expression> expr)
    : expression(std::move(expr))
{
}

std::string Statements::ExpressionStatement::to_string(std::size_t offset) const {
    return fmt::format(
        "{0:>{w}}ExpressionStatement:\n"
        "{0:>{w}}  .expression =\n{2}\n",
        "",  // dummy argument for padding
        fmt::arg("w", offset),
        expression ? expression->to_string(offset + 4) : "nullptr"
    );
}

// ============================================================================
// Print
// ============================================================================
Statements::Print::Print(std::unique_ptr<Expressions::Expression> expr)
    : expression(std::move(expr))
{
}

std::string Statements::Print::to_string(std::size_t offset) const {
    return fmt::format(
        "{0:>{w}}PrintStatement:\n"
        "{0:>{w}}  .expression = {2}\n",
        "",  // dummy argument for padding
        fmt::arg("w", offset),
        expression->to_string(offset + 4)
    );
}

// ============================================================================
// Function
// ============================================================================
Statements::Function::Function(Token name, std::vector<Token> params, std::vector<std::unique_ptr<Statement>> body, DataType return_datatype)
    : name(name)
    , params(std::move(params))
    , body(std::move(body))
    , return_datatype(std::move(return_datatype))
{
    //stack_size = params.size() * 4; // @todo for now assume that every datatype use 4 bytes and don't care about local variables
}

std::string Statements::Function::to_string(std::size_t offset) const {
    std::stringstream ss{};

    for (auto& statement : body) {
        ss << statement->to_string(offset + 4);
    }

    return fmt::format(
        "{0:>{w}}FunctionStatement:\n"
        "{0:>{w}}  .returntype = {2}\n"
        "{0:>{w}}  .name = {3}\n"
        "{0:>{w}}  .params = {4}\n"
        "{0:>{w}}  .body = \n{5}\n"
        "{0:>{w}}  .scope_distance = {6}\n",
        "",  // dummy argument for padding
        fmt::arg("w", offset),
        return_datatype.to_string(),
        name,
        fmt::join(params, ", "),
        ss.str(),
        scope_distance
    );
}

// ============================================================================
// Return
// ============================================================================
Statements::Return::Return(std::unique_ptr<Expressions::Expression> value)
    : value(std::move(value))
{
}

std::string Statements::Return::to_string(std::size_t offset) const {
    return fmt::format(
        "{0:>{w}}Return:\n"
        "{0:>{w}}  .value =\n{2}\n",
        "",  // dummy argument for padding
        fmt::arg("w", offset),
        value ? value->to_string(offset + 4) : "nullptr"
    );
}

// ============================================================================
// Block
// ============================================================================
Statements::Block::Block(std::vector<std::unique_ptr<Statement>> statements)
    : statements(std::move(statements))
{
}

std::string Statements::Block::to_string(std::size_t offset) const {
    std::stringstream ss;

    for (const auto& statement : statements) {
        ss << statement->to_string(offset + 4) << '\n';
    }
    return fmt::format(
        "{0:>{w}}Block:\n"
        "{0:>{w}}  .statements =\n{2}\n",
        "",  // dummy argument for padding
        fmt::arg("w", offset),
        ss.str()
    );
}

// ============================================================================
// If statement
// ============================================================================
Statements::IfStatement::IfStatement(std::unique_ptr<Expressions::Expression> condition, std::unique_ptr<Statement> then_branch, std::unique_ptr<Statement> else_branch)
    : condition(std::move(condition))
    , then_branch(std::move(then_branch))
    , else_branch(std::move(else_branch))
{
}

std::string Statements::IfStatement::to_string(std::size_t offset) const {
    return fmt::format(
        "{0:>{w}}IfStatement:\n"
        "{0:>{w}}  .condition =\n{2}\n"
        "{0:>{w}}  .then_branch =\n{3}\n"
        "{0:>{w}}  .else_branch =\n{4}\n",
        "",  // dummy argument for padding
        fmt::arg("w", offset),
        condition ? condition->to_string(offset + 4) : "nullptr",
        then_branch ? then_branch->to_string(offset + 4) : "nullptr",
        else_branch ? else_branch->to_string(offset + 4) : "nullptr"
    );
}
