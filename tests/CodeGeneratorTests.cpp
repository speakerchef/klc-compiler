#include "../src/parser.hpp"
#include "code-generator.hpp"
#include "lexer.hpp"
#include "syntax-tree.hpp"
#include <memory>
#include <vector>

int main() {
    const std::string path("./CodeGeneratorTests.knv");

    Lexer lex(path);
    Parser parser{ lex.tokenize() };
    NodeProgram prog = parser.create_program();
    auto& res = std::get<NodeBinaryExpr>(std::get<NodeVarDeclaration>(prog.main.at(0).m_node).value->m_node);
    // res.print();
    CodeGenerator gen(std::move(prog));

    auto res2 = gen.eval_expr<int>(res);
    std::println("RESULT: {}", res2);
    return 0;
}
