#include "syntax-tree.hpp"
#include "include/utils.hpp"
#include <cstddef>
#include <memory>

//====================================//
NodeType SyntaxNode::get_node_type() const {
    auto node_typer = Overload {
        [](const std::unique_ptr<NodeExpr>&)           { return NodeType::EXPR_NODE; },
        [](const NodeBinaryExpr&)                      { return NodeType::EXPR_BIN; },
        [](const NodeUnaryExpr&)                       { return NodeType::EXPR_UNARY; },
        [](const NodeIdentifier&)                      { return NodeType::VAR_IDENT; },
        [](const NodeVarDeclaration&)                  { return NodeType::VAR_DECL; },
        [](const NodeIntLiteral&)                      { return NodeType::LIT_INT; },
        [](const NodeStmtExit&)                        { return NodeType::STMT_EXIT; },
    };
    return std::visit(node_typer, m_node);
}

std::string NodeBinaryExpr::op_to_string(const BinOp bop) {
    switch (bop) {
        case BinOp::ADD: return "+";
        case BinOp::SUB: return "-";
        case BinOp::MULT: return "*";
        case BinOp::DIV: return "/";
    }
}

void NodeBinaryExpr::print() const {
    if (!lhs && !rhs) {
        std::print(" {}", atom);
        return;
    }

    std::print("({}", op_to_string(op));
    if (lhs) lhs->print();
    if (rhs) rhs->print();
    std::print(")");
}

