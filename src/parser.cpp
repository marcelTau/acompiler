#include "parser.h"
#include "fmt/ranges.h"
#include "spdlog/spdlog.h"

Parser::StatementList Parser::parse(const TokenList& tokens) {
    StatementList statements;

    m_tokens = tokens;

    spdlog::info(fmt::format("Start parsing with {} tokens", m_tokens.size()));

    while (! isAtEnd()) {
        auto decl = declaration();

        if (!decl) {
            spdlog::error(fmt::format("ParserError: {} {}", decl.get_err().msg, decl.get_err().token));
            return statements;
        }

        statements.push_back(decl.unwrap());
    }

    spdlog::info(fmt::format("Finished parsing with {} statements", statements.size()));
    return statements;
}

// ============================================================================
// Helper Functions
// ============================================================================
auto Parser::isAtEnd() -> bool {
    return peek().type == TokenType::Eof;
}

auto Parser::peek() -> Token {
    return m_tokens[m_current];
}

auto Parser::consume(const TokenType& ttype, const std::string& msg) -> Result<Token> {
    if (check(ttype)) {
        return Result<Token>(advance());
    } else {
        return Result<Token>::ParseError(peek(), msg);
    }
}

template <typename ...Tokens>
auto Parser::checkAndAdvance(Tokens&& ...tokens) -> bool const {
    bool found = (check(tokens) || ...);
    if (found) {
        advance();
    }
    return found;
}

auto Parser::check(const TokenType& ttype) -> bool const {
    if (isAtEnd()) {
        return false;
    }
    return peek().type == ttype;
}

auto Parser::advance() -> Token {
    if (not isAtEnd()) {
        m_current++;
    }
    return previous();
}

auto Parser::previous() -> Token {
    return m_tokens[m_current - 1];
}

auto Parser::errorStmt(const Token& token, const std::string& msg) -> Result<UniqStatement> {
    m_hasError = true;
    return Result<UniqStatement>::ParseError(token, msg);
}

auto Parser::errorExpr(const Token& token, const std::string& msg) -> Result<UniqExpression> {
    m_hasError = true;
    return Result<UniqExpression>::ParseError(token, msg);
}

// ============================================================================
// Statements
// ============================================================================
auto Parser::declaration() -> Result<UniqStatement> {
    spdlog::info(fmt::format("Parser: {}", __PRETTY_FUNCTION__));
    if (checkAndAdvance(TokenType::Let)) {
        return varDeclaration();
    } else if (checkAndAdvance(TokenType::Fun)) {
        return function();
    } else {
        return statement();
    }
    assert(false);
}

auto Parser::function() -> Result<UniqStatement> {
    spdlog::info(fmt::format("Parser: {}", __PRETTY_FUNCTION__));
    auto name = consume(TokenType::Identifier, "Expect function name.");
    std::ignore = consume(TokenType::LeftParen, "Expect '(' after function name.");

    std::vector<Token> params;
    if (!check(TokenType::RightParen)) {
        auto param = consume(TokenType::Identifier, "Expect parameter name.");
        if (!param) {
            return param.get_err();
        }
        params.push_back(param.unwrap());

        while (checkAndAdvance(TokenType::Comma)) {
            param = consume(TokenType::Identifier, "Expect parameter name.");
            if (!param) {
                return param.get_err();
            }
            params.push_back(param.unwrap());
        }
    }

    std::ignore = consume(TokenType::RightParen, "Expect ')' after parameters.");
    std::ignore = consume(TokenType::Arrow, "Expect '->' after function declaration.");

    auto return_datatype_token = consume(TokenType::Identifier, "Expect return value after '->'.");

    if (!return_datatype_token) {
        return return_datatype_token.get_err();
    }

    auto return_datatype_name = return_datatype_token.unwrap().lexeme;

    DataType datatype {};

    try {
        datatype = availableDataTypes.at(return_datatype_name);
    } catch (std::out_of_range&) {
        return Result<UniqStatement>::ParseError(return_datatype_token.unwrap(), 
                                                 fmt::format("Expect datatype after '->' found {}.", 
                                                 return_datatype_name));
    }

    current_function_end_depth = 0;
    auto block_stmts = block();

    if (!block_stmts) {
        return block_stmts.get_err();
    }

    auto function_stmt = std::make_unique<Statements::Function>(name.unwrap(), params, block_stmts.unwrap(), datatype);
    return Result<UniqStatement>(std::move(function_stmt));
}

