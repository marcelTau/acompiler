#include <cstdlib>
#include <gtest/gtest.h>
#include <sstream>
#include "parser.h"
#include "scanner.h"
#include "emitter.h"
#include "resolver.h"

static constexpr const char * const bin_path = "/tmp/asm_output";

std::string run_file(std::string_view filepath) {
    std::string out_path { filepath };
    out_path += ".tmp";

    std::system(fmt::format("nasm -felf64 {0}.asm && ld {0}.o -o {1}", filepath, bin_path).c_str());
    std::system(fmt::format("{} ; echo $? > {}", bin_path, out_path).c_str());
    
    std::stringstream ss;
    ss << std::ifstream(out_path).rdbuf();

    auto ret = ss.str();

    if (ret.ends_with('\n')) {
        ret = ret.substr(0, ret.size() - 1);
    }
    return ret;
}

TEST(emitter, return_single) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    return 12;
end
)";

    e.emit(p.parse(s.scan(code)));

    EXPECT_EQ(run_file(filepath), "12");
}

TEST(emitter, return_addition) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    return 12 + 5;
end
)";

    e.emit(p.parse(s.scan(code)));
    EXPECT_EQ(run_file(filepath), "17");
}

TEST(emitter, return_subtraction) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    return 12 - 5;
end
)";

    e.emit(p.parse(s.scan(code)));
    EXPECT_EQ(run_file(filepath), "7");
}

TEST(emitter, return_add_sub) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    return 1 + 12 - 5 + 10;
end
)";

    e.emit(p.parse(s.scan(code)));
    EXPECT_EQ(run_file(filepath), "18");
}

TEST(emitter, return_mixed) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    return 2 + 3 * 3 - 1;
end
)";

    e.emit(p.parse(s.scan(code)));
    EXPECT_EQ(run_file(filepath), "10");
}

TEST(emitter, return_division) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    return 15 / 5;
end
)";

    e.emit(p.parse(s.scan(code)));
    EXPECT_EQ(run_file(filepath), "3");
}

TEST(emitter, return_precedence_check) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    return 15 + 5 * 2 - 9 / 3;
end
)";

    e.emit(p.parse(s.scan(code)));
    EXPECT_EQ(run_file(filepath), "22");
}

TEST(emitter, return_single_variable) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 10;
    return x;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "10");
}

TEST(emitter, return_single_variable_plus_number) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 10;
    return x + 10;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "20");
}

TEST(emitter, return_single_variable_with_precednece_check) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 10;
    return x + 10 * 2;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "30");
}

TEST(emitter, return_multiple_variables_simple) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 10;
    let y: Int = 7;
    return x + y;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "17");
}

TEST(emitter, return_multiple_variables_precedence) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 10;
    let y: Int = 8;
    let z: Int = 2;
    return x + y / 2;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "14");
}

TEST(emitter, return_variable_with_assignment) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 10;
    x = 20;
    return x;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "20");
}

TEST(emitter, return_variable_with_multiple_assignments) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 10;
    x = 20;
    x = 33;
    return x;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "33");
}

TEST(emitter, return_variable_with_multiple_assignments_with_addition) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 10;
    x = 20 + 8;
    return x;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "28");
}

TEST(emitter, return_incremented_variable) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 10;
    x = x + 1;
    return x;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "11");
}

TEST(emitter, return_multiple_variables_assignment_addition) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 10;
    let y: Int = 5;
    return x + y;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "15");
}

TEST(emitter, return_multiple_variables_assignment_multiplication) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 10;
    let y: Int = 5;
    return x * y;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "50");
}

TEST(emitter, return_multiple_variables_assignment_with_precedence_check) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 10;
    let y: Int = 5;
    y = 2;
    return x + 2 * y;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "14");
}

TEST(emitter, return_variable_assignment_with_other_variable) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 10;
    let y: Int = 5;
    x = x + y + 5;
    return x;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "20");
}

TEST(emitter, return_comparison_equal_equal_true) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    return 1 == 1;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "1");
}

TEST(emitter, return_comparison_equal_equal_false) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    return 1 == 2;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "0");
}

TEST(emitter, return_comparison_bang_equal_true) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    return 1 != 2;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "1");
}

TEST(emitter, return_comparison_bang_equal_false) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    return 1 != 1;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "0");
}

TEST(emitter, return_comparison_equal_equal_false_with_variables) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 1;
    let y: Int = 2;
    return x == y;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "0");
}

TEST(emitter, return_comparison_bang_equal_true_with_variables) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 1;
    let y: Int = 2;
    return x != y;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "1");
}

TEST(emitter, return_comparison_less_true) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    return 1 < 2;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "1");
}

TEST(emitter, return_comparison_less_false) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    return 2 < 1;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "0");
}

TEST(emitter, return_comparison_less_true_with_variables) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 1;
    let y: Int = x + 1;
    return x < y;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "1");
}

TEST(emitter, return_comparison_less_false_with_variables) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 1;
    let y: Int = x + 1;
    return y < x;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "0");
}

TEST(emitter, return_comparison_less_equal_false_with_variables) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 1;
    let y: Int = x + 1;
    return y <= x;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "0");
}

TEST(emitter, return_comparison_less_equal_true_equal_with_variables) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 1;
    let y: Int = x;
    return y <= x;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "1");
}

TEST(emitter, return_comparison_less_equal_true_less_with_variables) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 1;
    let y: Int = x + 1;
    return x <= y;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "1");
}

TEST(emitter, return_comparison_greater_true_with_variables) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 1;
    let y: Int = x + 1;
    return y > x;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "1");
}

TEST(emitter, return_comparison_greater_false_with_variables) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 1;
    let y: Int = x + 1;
    return x > y;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "0");
}

TEST(emitter, return_comparison_greater_equal_false_with_variables) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 1;
    let y: Int = x + 1;
    return x >= y;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "0");
}

TEST(emitter, return_comparison_greater_equal_true_equal_with_variables) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 1;
    let y: Int = x;
    return y >= x;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "1");
}

TEST(emitter, return_comparison_greater_equal_true_greater_with_variables) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 1;
    let y: Int = x + 1;
    return y >= x;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "1");
}

TEST(emitter, if_statement_simple_numbers_true) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 5;
    if 1 == 1 then
        x = x + 2;
    end
    return x;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "7");
}

TEST(emitter, if_statement_simple_numbers_false) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 5;
    if 1 == 2 then
        x = x + 2;
    end
    return x;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "5");
}

TEST(emitter, if_statement_simple_vars_false) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 5;
    let y: Int = 10;
    if x == y then
        x = x + 2;
    end
    return x;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "5");
}

TEST(emitter, if_statement_simple_vars_true) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 5;
    let y: Int = 2 + 3;
    if x == y then
        x = x + 2;
    end
    return x;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "7");
}

TEST(emitter, if_statement_simple_with_boolean_value_true) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 5;
    let is_true: Bool = true;
    if is_true then
        x = x + 2;
    end
    return x;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "7");
}

TEST(emitter, if_statement_simple_with_boolean_value_false) {
    std::string filepath(std::string("/tmp/") + test_info_->test_case_name());
    auto filepathasm = filepath + ".asm";
    Scanner s;
    Parser p;
    Resolver r;
    Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    let x: Int = 5;
    let is_false: Bool = false;
    if is_false then
        x = x + 2;
    end
    return x;
end
)";

    auto tokens = s.scan(code);
    auto stmts = p.parse(tokens);
    r.resolve(stmts);
    e.emit(stmts);
    EXPECT_EQ(run_file(filepath), "5");
}
