#include "include/utils.hpp"
#include "tokenizer.cpp"

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

    std::ofstream o_stream{};
    o_stream << ".global _start\n.align 4\n\t_start:\n";

    // Tokenize
    // replace this with proper character traversal
    Tokenizer tokenizer{};

    std::vector<Token> tokens = tokenizer.tokenize(std::move(file));

    auto visitor = [](const auto &value) { println("Token value: {}", value); };
    for (const Token &tok : tokens) {
        std::visit(visitor, tok.value);
    }

    o_stream << "\n\tMOV x0, 69\nBL _exit";
    o_stream.close();

    const char *assemble_cmd = "as -o ./gen_asm.o ./gen_asm.s";
    const char *linker_cmd =
        "ld -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path` -e "
        "_start -o ../build/bin/gen_asm ./gen_asm.o";
    const char *exec_cmd = "../build/bin/gen_asm; echo $?";

    // std::system(assemble_cmd);
    // std::system(linker_cmd);
    // std::system(exec_cmd);
    // rprintln("", word_buffer);
}
