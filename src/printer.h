#pragma once

#include <sstream>
#include "parser.h"

namespace Printer {

using namespace Statements;
using namespace Expressions;

using Expressions::Expression;
using Statements::Statement;

struct Printer : public Statements::StatementVisitor, Expressions::ExpressionVisitor  {

private:
    std::stringstream output{};
};

}
