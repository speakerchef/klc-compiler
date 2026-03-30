#include "syntax-tree.hpp"
#include "include/utils.hpp"
#include <cstddef>
#include <memory>
#include <variant>

//====================================//
NodeType SyntaxNode::get_node_type() const {
    auto node_typer = Overload {
        [](const NodeScope&)                           { return NodeType::SCOPE_NODE; },
        [](const NodeBinaryExpr&)                      { return NodeType::EXPR_BIN; },
        [](const NodeUnaryExpr&)                       { return NodeType::EXPR_UNARY; },
        [](const NodeIdentifier&)                      { return NodeType::VAR_IDENT; },
        [](const NodeVarDeclaration&)                  { return NodeType::VAR_DECL; },
        [](const NodeIntLiteral&)                      { return NodeType::LIT_INT; },
        [](const NodeStmtExit&)                        { return NodeType::STMT_EXIT; },
        [](const NodeStmtIf&)                          { return NodeType::STMT_IF; },
        [](const NodeStmtElif&)                        { return NodeType::STMT_ELIF; },
        [](const NodeStmtElse&)                        { return NodeType::STMT_ELSE; },
        [](const NodeStmtWhile&)                       { return NodeType::STMT_WHILE; },
        [](const NodeStmtFor&)                         { return NodeType::STMT_FOR; },
    };
    return std::visit(node_typer, m_node);
}

std::string NodeBinaryExpr::op_to_string(const BinOp bop) {
    switch (bop) {
    case BinOp::ADD:     return "+";
    case BinOp::SUB:     return "-";
    case BinOp::MUL:     return "*";
    case BinOp::DIV:     return "/";
    case BinOp::PWR:     return "**";
    case BinOp::EQ:      return "=";
    case BinOp::EQUIV:   return "==";
    case BinOp::NEQUIV:  return "!=";
    case BinOp::LTE:     return "<=";
    case BinOp::GTE:     return ">=";
    case BinOp::LT:      return "<";
    case BinOp::GT:      return ">";
    case BinOp::BW_OR:   return "|";
    case BinOp::BW_AND:  return "&";
    case BinOp::BW_XOR:  return "^";
    case BinOp::LG_OR:   return "||";
    case BinOp::LG_AND:  return "&&";
    }
}

void NodeBinaryExpr::print() const {
    if (!lhs && !rhs) {
        const auto print_v = Overload {
            [](const NodeIdentifier& val) { std:: print(" {}", val.name); },
            [](const NodeIntLiteral& val) { std:: print(" {}", val.value); },
            [](const std::monostate&) {},
        };
        std::visit(print_v, atom);
        return;
    }

    std::print("({}", op_to_string(op));
    if (lhs) lhs->print();
    if (rhs) rhs->print();
    std::print(")");
}

