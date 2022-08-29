#pragma once

#include "environment.h"
#include "resolver.h"
#include "scanner.h"
#include "parser.h"
#include "emitter.h"

struct Compiler {
    Compiler() = default;

    int run(int ac, char **av) {
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


        // @todo if parser is OK
        auto resolver = Resovler::Resolver(locals);

        spdlog::info("Resolver done");
        for (const auto &[expr, depth] : locals) {
            fmt::print("[ {}  --  {} ]\n", expr->to_string(), depth);
        }

        resolver.resolve(statements);

        e.emit(statements);

        return 0;
    }


private:
    Environment environment;
    Environment globals;
    std::unordered_map<Expressions::Expression *, std::size_t> locals;
};
