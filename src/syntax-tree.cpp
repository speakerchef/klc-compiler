#include "syntax-tree.hpp"

//====================================//
TokenType SyntaxNode::get_node_type() const {
    auto node_typer = Overload{
        [](ExprExit) { return TokenType::KW_EXIT; },
        [](ExprIntVariable) { return TokenType::VAR_INT; },
        [](ExprIntLiteral) { return TokenType::LIT_INT; },
        [](ExprLet) { return TokenType::KW_LET; },
    };
    return std::visit(node_typer, m_node);
}

//====================================//
void SyntaxNode::set_node_value(auto &&node_val) {
    auto setter_v = [&](auto &&val) {
        using decay_t = std::decay_t<decltype(val)>;
        if constexpr (std::is_same_v<decay_t, NodeStmtExit>) {
            m_node = {.exit_code = val.exit_code};
        } else if constexpr (std::is_same_v<decay_t, NodeVarDeclaration>) {
            m_node = {};
        } else if constexpr (std::is_same_v<decay_t, NodeVarDeclarator>) {
            m_node = {.value = val.value,
                .ident = std::move(val.ident),
                .is_immutable = val.is_immutable};
        } else if constexpr (std::is_same_v<decay_t, NodeIntLiteral>) {
            m_node = {.value = val.value};
        }
    };
    std::visit(setter_v, std::move(node_val));
}

//TODO: figure out node construction from top down
//TODO: recursive exploration of nodes
//====================================//
void SyntaxTree::push_node(SyntaxNode &&node) {

    auto setter_v = Overload {
        [](NodeBinaryExpr &&val) { val.expr }
    };
    switch (node.get_node_type()) {
        case TokenType::VAR_INT: {
            auto n = std::get<ExprIntVariable>(node.get_node_value());
            m_var_table.insert({std::move(n.ident), SyntaxNode(n)});
            m_called_nodes.emplace_back(std::move(node));
            break;
        }
        case TokenType::KW_LET:
        case TokenType::DELIM_SEMI:
        case TokenType::KW_RETURN:
        case TokenType::KW_EXIT:
        case TokenType::KW_INT:
        case TokenType::LIT_INT:
        case TokenType::LIT_STR:
        case TokenType::UNCLASSED_VAR_DEC:
        case TokenType::OP_EQUALS:
        case TokenType::OP_PLUS:
        case TokenType::OP_MINUS: {
            m_called_nodes.emplace_back(std::move(node));
            break;
        }
    }
}
auto& SyntaxTree::lookup_node(const std::string& ident) const {
    if (ident.empty()) {
        std::println(stderr, "Error: Identifier required.");
        exit(EXIT_FAILURE);
    }
    return m_var_table.at(ident);
}

const std::vector<SyntaxNode>& SyntaxTree::get_called_nodes() {
    return m_called_nodes;
}
const std::unordered_map<std::string, NodeExpr>& SyntaxTree::get_var_table() const {
    return m_var_table;
}
const SyntaxNode& SyntaxTree::peek() const {

}
