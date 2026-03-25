#pragma once

#include "syntax-tree.hpp"
#include <concepts>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <print>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>

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

enum class NodeType {
    SYNTAX_NODE,
    EXPR_NODE,
    EXPR_BIN,
    EXPR_UNARY,
    VAR_IDENT,
    VAR_DECL,
    LIT_INT,
    STMT_EXIT,
};

enum class VarType {
    MUT,
    CONST,
};

struct NodeProgram {
    std::vector<SyntaxNode> program;
    std::unordered_map<std::string, NodeExpr> var_table;

    auto &lookup_node(const std::string &ident) {
        if (ident.empty()) {
            std::println(stderr, "Error: Identifier required.");
            exit(EXIT_FAILURE);
        }
        return var_table.at(ident);
    }
};

struct NodeBinaryExpr { // eg. a + b or 4 * 5
    std::string atom;
    BinOp op;
    std::unique_ptr<NodeBinaryExpr> lhs;
    std::unique_ptr<NodeBinaryExpr> rhs;
    void print() const;
private:
    std::string op_to_string(BinOp bop) const;


};

struct NodeUnaryExpr {
    std::unique_ptr<SyntaxNode> right; // right recursed
    BinOp op;
};

struct NodeIdentifier {
    std::string name; // used for variable lookup
};

// Variables
struct NodeVarDeclaration {
    VarType kind; // mut, const
    NodeIdentifier ident;
    std::unique_ptr<SyntaxNode> value;
};

struct NodeIntLiteral {
    int value;
};

struct NodeStmtExit {
    std::unique_ptr<SyntaxNode> exit_code;
};

struct NodeExpr {
    std::unique_ptr<SyntaxNode> expr; // Literal or expression
};

struct SyntaxNode {
  public:
    std::variant<std::unique_ptr<NodeExpr>, NodeBinaryExpr, NodeUnaryExpr,
                 NodeIdentifier, NodeVarDeclaration, NodeIntLiteral,
                 NodeStmtExit>
        m_node;

    //====================================//
    // SyntaxNode(auto &&val)
        // requires(!std::same_as<std::decay_t<decltype(val)>, SyntaxNode>)
        // requires(!std::same_as<decltype(val), SyntaxNode>)
        // : m_node(std::forward<decltype(val)>(val)) {};

    [[nodiscard]] NodeType get_node_type() const;
    //====================================//
};
