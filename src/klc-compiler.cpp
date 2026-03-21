#include "parser.hpp"
#include "include/utils.hpp"

#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <print>
#include <vector>

using std::println;
using std::string;

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        println(stderr, ERR_ARGS);
        return EXIT_FAILURE;
    }

    string path = argv[1];
    std::ifstream file(path);

    if (!file.is_open()) {
        println(stderr, ERR_FILE);
        return EXIT_FAILURE;
    }

    std::ofstream o_stream("./gen_asm.s");
    o_stream << ".global _start\n.align 4\n_start:\n";

    auto visitor = [](const auto &value) { println("Token value: {}", value); };
    
    // Process
    Tokenizer tokenizer{};
    std::vector<Token> tokens = tokenizer.tokenize(std::move(file));
    for (const Token &tok : tokens) {
        std::visit(visitor, tok.value);
    }

    Parser parser(tokens, o_stream); 

    o_stream.close();
    println("Successful Compilation!");

    const char *assemble_cmd = "as -o ./gen_asm.o ./gen_asm.s";
    const char *linker_cmd =
        "ld -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path` -e "
        "_start -o ../build/gen_asm ./gen_asm.o";
    const char *exec_cmd = "../build/gen_asm; echo $?";

    std::system(assemble_cmd);
    std::system(linker_cmd);
    std::system(exec_cmd);
}
