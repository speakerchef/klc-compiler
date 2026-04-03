#include "syntax-tree.hpp"
#include "include/utils.hpp"
#include <cstddef>
#include <memory>
#include <variant>

//====================================//
NodeType SyntaxNode::get_node_type() const {
    auto node_typer = Overload {
        [](const NodeScope&)                           { return NodeType::SCOPE_NODE; },
        [](const NodeExpr&)                      { return NodeType::EXPR_BIN; },
        [](const NodeUnaryExpr&)                       { return NodeType::EXPR_UNARY; },
        [](const NodeIdentifier&)                      { return NodeType::VAR_IDENT; },
        [](const NodeVarDeclaration&)                  { return NodeType::VAR_DECL; },
        [](const NodeIntLiteral&)                      { return NodeType::LIT_INT; },
        [](const NodeStmtExit&)                        { return NodeType::STMT_EXIT; },
        [](const NodeStmtIf&)                          { return NodeType::STMT_IF; },
        [](const NodeStmtElif&)                        { return NodeType::STMT_ELIF; },
        [](const NodeStmtElse&)                        { return NodeType::STMT_ELSE; },
        [](const NodeStmtWhile&)                       { return NodeType::STMT_WHILE; },
        [](const NodeFunc&)                            { return NodeType::STMT_FN; },
    };
    return std::visit(node_typer, m_node);
}

std::string NodeExpr::op_to_string(const Op op) {
    switch (op) {
    case Op::ADD:     return "+";
    case Op::SUB:     return "-";
    case Op::MUL:     return "*";
    case Op::DIV:     return "/";
    case Op::PWR:     return "**";
    case Op::MOD:     return "%";
    case Op::INC:     return "++";
    case Op::DEC:     return "--";
    case Op::EQ:      return "=";
    case Op::ADD_EQ:  return "+=";
    case Op::SUB_EQ:  return "-=";
    case Op::MUL_EQ:  return "*=";
    case Op::DIV_EQ:  return "/=";
    case Op::PWR_EQ:  return "**=";
    case Op::MOD_EQ:  return "%=";
    case Op::AND_EQ:  return "&=";
    case Op::OR_EQ:   return "|=";
    case Op::XOR_EQ:  return "^=";
    case Op::LSL_EQ:  return "<<=";
    case Op::LSR_EQ:  return ">>=";
    case Op::EQUIV:   return "==";
    case Op::NEQUIV:  return "!=";
    case Op::LTE:     return "<=";
    case Op::GTE:     return ">=";
    case Op::LT:      return "<";
    case Op::GT:      return ">";
    case Op::LSL:     return "<<";
    case Op::LSR:     return ">>";
    case Op::BW_NOT:  return "~";
    case Op::BW_OR:   return "|";
    case Op::BW_AND:  return "&";
    case Op::BW_XOR:  return "^";
    case Op::LG_NOT:  return "!";
    case Op::LG_OR:   return "||";
    case Op::LG_AND:  return "&&"; 
    }
}

void NodeExpr::print() const {
    if (!lhs && !rhs) {
        const auto print_v = Overload {
            [](const NodeIdentifier& val) { std:: print(" {}", val.name); },
            [](const NodeIntLiteral& val) { std:: print(" {}", val.value); },
            [](const std::monostate&) {},
        };
        std::visit(print_v, atom);
        return;
    }
    // if (!lhs) {
    //     const auto print_v = Overload {
    //         [](const NodeIdentifier& val) { std:: print(" {})", val.name); },
    //         [](const NodeIntLiteral& val) { std:: print(" {})", val.value); },
    //         [](const std::monostate&) {},
    //     };
    //     std::visit(print_v, atom);
    // }
    // if (!rhs) {
    //     const auto print_v = Overload {
    //         [](const NodeIdentifier& val) { std:: print(" ({}", val.name); },
    //         [](const NodeIntLiteral& val) { std:: print(" ({}", val.value); },
    //         [](const std::monostate&) {},
    //     };
    //     std::visit(print_v, atom);
    // }

    std::print(" ({}", op_to_string(op));
    if (lhs) lhs->print();
    if (rhs) rhs->print();
    std::print(")");
}

