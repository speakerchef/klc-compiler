#pragma once

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
[[nodiscard]] inline T eval_expr(NodeBinaryExpr& node) {
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
        if (lhs) lres = eval_expr<int>(*lhs);
        if (rhs) rres = eval_expr<int>(*rhs);

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
                std::println(stderr, "Goofy, this operator is not supported");
                exit(EXIT_FAILURE);
            }
        }
        
    }
    if (!node.lhs && !node.rhs) {
        m_cached_var[ident] = node.atom; 
        return std::stoi(node.atom);
    }

    if (node.lhs) lres = eval_expr<int>(*node.lhs);
    if (node.rhs) rres = eval_expr<int>(*node.rhs);
    
    switch (node.op) {
        case BinOp::ADD:  return lres + rres;
        case BinOp::SUB:  return lres - rres;
        case BinOp::MULT: return lres * rres;
        case BinOp::DIV:  return lres / rres;
    default: assert(false && "Unknown operator!");

    }
}

  private:
    std::ofstream m_os;
    size_t m_node_ptr = 0;
    size_t m_stack_sz = 0;
    NodeProgram m_program;
    std::unordered_map<std::string, std::string> m_cached_var;

    //===================================

    [[nodiscard]] const SyntaxNode* peek(size_t offset) const;
    [[maybe_unused]] const SyntaxNode* next();
    [[nodiscard]] std::string gen_var_declaration();
};