auto Parser::block() -> Result<std::vector<UniqStatement>> {
    spdlog::info(fmt::format("Parser: {}", __PRETTY_FUNCTION__));
    std::vector<UniqStatement> statements;

    // @todo this needs to change when there are other constructs that use the
    // end keyword such as 'if' or 'for'
    while (!check(TokenType::End) && !isAtEnd() && !check(TokenType::Else)) {
        auto stmt = declaration();
        if (!stmt) {
            return stmt.get_err();
        }
        statements.emplace_back(stmt.unwrap());
    }
    std::ignore = consume(TokenType::End, "Expect 'end' after block.");
    return statements;
}

auto Parser::statement() -> Result<UniqStatement> {
    spdlog::info(fmt::format("Parser: {}", __PRETTY_FUNCTION__));
    if (checkAndAdvance(TokenType::Print)) {
        return printStatement();
    }

    if (checkAndAdvance(TokenType::Return)) {
        return returnStatement();
    }

    if (checkAndAdvance(TokenType::If)) {
        return ifStatement();
    }

    //if (checkAndAdvance(TokenType::Then)) {
        //// @todo errorhandling else statement fails here
        //auto newBlock = std::make_unique<Statements::Block>(block().unwrap());
        //return Result<UniqStatement>(std::move(newBlock));
    //}

    return expressionStatement();
}

auto Parser::ifStatement() -> Result<UniqStatement> {
    spdlog::info(fmt::format("Parser: {}", __PRETTY_FUNCTION__));
    auto condition = expression();
    if (!condition) {
        return condition.get_err();
    }

    std::ignore = consume(TokenType::Then, "Expect 'then' after if-condition.");

    spdlog::warn(fmt::format("Parser: get then branch: {}", peek()));
    auto then_branch = block();
    spdlog::warn(fmt::format("Parser: got then branch: {}", peek()));
    //auto then_branch = statement();
    if (!then_branch) {
        return then_branch.get_err();
    }

    std::vector<std::unique_ptr<Statements::Statement>> else_branch { };

    //auto else_branch_result = statement();
    //if (!else_branch_result) {
        //return else_branch_result.get_err();
    //}

    //else_branch = else_branch_result.unwrap();


    if (checkAndAdvance(TokenType::Else)) {
        spdlog::warn(fmt::format("Parser: get else branch"));
        auto else_branch_result = block();
        spdlog::warn(fmt::format("Parser: got else branch: {}", previous()));
        if (!else_branch_result) {
            return else_branch_result.get_err();
        }
        else_branch = else_branch_result.unwrap();
    }

    auto then_block = std::make_unique<Statements::Block>(std::move(then_branch.unwrap()));
    auto else_block = std::make_unique<Statements::Block>(std::move(else_branch));
    auto ifStatement = std::make_unique<Statements::IfStatement>(condition.unwrap(), std::move(then_block), std::move(else_block));
    return Result<UniqStatement>(std::move(ifStatement));
}

auto Parser::returnStatement() -> Result<UniqStatement> {
    spdlog::info(fmt::format("Parser: {}", __PRETTY_FUNCTION__));
    std::unique_ptr<Expression> value;
    if (!check(TokenType::Semicolon)) {
        auto value_result = expression();
        if (!value_result) {
            return value_result.get_err();
        }
        value = value_result.unwrap();
    }

    std::ignore = consume(TokenType::Semicolon, "Expect ';' after return.");

    auto returnStatement = std::make_unique<Statements::Return>(std::move(value));
    return Result<UniqStatement>(std::move(returnStatement));
}

auto Parser::expressionStatement() -> Result<UniqStatement> {
    spdlog::info(fmt::format("Parser: {}", __PRETTY_FUNCTION__));
    auto expr = expression();
    if (!expr) {
        return expr.get_err();
    }
    std::ignore = consume(TokenType::Semicolon, "Expect ';' after expression.");

    auto exprStatement = std::make_unique<Statements::ExpressionStatement>(expr.unwrap());
    return Result<UniqStatement>(std::move(exprStatement));
}

auto Parser::printStatement() -> Result<UniqStatement> {
    spdlog::info(fmt::format("Parser: {}", __PRETTY_FUNCTION__));
    auto expr = expression();
    if (!expr) {
        return expr.get_err();
    }
    std::ignore = consume(TokenType::Semicolon, "Expect ';' after value.");

    auto printExpr = std::make_unique<Statements::Print>(expr.unwrap());
    return Result<UniqStatement>(std::move(printExpr));
}

