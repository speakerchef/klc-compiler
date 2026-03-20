#include <cctype>
#include <concepts>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <print>
#include <ranges>
#include <string_view>
#include <vector>

using std::println;
using std::string;

template <std::ranges::range R>
    requires(!std::convertible_to<R, std::string_view>)
// print range
void rprintln(string fmt, R &r) {
    std::copy(r.begin(), r.end(),
              std::ostream_iterator<string>(std::cout, ", "));
    std::cout << fmt << std::endl;
}

enum TokenType { _KEYWORD, _STR_LIT, _INT_LIT };

typedef struct Token {
    TokenType type;
    std::variant<string, int> value;
} Token;

// Constants
constexpr int PROC_FAIL = 1;

constexpr std::string_view
    ERR_ARGS("Error: Please supply arguments: klc <FILE.knv>");
constexpr std::string_view ERR_FILE("Error: Could not open file.");

int main(int argc, char **argv) {
    if (argc < 2 || argc > 3) {
        println(stderr, ERR_ARGS);
        return PROC_FAIL;
    }

    std::string raw_str;
    std::vector<string> word_buffer{};
    string path = argv[1];
    std::ifstream file(path);

    if (!file.is_open()) {
        println(stderr, ERR_FILE);
        return PROC_FAIL;
    }

    const std::string o_asm_path = "./gen_asm.s";
    char c;
    std::ofstream o_stream(o_asm_path);
    o_stream << ".global _start\n.align 4\n\t_start:\n";

    // Tokenize
    // replace this with proper character traversal
    Token tok{};

    o_stream << "\n\tMOV x0, 69\nBL _exit";
    o_stream.close();

    const char *assemble_cmd = "as -o ./gen_asm.o ./gen_asm.s";
    const char *linker_cmd =
        "ld -lSystem -syslibroot `xcrun -sdk macosx --show-sdk-path` -e "
        "_start -o ../build/bin/gen_asm ./gen_asm.o";
    const char *exec_cmd = "../build/bin/gen_asm; echo $?";
    std::system(assemble_cmd);
    std::system(linker_cmd);
    std::system(exec_cmd);
    // rprintln("", word_buffer);
}
