#include "code-generator.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "include/utils.hpp"
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <print>

using std::println;
using std::string;

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        println(stderr, ERR_ARGS);
        return EXIT_FAILURE;
    }

    string path = argv[1];

    if (std::ifstream file(path); !file.is_open()) {
        println(stderr, ERR_FILE);
        return EXIT_FAILURE;
    }

    // Process
    Lexer lexer{ path };
    Parser parser { lexer.tokenize() };
    CodeGenerator gen{ parser.create_program() };

    println("Successful Compilation!");

    // const char *assemble_cmd = "as -o ../build/gen_asm.o ../build/gen_asm.s";
    // const char *linker_cmd =
    //     "ld -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path`"
    //     " -o ../build/gen_asm ../build/gen_asm.o";
    // const char *exec_cmd = "../build/gen_asm; echo $?";

    // std::system(assemble_cmd);
    // std::system(linker_cmd);
    // std::system(exec_cmd);
}
