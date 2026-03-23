#pragma once

#include "include/utils.hpp"
#include "tokenizer.hpp"
#include <cstddef>
#include <cstdlib>
#include <type_traits>
#include <unordered_map>

struct NodeExit {
    int exit_code;
};

struct NodeLet {};

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
    SyntaxNode(std::variant<NodeExit, NodeLet, NodeIntVar, NodeIntLit> &&node)
    : m_node(std::move(node)) {}
    SyntaxNode();

    [[nodiscard]] inline TokenType get_node_type() const {
        auto node_typer = Overload {
            [](NodeExit)   { return TokenType::KW_EXIT; },
            [](NodeIntVar) { return TokenType::VAR_INT; },
            [](NodeIntLit) { return TokenType::LIT_INT; },
            [](NodeLet)    { return TokenType::KW_LET;  },
        };
        return std::visit(node_typer, m_node);
    }

    [[nodiscard]] inline auto get_node_value() const {
        auto getter_v = Overload {
            [&](const NodeExit)   { return m_node; },
            [&](const NodeLet)    { return m_node; },
            [&](const NodeIntVar) { return m_node; },
            [&](const NodeIntLit) { return m_node; },
        };
        return std::visit(getter_v, m_node);
    }

    inline void set_node_value(auto &&node_val) {
        auto setter_v = [&](auto &&val) {
            using decay_t = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<decay_t, NodeExit>) {
                m_node = { .exit_code = val.exit_code }; 
            }
            else if constexpr (std::is_same_v<decay_t, NodeLet>) {
                m_node = {}; 
            }
            else if constexpr (std::is_same_v<decay_t, NodeIntVar>) {
                m_node = { 
                    .value = val.value, 
                    .ident = std::move(val.ident), 
                    .is_immutable = val.is_immutable 
                }; 
            } 
            else if constexpr (std::is_same_v<decay_t, NodeIntLit>) {
                m_node = { .value = val.value }; 
            }
        };
        std::visit(setter_v, std::move(node_val));
    }

  private:
    std::variant<NodeExit, NodeLet, NodeIntVar, NodeIntLit> m_node;

};

struct NodeExpr {
    SyntaxNode atom{};
    std::unordered_map<std::string, NodeExpr> exprP{};
};

class SyntaxTree {
  public:
    inline void push_node(SyntaxNode &&node) {
        switch (node.get_node_type()) {
            case TokenType::VAR_INT: {
                auto n = std::get<NodeIntVar>(node.get_node_value());
                m_var_table.insert({
                    std::move(n.ident),
                    SyntaxNode(n)
                });
                m_called_nodes.emplace_back(std::move(node));
                break;                    
            }                             
            case TokenType::KW_LET:                             
            case TokenType::DELIM_SEMI:
            case TokenType::KW_RETURN:
            case TokenType::KW_EXIT:
            case TokenType::KW_INT:
            case TokenType::LIT_INT:                              
            case TokenType::LIT_STR:
            case TokenType::UNCLASSED_VAR_DEC:
            case TokenType::OP_EQUALS:
            case TokenType::OP_PLUS:
            case TokenType::OP_MINUS: {
                m_called_nodes.emplace_back(std::move(node));
                break;
            }
        }
    }
    [[nodiscard]] inline std::optional<SyntaxNode> lookup_node(TokenType ttype, const std::string ident = "") const {
        if (ttype == TokenType::VAR_INT ) {
            if (ident.empty()) { 
                std::println(stderr, "Error: Identifier required.");
                exit(EXIT_FAILURE);
            }
            return m_var_table.at(ident);
        } 
        return {};
    }

    [[nodiscard]] inline std::vector<SyntaxNode> get_called_nodes() const {
        return m_called_nodes;
    }
    [[nodiscard]] inline std::unordered_map<std::string, SyntaxNode> get_var_table() const {
        return m_var_table;
    }


  private:
    std::vector<SyntaxNode> m_called_nodes{};
    std::unordered_map<std::string, SyntaxNode> m_var_table;
};
