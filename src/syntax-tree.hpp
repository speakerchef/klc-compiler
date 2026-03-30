#pragma once

#include "lexer.hpp"
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <print>
#include <unordered_map>
#include <variant>
#include <vector>

struct SyntaxNode;
struct NodeBinaryExpr;
struct NodeUnaryExpr;
struct NodeIdentifier;
struct NodeIntLiteral;
struct NodeStmtExit;

enum class BinOp {
    ADD,
    SUB,
    MUL,
    DIV,
    PWR,
    BW_OR,
    BW_AND,
    BW_XOR,
    LG_OR,
    LG_AND,
    EQ,
    EQUIV,
    NEQUIV,
    LT,
    GT,
    LTE,
    GTE,
    NIL_,
};

enum class NodeType {
    SYNTAX_NODE,
    SCOPE_NODE,
    EXPR_NODE,
    EXPR_BIN,
    EXPR_UNARY,
    VAR_IDENT,
    VAR_DECL,
    LIT_INT,
    STMT_EXIT,
    STMT_IF,
    STMT_ELSE,
    STMT_ELIF,
    STMT_WHILE,
    STMT_FOR,
};

enum class VarType {
    MUT, // mutable
    LET, // default
};


struct NodeIdentifier {
    std::string name; // used for variable lookup
    LocData loc;
};

struct NodeIntLiteral {
    int value;
    LocData loc;
};

struct NodeBinaryExpr { // eg. a + b or 4 * 5
    std::variant<std::monostate, NodeIdentifier, NodeIntLiteral> atom;
    BinOp op;
    std::unique_ptr<NodeBinaryExpr> lhs;
    std::unique_ptr<NodeBinaryExpr> rhs;
    size_t var_count;
    LocData loc;
    void print() const; // print expr in prefix notation eg. (+ 5 8) === 5 + 8

private:
    [[nodiscard]] static std::string op_to_string(BinOp bop);
};

struct NodeUnaryExpr {
    std::unique_ptr<SyntaxNode> right; // right recursed
    BinOp op;
    LocData loc;
};

// Variables
struct NodeVarDeclaration {
    VarType kind; // mut, let
    NodeIdentifier ident;
    std::unique_ptr<SyntaxNode> value;
    LocData loc;
};

struct NodeStmtExit {
    std::unique_ptr<SyntaxNode> exit_code;
    LocData loc;
};

struct NodeScope {
    std::vector<std::unique_ptr<SyntaxNode>> stmts;
    std::unordered_map<std::string, SyntaxNode*> var_table;
    LocData loc;
};

struct NodeStmtIf {
    std::unique_ptr<SyntaxNode> cond;
    NodeScope scope;
};

struct NodeStmtElif {
    std::unique_ptr<SyntaxNode> cond;
    NodeScope scope;
};

struct NodeStmtElse {
    NodeScope scope;
};

struct NodeStmtWhile {
    std::unique_ptr<SyntaxNode> cond;
    NodeScope scope;
};

struct NodeStmtFor {
    std::unique_ptr<SyntaxNode> cond;
    NodeScope scope;
};

struct NodeProgram {
    NodeScope main;
};

struct SyntaxNode {
  public:
    std::variant<NodeScope, NodeBinaryExpr, NodeUnaryExpr,
                 NodeIdentifier, NodeVarDeclaration, NodeIntLiteral,
                 NodeStmtExit, NodeStmtIf, NodeStmtElse, NodeStmtElif, NodeStmtWhile> m_node;
    
    //====================================//
    [[nodiscard]] NodeType get_node_type() const;
    [[nodiscard]] const SyntaxNode* get_node() const;
    //====================================//
};
