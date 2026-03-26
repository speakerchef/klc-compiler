#include "../src/parser.hpp"
#include "code-generator.hpp"
#include "lexer.hpp"
#include "syntax-tree.hpp"
#include <cassert>
#include <cstddef>
#include <functional>
#include <memory>
#include <vector>

int main() {
    const std::string path("../tests/CodeGeneratorTests.knv");
    Lexer lex(path);
    Parser parser{ lex.tokenize() };
    NodeProgram prog = parser.create_program();
    std::vector<std::reference_wrapper<NodeBinaryExpr>> res;
    for (auto& node : prog.main) {
        res.emplace_back(std::get<NodeBinaryExpr>(std::get<NodeVarDeclaration>(node.m_node).value->m_node));
    }
    CodeGenerator gen(std::move(prog));

    constexpr auto GREEN    = "\033[92m";
    constexpr auto RED      = "\033[31m";
    constexpr auto CYAN     = "\033[36m";
    constexpr auto BOLD     = "\033[1m";
    constexpr auto RESET    = "\033[0m";
    constexpr auto B_YELLOW = "\033[1;33m";

    auto run_test = [&](const int tnum, int expected) {
        auto t1_pass = std::format("{}Test {} Passed!{}", GREEN, tnum, RESET);
        auto t1_fail = std::format("{}Test {} Failed!{}", RED, tnum, RESET);

        auto t = gen.eval_expr<int>( res.at(tnum - 1) );

        std::println("{}{}Test {}, Result = {} : Expected ({})", CYAN, BOLD, tnum, t, expected);
        std::println("{}", (t == expected) ? t1_pass : t1_fail);
    };
    std::println("{}========= [TEST] CodeGenerator::eval_expr() ========={}", B_YELLOW, RESET);
    run_test(1, 29);
    run_test(2, 34);
    run_test(3, 5);
    run_test(4, 25);

    return 0;
}
