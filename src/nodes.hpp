#pragma once

#include "include/utils.hpp"
#include "tokenizer.hpp"
#include <variant>

struct NodeExit {
    int exit_code;
};

struct SyntaxNode {
  private:
    std::variant<NodeExit> m_node;

  public:
    SyntaxNode(const std::variant<NodeExit> &node)
    : m_node(node) {}
        
    [[nodiscard]] inline TokenType get_node_type() const {
        auto node_typer = Overload{
            [](NodeExit) { return TokenType::KW_EXIT; },
        };

        return std::visit(node_typer, m_node);
    }
    [[nodiscard]] inline auto get_node_value() const {
        auto node_getter = Overload {
            [](const NodeExit n) { 
                return n.exit_code;
            },
        };

        return std::visit(node_getter, m_node);
    }
};
