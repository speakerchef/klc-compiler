#pragma once

#include "lexer.hpp"
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <print>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

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
    MUT, // mutable
    LET, // default
};

struct NodeProgram {
    std::vector<SyntaxNode> main;
    std::unordered_map<std::string, SyntaxNode*> var_table;

    [[nodiscard]] const SyntaxNode* lookup_node(const std::string &ident) const {
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
    size_t var_count;
    LocData loc;
    void print() const;

private:
    [[nodiscard]] static std::string op_to_string(BinOp bop);
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

struct SyntaxNode {
  public:
    std::variant<NodeBinaryExpr, NodeUnaryExpr,
                 NodeIdentifier, NodeVarDeclaration, NodeIntLiteral,
                 NodeStmtExit> m_node;
    
    //====================================//
    [[nodiscard]] NodeType get_node_type() const;
    [[nodiscard]] const SyntaxNode* get_node() const;
    //====================================//
};