auto Parser::varDeclaration() -> Result<UniqStatement> {
    spdlog::info(fmt::format("Parser: {}", __PRETTY_FUNCTION__));
    const auto name = consume(TokenType::Identifier, "Expect variable name.");

    if (!name) {
        return Result<UniqStatement>::Error(name.get_err());
    }

    DataType datatype = {};

    if (checkAndAdvance(TokenType::Colon)) {
        auto datatype_token = consume(TokenType::Identifier, "Exptect datatype after ':'.");

        if (!datatype_token) {
            return datatype_token.get_err();
        }

        const auto datatype_name = datatype_token.unwrap().lexeme;

        try {
            datatype = availableDataTypes.at(datatype_name);
        } catch (std::out_of_range& e) {
            return Result<UniqStatement>::ParseError(datatype_token.unwrap(), 
                    fmt::format("Expect datatype after ':' found {}.", datatype_name));
        }
    }

    UniqExpression initializer;

    if (checkAndAdvance(TokenType::Equal)) {
        if (auto result = expression(); ! result) {
            initializer = nullptr;
        } else {
            initializer = result.unwrap();
        }
    } else {
        return Result<UniqStatement>::ParseError(name.unwrap(), fmt::format("All variables have to be initialized"));
    }

    std::ignore = consume(TokenType::Semicolon, "Expect ';' after variable declaration.");

    auto varDefinition = std::make_unique<Statements::VariableDefinition>(name.unwrap(), std::move(initializer), datatype);

    return Result<UniqStatement>(std::move(varDefinition));
}

// ============================================================================
// Expressions
// ============================================================================
auto Parser::expression() -> Result<UniqExpression> {
    spdlog::info(fmt::format("Parser: {}", __PRETTY_FUNCTION__));
    return assignment();
}

auto Parser::assignment() -> Result<UniqExpression> {
    spdlog::info(fmt::format("Parser: {}", __PRETTY_FUNCTION__));
    auto expr = or_();

    if (!expr) {
        return expr.get_err();
    }

    if (checkAndAdvance(TokenType::Equal)) {
        //auto equals = previous(); // use this as error token
        auto value = assignment();

        if (!value) {
            return value.get_err();
        }

        // @todo add errorhandling
        auto name = dynamic_cast<Expressions::Variable *>(expr.unwrap().get())->name;
        auto assignment = std::make_unique<Expressions::Assignment>(name, value.unwrap());

        return Result<UniqExpression>(std::move(assignment));
    }
    return expr;
}

auto Parser::or_() -> Result<UniqExpression> {
    spdlog::info(fmt::format("Parser: {}", __PRETTY_FUNCTION__));
    auto expr = and_();
    if (!expr) {
        return expr.get_err();
    }

    auto expr_unwrapped = expr.unwrap();

    while (checkAndAdvance(TokenType::Or)) {
        auto op = previous();
        auto right = and_();
        if (!right) {
            return right.get_err();
        }
        expr_unwrapped = std::make_unique<Expressions::Logical>(std::move(expr_unwrapped), op, right.unwrap());
    }

    return expr_unwrapped;
}

auto Parser::and_() -> Result<UniqExpression> {
    spdlog::info(fmt::format("Parser: {}", __PRETTY_FUNCTION__));
    auto expr = equality();
    if (!expr) {
        return expr.get_err();
    }

    auto expr_unwrapped = expr.unwrap();

    while (checkAndAdvance(TokenType::And)) {
        auto op = previous();
        auto right = equality();
        if (!right) {
            return right.get_err();
        }
        expr_unwrapped = std::make_unique<Expressions::Logical>(std::move(expr_unwrapped), op, right.unwrap());
    }

    return expr_unwrapped;
}

auto Parser::equality() -> Result<UniqExpression> {
    spdlog::info(fmt::format("Parser: {}", __PRETTY_FUNCTION__));
    auto expr = comparison();
    if (!expr) {
        return expr.get_err();
    }

    auto expr_unwrapped = expr.unwrap();

    while (checkAndAdvance(TokenType::EqualEqual, TokenType::BangEqual)) {
        auto op = previous();
        auto right = comparison();
        if (!right) {
            return right.get_err();
        }
        expr_unwrapped = std::make_unique<Expressions::BinaryOperator>(std::move(expr_unwrapped), op, right.unwrap());
    }

    return expr_unwrapped;
}

auto Parser::comparison() -> Result<UniqExpression> {
    spdlog::info(fmt::format("Parser: {}", __PRETTY_FUNCTION__));
    auto expr = term();
    if (!expr) {
        return expr.get_err();
    }

    auto expr_unwrapped = expr.unwrap();

    while (checkAndAdvance(TokenType::Greater, TokenType::GreaterEqual, TokenType::Less, TokenType::LessEqual)) {
        auto op = previous();
        auto right = term();
        if (!right) {
            return right.get_err();
        }
        expr_unwrapped = std::make_unique<Expressions::BinaryOperator>(std::move(expr_unwrapped), op, right.unwrap());
    }

    return expr_unwrapped;
}

