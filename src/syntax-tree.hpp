#pragma once

#include "lexer.hpp"
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
    EQ,
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
    MUT, // mutable
    LET, // default
};

struct NodeProgram {
    std::vector<SyntaxNode> main;
    std::unordered_map<std::string, SyntaxNode*> var_table;

    SyntaxNode* lookup_node(const std::string &ident) {
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
    // NodeBinaryExpr* lhs;
    // NodeBinaryExpr* rhs;
    LocData loc;

    // NodeBinaryExpr(const NodeBinaryExpr& node) = default;
    // NodeBinaryExpr(NodeBinaryExpr&& node) = default;
    // NodeBinaryExpr operator=(NodeBinaryExpr&& node) { return std::move(node); };
    // ~NodeBinaryExpr() { free(lhs); free(rhs); }

    void print() const;

private:
    std::string op_to_string(BinOp bop) const;
};

struct NodeUnaryExpr {
    std::unique_ptr<SyntaxNode> right; // right recursed
    BinOp op;
    LocData loc;
};

struct NodeIdentifier {
    std::string name; // used for variable lookup
    LocData loc;
};

// Variables
struct NodeVarDeclaration {
    VarType kind; // mut, const
    NodeIdentifier ident;
    std::unique_ptr<SyntaxNode> value;
    LocData loc;
};

struct NodeIntLiteral {
    int value;
    LocData loc;
};

struct NodeStmtExit {
    std::unique_ptr<SyntaxNode> exit_code;
    LocData loc;
};

struct NodeExpr {
    std::unique_ptr<SyntaxNode> expr; // Literal or expression
    LocData loc;
};

struct SyntaxNode {
  public:
    std::variant<std::unique_ptr<NodeExpr>, NodeBinaryExpr, NodeUnaryExpr,
                 NodeIdentifier, NodeVarDeclaration, NodeIntLiteral,
                 NodeStmtExit>
        m_node;
    LocData loc;

    // SyntaxNode(const auto& node) : m_node(node) {}
    // SyntaxNode operator=(const auto& node) { return node; }
    // SyntaxNode(auto&& node) : m_node(std::move(node)) {}
    // SyntaxNode operator=(auto&& node) { return std::move(node); }
    // ~SyntaxNode() = default;

    //====================================//
    [[nodiscard]] NodeType get_node_type() const;
    [[nodiscard]] const SyntaxNode* get_node() const;
    //====================================//
};
