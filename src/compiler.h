#pragma once

#include "environment.h"
#include "error.h"
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
        //auto resolver = Resovler::Resolver;

        Resovler::Resolver resolver;


        try {
            locals = resolver.resolve(statements);
        } catch (Error& e) {
            spdlog::error(fmt::format("Resolver failed on token: {}", e.token));
            return 1;
        }

        spdlog::info(fmt::format("Resolver done with {} locals", locals.size()));


        ValuePrintVisitor printer;

        for (const auto &[expr, depth] : locals) {
            fmt::print("[ {}  --  {} ]\n", std::visit(printer, expr), depth);
        }

        Emitter::Emitter e("testoutput.asm", locals);
        e.emit(statements);

        return 0;
    }


private:
    Environment environment;
    Environment globals;
    std::unordered_map<Value, std::size_t> locals {};
};
