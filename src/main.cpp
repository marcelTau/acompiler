#include <fstream>
#include <iostream>
#include <fmt/core.h>
#include "parser.h"
#include "scanner.h"
#include "printer.h"
#include "emitter.h"
#include "spdlog/spdlog.h"

int main(int ac, char **av) {
    spdlog::info("Compiler started");
    Scanner s;
    Parser p;
    Emitter::Emitter e("testoutput.asm");

    if (ac == 1) {
        fmt::print(stderr, "No file provided\n");
        return 1;
    }

    std::ifstream file(av[1]);

    if (!file.is_open()) {
        spdlog::error(fmt::format("Cannot open file '{}'", av[1]));
        return 1;
    }

    std::string line;
    std::string content;

    spdlog::info(fmt::format("Start reading file '{}'", av[1]));
    while (std::getline(file, line)) {
        content += line + '\n';
    }

    auto tokens = s.scan(content);
    auto statements = p.parse(tokens);

    for (const auto& statement : statements) {
        fmt::print("{}\n", statement->to_string());
    }

    e.emit(statements);

    return 0;
}
