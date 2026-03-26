#pragma once

#include "syntax-tree.hpp"
#include <cstddef>
#include <fstream>

enum class Marker{
    END
};

class CodeGenerator {
  public:
    CodeGenerator(NodeProgram&& prog) noexcept;
    ~CodeGenerator();

    void emit();

template<typename T>
[[nodiscard]] inline T eval_expr(NodeBinaryExpr& node) {
    std::string ident;
    auto lres = 0;
    auto rres = 0;
    node.print();

    if (!std::isdigit(node.atom.front()) && !node.atom.empty()) {
            std::println("IDENT AT EVAL EXPR: {}", node.atom);
        ident = node.atom;
        auto& id_node = std::get<NodeBinaryExpr>(m_program.lookup_node(ident)->m_node);
        auto lhs = std::get<NodeBinaryExpr>(m_program.lookup_node(ident)->m_node).lhs.get();
        auto rhs = std::get<NodeBinaryExpr>(m_program.lookup_node(ident)->m_node).rhs.get();

        // id_node.print();
        // if (lhs) res = eval_expr<int>(*lhs);
        // if (rhs) res = eval_expr<int>(*rhs);

    }
    if (!node.lhs && !node.rhs) {
        return std::stoi(node.atom);
    }

    if (node.lhs) lres = eval_expr<int>(*node.lhs);
    if (node.rhs) rres = eval_expr<int>(*node.rhs);
    
    switch (node.op) {
        case BinOp::ADD:  return lres + rres;
        case BinOp::SUB:  return lres - rres;
        case BinOp::MULT: return lres * rres;
        case BinOp::DIV:  return lres / rres;
    }
}

  private:
    std::ofstream m_os;
    size_t m_node_ptr = 0;
    size_t m_stack_sz = 0;
    NodeProgram m_program;

    //===================================

    [[nodiscard]] const SyntaxNode* peek(const size_t offset) const;
    [[maybe_unused]] const SyntaxNode* next();
    [[nodiscard]] std::string gen_var_declaration();

    // [[nodiscard]] std::string eval_expr(SyntaxNode& node, const std::string& ident);

};
