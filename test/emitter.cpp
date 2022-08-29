#include <cstdlib>
#include <gtest/gtest.h>
#include <sstream>
#include "parser.h"
#include "scanner.h"
#include "emitter.h"

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
    Emitter::Emitter e(filepathasm);

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
    Emitter::Emitter e(filepathasm);

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
    Emitter::Emitter e(filepathasm);

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
    Emitter::Emitter e(filepathasm);

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
    Emitter::Emitter e(filepathasm);

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
    Emitter::Emitter e(filepathasm);

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
    Emitter::Emitter e(filepathasm);

    auto code = R"(
fun main() -> Int
    return 15 + 5 * 2 - 9 / 3;
end
)";

    e.emit(p.parse(s.scan(code)));
    EXPECT_EQ(run_file(filepath), "22");
}
