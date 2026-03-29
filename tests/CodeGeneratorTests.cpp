#include "../src/parser.hpp"
#include "code-generator.hpp"
#include "lexer.hpp"
#include "syntax-tree.hpp"
#include <cassert>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>

int main() {
    const std::string path("../tests/CodeGeneratorTests.knv");
    Lexer lex(path);
    Parser parser{ lex.tokenize() };
    NodeProgram prog = parser.create_program();
    std::vector<std::reference_wrapper<NodeBinaryExpr>> bin_expr;
    std::vector<std::reference_wrapper<NodeVarDeclaration>> decl;
    bin_expr.reserve(20);
    decl.reserve(20);
    for (auto& node : prog.main) {
        bin_expr.emplace_back(std::get<NodeBinaryExpr>(std::get<NodeVarDeclaration>(node.m_node).value->m_node));
        decl.emplace_back(std::get<NodeVarDeclaration>(node.m_node));
    }
    CodeGenerator gen(std::move(prog));

    constexpr auto GREEN    = "\033[92m";
    constexpr auto RED      = "\033[31m";
    constexpr auto CYAN     = "\033[36m";
    constexpr auto BOLD     = "\033[1m";
    constexpr auto RESET    = "\033[0m";
    constexpr auto B_YELLOW = "\033[1;33m";
    int success_count = 0;
    int total_count = 0;

    auto t_consteval_expr= [&](const size_t tnum, const int expected) {
        auto t1_pass = std::format("{}Test {} Passed!{}", GREEN, tnum, RESET);
        auto t1_fail = std::format("{}Test {} Failed!{}", RED, tnum, RESET);

        try {
            auto t = gen.consteval_expr<int>( bin_expr.at(tnum - 1) );
            std::println("{}{}Test {}, Result = {} : Expected ({})", CYAN, BOLD, tnum, t, expected);
            if (t == expected) success_count++;
            total_count++;
            std::println("{}", (t == expected) ? t1_pass : t1_fail);
        }
        catch (std::exception& e) {
        std::println(stderr, "Exception: {}", e.what());
        }
    };
    // std::println("{}========= [TEST] CodeGenerator::eval_expr() ========={}", B_YELLOW, RESET);
    // t_consteval_expr(1, 0);
    // t_consteval_expr(2, 5);
    // t_consteval_expr(3, 5);
    // t_consteval_expr(4, 30);
    // t_consteval_expr(5, 25);
    // t_consteval_expr(6, 16);
    // std::println("{}Total Passed: {}/{}{}", success_count != total_count ? B_YELLOW : GREEN, success_count, total_count, RESET);

    auto t_emit_decl = [&](const size_t tnum, const int expected) {
        auto t1_pass = std::format("{}Test {} Passed!{}", GREEN, tnum, RESET);
        auto t1_fail = std::format("{}Test {} Failed!{}", RED, tnum, RESET);

        try {
            // auto t = gen.emit_decl(decl.at( tnum - 1 ));
            gen.emit_decl(decl.at( tnum - 1 ));

            // std::println("{}{}Test {}, Result = {} : Expected ({})", CYAN, BOLD, tnum, t, expected);
            // if (t == expected) success_count++;
            // total_count++;
            // std::println("{}", (t == expected) ? t1_pass : t1_fail);
        }
        catch (std::exception& e) {
            std::println(stderr, "Exception: {}", e.what());
        }
    };
    std::println("{}========= [TEST] CodeGenerator::eval_expr() ========={}", B_YELLOW, RESET);
    t_emit_decl(1, 0);
    t_emit_decl(2, 5);
    // t_emit_decl(3, 5);
    // t_emit_decl(4, 30);
    // t_emit_decl(5, 25);
    // t_emit_decl(6, 16);
    std::println("{}Total Passed: {}/{}{}", success_count != total_count ? B_YELLOW : GREEN, success_count, total_count, RESET);
    return 0;
}
