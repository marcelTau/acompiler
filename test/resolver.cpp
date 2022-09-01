#include <gtest/gtest.h>

#include "error.h"
#include "parser.h"
#include "scanner.h"
#include "resolver.h"
#include "environment.h"

TEST(resolver, resolver_error_use_variable_in_its_own_initialier) {
    Scanner s;
    Parser p;
    Resolver r;

    auto tokens = s.scan(R"(
fun main() -> Int
    let x: Int = x;
    return x;
end
)");

    auto stmts = p.parse(tokens);

    bool is_ok { false };

    try {
        r.resolve(stmts);
    } catch (const Error& e) {
        is_ok = e.msg == "Can't read variable 'x' in it's own initializer.";
    }

    EXPECT_TRUE(is_ok);
}

TEST(resolver, resolver_error_variable_redefinition) {
    Scanner s;
    Parser p;
    Resolver r;

    auto tokens = s.scan(R"(
fun main() -> Int
    let x: Int = 1;
    let x: Int = 2;
    return x;
end
)");

    auto stmts = p.parse(tokens);

    bool is_ok { false };

    try {
        r.resolve(stmts);
    } catch (const Error& e) {
        is_ok = e.msg == "Variable 'x' already exists in this scope.";
    }

    EXPECT_TRUE(is_ok);
}

TEST(resolver, resolver_error_return_outside_of_function) {
    Scanner s;
    Parser p;
    Resolver r;

    auto tokens = s.scan(R"(
return;
)");

    auto stmts = p.parse(tokens);

    bool is_ok { false };

    try {
        r.resolve(stmts);
    } catch (const Error& e) {
        is_ok = e.msg == "Can't return outside of function.";
    }

    EXPECT_TRUE(is_ok);
}
