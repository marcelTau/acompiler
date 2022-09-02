#pragma once

#include "error.h"
#include "resolver.h"
#include "scanner.h"
#include "parser.h"
#include "emitter.h"

#include <istream>
#include <iostream>

struct Compiler {
    Compiler() = default;

    int run(int ac, char **av) {
        spdlog::info("Compiler started");
        Scanner s;
        Parser p;

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


        Resolver resolver;

        try {
            resolver.resolve(statements);
        } catch (Error& e) {
            spdlog::error(fmt::format("Resolver failed on token: {}", e.token));
            return 1;
        }

        for (const auto& statement : statements) {
            fmt::print("{}\n", statement->to_string());
        }

        Emitter e("testoutput.asm");
        e.emit(statements);

        return 0;
    }
};
