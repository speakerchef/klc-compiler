#pragma once

#include "include/utils.hpp"
#include "tokenizer.hpp"
#include <cstddef>
#include <type_traits>
#include <unordered_map>

struct NodeExit {
    int exit_code;
};

struct NodeIntVar {
    int value;
    std::string ident;
    bool is_mutable;
};

struct NodeIntLit {
    int value;
};


struct SyntaxNode {
  public:
    SyntaxNode(std::variant<NodeExit, NodeIntVar, NodeIntLit> node)
    : m_node(std::move(node)) {}
    SyntaxNode();

    [[nodiscard]] inline TokenType get_node_type() const {
        auto node_typer = Overload{
            [](NodeExit) { return TokenType::KW_EXIT; },
            [](NodeIntVar) { return TokenType::VAR_INT; },
            [](NodeIntLit) { return TokenType::LIT_INT; }
        };
        return std::visit(node_typer, m_node);
    }

    [[nodiscard]] inline std::variant<NodeExit, NodeIntVar> get_node_value(TokenType ttype) const {
        if (ttype == TokenType::VAR_INT) {
            return NodeIntVar({ 
                .value = std::get<NodeIntVar>(m_node).value, 
                .ident = std::get<NodeIntVar>(m_node).ident, 
                .is_mutable = std::get<NodeIntVar>(m_node).is_mutable, 
            });
        }
        else if (ttype == TokenType::KW_EXIT) {
            return NodeExit({ 
                .exit_code = std::get<NodeExit>(m_node).exit_code, 
            });
        }
        return {};
    }

    inline void set_node_value(const auto &ref) {
        auto vis_setter = [&](const auto &val) {
            if constexpr (std::is_same_v<decltype(val), NodeIntVar>) {
                m_node = { .value = val.value, .ident = val.ident, .is_immutable = val.is_immutable }; 
            } else if constexpr (std::is_same_v<decltype(val), NodeExit>) {
                m_node = { .exit_code = val.exit_code }; 
            };
        };
        std::visit(vis_setter, ref);
    }

  private:
    std::variant<NodeExit, NodeIntVar, NodeIntLit> m_node;

};

struct NodeExpr {
    SyntaxNode atom{};
    std::unordered_map<std::string, NodeExpr> exprP{};
};

class SyntaxTree {
  public:
    inline void push_node(SyntaxNode node) {
        switch (node.get_node_type()) {
            case TokenType::VAR_INT: {
                NodeIntVar n = std::get<NodeIntVar>(node.get_node_value(TokenType::VAR_INT));
                m_nodes.insert({
                    n.ident,
                    SyntaxNode(n)
                });
                break;
            }
        }
    }
    [[nodiscard]] inline std::optional<SyntaxNode> get_node(const std::string &ident) const {
        return m_nodes.at(ident);
    }

  private:
    std::unordered_map<std::string, SyntaxNode> m_nodes;

};
