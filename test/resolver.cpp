#include <gtest/gtest.h>

#include "parser.h"
#include "scanner.h"
#include "resolver.h"
#include "environment.h"

using namespace Resovler;

TEST(resolver, resolve_one_variable) {
    Scanner s;
    Parser p;
    Resolver r;

    auto tokens = s.scan(R"(
fun main() -> Int
    let x: Int = 10;
    return x;
end
)");

    auto stmts = p.parse(tokens);

    auto res = r.resolve(stmts);

    EXPECT_EQ(res.size(), 1);
}

TEST(resolver, resolve_multiple_variable) {
    Scanner s;
    Parser p;
    Resolver r;

    auto tokens = s.scan(R"(
fun main() -> Int
    let x: Int = 10;
    return x;
end

fun other() -> Int
    let y: Int = 20;
    return y;
end
)");

    auto stmts = p.parse(tokens);

    auto res = r.resolve(stmts);
    ValuePrintVisitor printer;

    EXPECT_EQ(res.size(), 2);
}

