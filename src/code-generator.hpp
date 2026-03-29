#include "syntax-tree.hpp"
#include <cstddef>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

enum class Marker{
    END
};

class CodeGenerator {
  public:
     explicit CodeGenerator(NodeProgram&& prog) noexcept;
    ~CodeGenerator();

    void emit();

template<typename T>
[[nodiscard]] inline T consteval_expr(NodeBinaryExpr& node) { // use only for compile time eval
    std::string ident;
    auto lres = 0;
    auto rres = 0;
    // node.print();

    if (!std::isdigit(node.atom.front()) && !node.atom.empty()) {
        // std::println("IDENT AT EVAL EXPR: {}", node.atom);
        ident = node.atom;
        const auto & id_node = std::get<NodeBinaryExpr>(m_program.lookup_node(ident)->m_node);
        const auto lhs = id_node.lhs.get();
        const auto rhs = id_node.rhs.get();

        // id_node.print();
        if (lhs) lres = consteval_expr<int>(*lhs);
        if (rhs) rres = consteval_expr<int>(*rhs);

        switch (id_node.op) {
            case BinOp::ADD: {
                node.atom = std::to_string(lres + rres);
                break;
            }
            case BinOp::SUB: {
                node.atom = std::to_string(lres - rres);
                break;
            }
            case BinOp::MULT: {
                node.atom = std::to_string(lres * rres);
                break;
            }
            case BinOp::DIV: {
                node.atom = std::to_string(lres / rres);
                break;
            }
            default: {
                assert(false && "Unknown operator!");
            }
        }
        
    }
    if (!node.lhs && !node.rhs) { return std::stoi(node.atom); }

    if (node.lhs) lres = consteval_expr<int>(*node.lhs);
    if (node.rhs) rres = consteval_expr<int>(*node.rhs);
    
    switch (node.op) {
        case BinOp::ADD:  return lres + rres;
        case BinOp::SUB:  return lres - rres;
        case BinOp::MULT: return lres * rres;
        case BinOp::DIV:  return lres / rres;
        default: assert(false && "Unknown operator!");
    }
}
    [[nodiscard]] int emit_expr(const NodeBinaryExpr&node);
    void emit_decl(const NodeVarDeclaration& node);

  private:
    std::ofstream m_os;
    size_t m_node_ptr = 0;
    size_t m_stack_sz = 0;
    int32_t m_stack_ptr = 0;
    bool expand_stack = true;
    NodeProgram m_program;
    std::unordered_map<std::string, int32_t> m_cached_var;

    //===================================

    [[nodiscard]] const SyntaxNode* peek(size_t offset) const;
    [[maybe_unused]] const SyntaxNode* next();
    [[nodiscard]] std::string gen_var_declaration();
};
