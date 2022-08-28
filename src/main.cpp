#include <fstream>
#include <iostream>
#include <fmt/core.h>
#include "parser.h"
#include "scanner.h"
#include "printer.h"
#include "emitter.h"

int main(int ac, char **av) {
    Scanner s;
    Parser p;
    Emitter::Emitter e;

    if (ac == 1) {
        fmt::print(stderr, "No file provided\n");
        return 1;
    }

    std::ifstream file(av[1]);

    if (! file.is_open()) {
        return 1;
    }

    std::string line;
    std::string content;

    while (std::getline(file, line)) {
        content += line;
    }

    auto tokens = s.scan(content);
    auto statements = p.parse(tokens);

    for (const auto& statement : statements) {
        fmt::print("Statement: {}\n", statement->to_string());
    }

    e.emit(statements);

    return 0;
}
