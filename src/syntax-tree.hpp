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
    SyntaxNode(std::variant<NodeExit, NodeIntVar, NodeIntLit, NodeLet> node)
    : m_node(std::move(node)) {}
    SyntaxNode();

    [[nodiscard]] inline TokenType get_node_type() const {
        auto node_typer = Overload{
            [](NodeExit) { return TokenType::KW_EXIT; },
            [](NodeIntVar) { return TokenType::VAR_INT; },
            [](NodeIntLit) { return TokenType::LIT_INT; },
            [](NodeLet) { return TokenType::KW_LET; },
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
    std::variant<NodeExit, NodeIntVar, NodeIntLit, NodeLet> m_node;

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
                m_var_table.insert({
                    n.ident,
                    SyntaxNode(n)
                });
                m_call_stack.emplace_back(node);
                break;
            }
            case TokenType::KW_LET: {
                m_call_stack.emplace_back(node);
                break;
            }
            case TokenType::LIT_INT: {
                m_call_stack.emplace_back(node);
                break;
            }
            case TokenType::KW_EXIT: {
                m_call_stack.emplace_back(node);
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
    // [[nodiscard]] inline std::optional<SyntaxNode> pop_stack_node() {
    //     if (!m_node_call_stack.empty()) {
    //         return m_node_call_stack.end();
    //     }
    // }

    [[nodiscard]] inline std::vector<SyntaxNode> get_call_stack() const {
        return m_call_stack;
    }
    [[nodiscard]] inline std::unordered_map<std::string, SyntaxNode> get_var_table() const {
        return m_var_table;
    }


  private:
    // DONE: Fix by using call stack for SyntaxNodes to be walked.
    // DONE: Change map to contain only variables
    // TODO: Verify get_node_value() function
    std::vector<SyntaxNode> m_call_stack{};
    std::unordered_map<std::string, SyntaxNode> m_var_table;
};
