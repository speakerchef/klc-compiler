#pragma once

#include "lexer.hpp"
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

struct SyntaxNode;
struct NodeExpr;
struct NodeUnaryExpr;
struct NodeIdentifier;
struct NodeIntLiteral;
struct NodeStmtExit;

enum class Op {
    NOP,
    ADD,
    SUB,
    MUL,
    DIV,
    PWR,
    MOD,
    INC,
    DEC,
    LSL,
    LSR,
    BW_NOT, // '~'
    BW_OR,
    BW_AND,
    BW_XOR,
    LG_NOT, // '!'
    LG_OR,
    LG_AND,
    EQ,
    ADD_EQ,
    SUB_EQ,
    MUL_EQ,
    DIV_EQ,
    PWR_EQ,
    MOD_EQ,
    AND_EQ,
    OR_EQ,
    XOR_EQ,
    LSL_EQ,
    LSR_EQ,
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
    STMT_FN,
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
    int64_t value;
    LocData loc;
};

using ExprAtom_T = std::variant<std::monostate, NodeIdentifier, NodeIntLiteral>;

enum class Fix {
    PREFIX,
    POSTFIX,
};

struct NodeExpr { // eg. a + b, a += 5, -5, i++, i--, etc
    ExprAtom_T atom;
    Op op = Op::NOP;
    Fix fix;
    bool is_negative = false;
    bool is_positive = false;
    std::unique_ptr<NodeExpr> lhs;
    std::unique_ptr<NodeExpr> rhs;
    size_t var_count;
    LocData loc;

    void print() const; // print expr in prefix notation eg. (+ 5 8) === 5 + 8
    [[nodiscard]] static std::string op_to_string(Op op);
};

// Variables
struct NodeVarDeclaration {
    VarType kind; // mut, let
    NodeIdentifier ident;
    std::unique_ptr<SyntaxNode> value;
    LocData loc;
};

struct NodeFunc {
    NodeIdentifier ident;
    std::vector<std::unique_ptr<SyntaxNode>> args;
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

struct NodeStmtElif {
    std::unique_ptr<SyntaxNode> cond;
    NodeScope scope;
    LocData loc;
};

struct NodeStmtElse {
    NodeScope scope;
    LocData loc;
};

struct NodeStmtIf {
    std::unique_ptr<SyntaxNode> cond;
    NodeScope scope;
    std::vector<NodeStmtElif> n_elif;
    std::optional<NodeStmtElse> n_else;
    LocData loc;
};

struct NodeStmtWhile {
    std::unique_ptr<SyntaxNode> cond;
    NodeScope scope;
    LocData loc;
};

struct NodeProgram {
    NodeScope main;
};

struct SyntaxNode {
  public:
    std::variant<NodeScope, NodeExpr,
                 NodeIdentifier, NodeVarDeclaration, NodeIntLiteral,
                 NodeStmtExit, NodeStmtIf, NodeStmtElse, NodeStmtElif, 
                 NodeStmtWhile, NodeFunc> m_node;
    
    //====================================//
    [[nodiscard]] NodeType get_node_type() const;
    [[nodiscard]] const SyntaxNode* get_node() const;
    //====================================//
};
