#pragma once

#include "include/utils.hpp"
#include "tokenizer.hpp"
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <unordered_map>

/*
 * Syntax Examples:
 * case 1: let x = 2;
 * case 2: let y = 5 + 5; :10
 * case 3: let z = 5 + (x * 4); :13
 *
 */

struct SyntaxNode;
struct NodeExpr;

enum class BinOp {
    ADD,
    SUB,
    MULT,
    DIV,
};

struct NodeBinaryExpr { // eg. a + b or 4 * 5
    std::unique_ptr<NodeExpr> left;
    std::unique_ptr<NodeExpr> right;
    BinOp op;
};

// struct NodeUnaryExpr {
//     std::unique_ptr<NodeExpr> right; // right recursed
//     BinOp op;
// };

struct NodeIdentifier {
    std::string name; // used for variable lookup
};

// Variables
struct NodeVarDeclarator {
    NodeIdentifier ident;
    std::unique_ptr<NodeExpr> value; // variable value; can be expr or literal
};
struct NodeVarDeclaration {
    std::string kind;             // let, const
    NodeVarDeclarator declarator; // literal, expression
};

struct NodeIntLiteral {
    int value;
};

struct NodeStmtExit {
    std::unique_ptr<NodeExpr> exit_code;
};

struct SyntaxNodev3 {
    std::variant<std::unique_ptr<NodeExpr>, NodeBinaryExpr, NodeIdentifier,
                 NodeVarDeclarator, NodeVarDeclaration, NodeIntLiteral,
                 NodeStmtExit>
        node;
};

struct NodeExpr {
    std::unique_ptr<SyntaxNode> expr; // Literal or expression
};

struct SyntaxNode {
  public:
    std::variant<std::unique_ptr<NodeExpr>, NodeBinaryExpr, 
    NodeIdentifier, NodeVarDeclarator, NodeVarDeclaration, 
    NodeIntLiteral, NodeStmtExit> m_node;

    //====================================//
    ~SyntaxNode() = default;
    [[nodiscard]] TokenType get_node_type() const;
    inline void set_node_value(auto &&node_val);
    //====================================//
};

class SyntaxTree {
  public:
    //====================================//
    void push_node(SyntaxNode &&node);
    [[nodiscard]] inline auto& lookup_node(const std::string& ident) const;
    [[nodiscard]] inline const std::vector<SyntaxNode>& get_called_nodes();
    [[nodiscard]] inline const std::unordered_map<std::string, NodeExpr>& get_var_table() const;
    [[nodiscard]] inline const SyntaxNode& peek() const;
    //====================================//

  private:
    std::vector<SyntaxNode> m_called_nodes{};
    std::unordered_map<std::string, NodeExpr> m_var_table;
};