auto Parser::term() -> Result<UniqExpression> {
    spdlog::info(fmt::format("Parser: {}", __PRETTY_FUNCTION__));
    auto expr = factor();
    if (!expr) {
        return expr.get_err();
    }

    auto expr_unwrapped = expr.unwrap();

    while (checkAndAdvance(TokenType::Plus, TokenType::Minus)) {
        auto op = previous();
        auto right = factor();

        if (!right) {
            return right.get_err();
        }
        expr_unwrapped = std::make_unique<Expressions::BinaryOperator>(std::move(expr_unwrapped), op, right.unwrap());
    }

    return expr_unwrapped;
}

auto Parser::factor() -> Result<UniqExpression> {
    spdlog::info(fmt::format("Parser: {}", __PRETTY_FUNCTION__));
    auto expr = unary();
    if (!expr) {
        return Result<UniqExpression>::Error(expr.get_err());
    }

    auto expr_unwrapped = expr.unwrap();

    while (checkAndAdvance(TokenType::Star, TokenType::Slash)) {
        auto op = previous();
        auto right = unary();
        if (!right) {
            return right.get_err();
        }
        expr_unwrapped = std::make_unique<Expressions::BinaryOperator>(std::move(expr_unwrapped), op, right.unwrap());
    }

    return expr_unwrapped;
}

auto Parser::unary() -> Result<UniqExpression> {
    spdlog::info(fmt::format("Parser: {}", __PRETTY_FUNCTION__));
    if (checkAndAdvance(TokenType::Bang, TokenType::Minus)) {
        auto op = previous();
        auto right = unary();

        if (!right) {
            return right.get_err();
        }
        auto unaryExpr = std::make_unique<Expressions::Unary>(op, right.unwrap());
        return Result<UniqExpression>(std::move(unaryExpr));
    } else {
        return call();
    }
}

auto Parser::call() -> Result<UniqExpression> {
    auto expr = primary();
    if (!expr) {
        return expr.get_err();
    }
    auto expr_unwrapped = expr.unwrap();

    std::vector<UniqExpression> arguments;

    auto name = previous();

    // @todo errorhandling
    if (checkAndAdvance(TokenType::LeftParen)) {
        if (!check(TokenType::RightParen)) {
            auto arg = expression();
            if (!arg) {
                return arg.get_err();
            }
            arguments.push_back(arg.unwrap());

            while (checkAndAdvance(TokenType::Comma)) {
                arg = expression();
                if (!arg) {
                    return arg.get_err();
                }
                arguments.push_back(arg.unwrap());
            }
        }

        std::ignore = consume(TokenType::RightParen, "Expect ')' after arguments.");

        auto function_call_expr = std::make_unique<Expressions::FunctionCall>(name, std::move(expr_unwrapped), std::move(arguments));
        return Result<UniqExpression>(std::move(function_call_expr));
    }

    return expr_unwrapped;


    /*
    fn finish_call(&mut self, callee: &Rc<Expr>) -> Result<Expr, LoxResult> {
        let mut arguments = Vec::new();

        if !self.check(&RightParen) {
            arguments.push(Rc::new(self.expression()?));
            while match_token!(self, Comma) {
                if arguments.len() >= 255 {
                    self.error(&self.peek(), "You can't have more than 255 arguments.");
                }
                arguments.push(Rc::new(self.expression()?));
            }
        }

        let paren = self.consume(&RightParen, "Expect ')' after arguments.")?;

        Ok(Expr::Call(Rc::new(CallExpr {
            callee: Rc::clone(callee),
            paren,
            arguments,
        })))
    }
    */
}

auto Parser::primary() -> Result<UniqExpression> {
    spdlog::info(fmt::format("Parser: {}", __PRETTY_FUNCTION__));
    if (checkAndAdvance(TokenType::Number)) {
        auto number = std::make_unique<Expressions::Number>(previous().lexeme);
        return Result<UniqExpression>(std::move(number));
    }

    if (checkAndAdvance(TokenType::True, TokenType::False)) {
        auto bool_expression = std::make_unique<Expressions::Bool>(previous().lexeme);
        return Result<UniqExpression>(std::move(bool_expression));
    }

    if (checkAndAdvance(TokenType::Identifier)) {
        auto variable_expression = std::make_unique<Expressions::Variable>(previous());
        return Result<UniqExpression>(std::move(variable_expression));
    }

    return errorExpr(peek(), "Expect expression.");
}

