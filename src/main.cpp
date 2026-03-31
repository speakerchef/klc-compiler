#include "code-generator.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "include/utils.hpp"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <print>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        println(stderr, ERR_ARGS);
        return EXIT_FAILURE;
    }

    const std::string path = argv[1];
    const std::string exec_name = argv[2];

    if (std::ifstream file(path); !file.is_open()) {
        println(stderr, ERR_FILE);
        return EXIT_FAILURE;
    }

    // Process
    std::println("Tokenizing...");
    Lexer lexer{ path };
    std::println("Parsing...");
    Parser parser { lexer.tokenize() };
    std::println("Generating...");
    CodeGenerator gen{ parser.create_program(), exec_name };

    const string assemble_cmd = std::format("clang -c -g -Wno-missing-sysroot -o a.o ./{}.s"
    " && ld -lSystem -syslibroot `xcrun --sdk macosx --show-sdk-path` -o {} ./a.o", exec_name, exec_name);

    const int ecode = (std::system(assemble_cmd.c_str()));
    if (ecode) {
        std::println(stderr, "Error: Could not compile!"); 
        return ecode;
    }
    std::filesystem::remove("./a.o");
    std::println("Successful Compilation!");
    std::println("Generated assembly at {}/{}.s", std::filesystem::current_path().string(), exec_name);
    std::println("Executable at {}/{}", std::filesystem::current_path().string(), exec_name);

    return 0;
}
