#include "parser.hpp"
#include "lexer.hpp"
#include "syntax-tree.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <optional>
#include <print>
#include <utility>
#include <vector>

Parser::Parser(std::vector<Token> &&toks) noexcept
    : m_tokens(std::move(toks)) {};

std::optional<Token> Parser::next() {
    if (m_tokens.empty() || m_tok_ptr >= m_tokens.size()) {
        return std::nullopt;
    }
    return m_tokens.at(m_tok_ptr++);
}

std::optional<Token> Parser::peek(const size_t offset = 0) const {
    if (m_tokens.empty() || (offset + m_tok_ptr) >= m_tokens.size()) {
        return std::nullopt;
    }
    return m_tokens.at(m_tok_ptr + offset);
}

bool Parser::validate_token(const size_t offset, const TokenType ttype) const {
    return (peek(offset).has_value() && peek(offset).value().type == ttype);
}

std::tuple<float, float> Parser::get_binding_power(const BinOp bop) {
    switch (bop) {
    case BinOp::SUB:  [[fallthrough]];
    case BinOp::ADD:  { return {1, 1.1}; }
    case BinOp::DIV:  [[fallthrough]];
    case BinOp::MULT: { return {2, 2.1}; }
    }
}

BinOp Parser::set_op(const std::string &optype) {
    if (optype == "+") { return BinOp::ADD; }
    if (optype == "-") { return BinOp::SUB; }
    if (optype == "*") { return BinOp::MULT; }
    if (optype == "/") { return BinOp::DIV; }
    // else if (optype == "=") { return BinOp::EQ; }
    std::println(stderr, "Error: Invalid binary operator.");
    exit(EXIT_FAILURE);
}

NodeVarDeclaration Parser::parse_declaration(const TokenType ttype) {
    #pragma clang diagnostic ignored "-Wswitch"

    NodeVarDeclaration dec{};
    switch (ttype) {
        case TokenType::KW_LET: {
            dec.kind = VarType::LET;
            break;
        }
        case TokenType::KW_MUT: {
            dec.kind = VarType::MUT;
            break;
        }
    }

    dec.ident = NodeIdentifier({
        .name = std::move(peek().value().value),
        .loc = next().value().loc
    });
    next(); // eat `=`

    std::println("ident: {}", dec.ident.name);
    dec.value = parse_expr();
    m_var_table.insert({ dec.ident.name, dec.value.get() });
    // std::println("ID IN MAP: {}", );
    std::get<NodeBinaryExpr>(m_var_table.at(dec.ident.name)->m_node).print();
    std::println();

    return dec;
}

std::unique_ptr<SyntaxNode> Parser::parse_expr() {
    // TODO: REFACTOR NodeBinaryExpr to hold variants in lhs and rhs
    return std::make_unique<SyntaxNode>(std::move(*parse_expr_impl(0)));
}

// std::unique_ptr<NodeBinaryExpr> Parser::parse_expr_impl(const float min_rbp) {
//     if (!peek().has_value()) {
//         std::println(stderr, "Error: Invalid expression.");
//         exit(EXIT_FAILURE);
//     }
//
//     auto lhs = std::make_unique<NodeBinaryExpr>();
//     if (peek().value().type == TokenType::DELIM_LPAREN) {
//         next(); // eat '('
//         lhs = parse_expr_impl(0);
//         next(); // eat ')'
//     }
//     if (validate_token(0, TokenType::DELIM_SEMI)) return lhs;
//     if (!peek(1).has_value() && peek().value().type != TokenType::DELIM_SEMI) {
//         std::println(stderr, "[{}:{}] Error: Missing `;`.", lhs->loc.line,lhs->loc.col);
//         exit(EXIT_FAILURE);
//     }
//
//     auto tok = peek().value();
//     lhs->atom = std::move( tok.value );
//     lhs->loc = tok.loc ;
//     lhs->var_count++;
//     next();
//
//     while (validate_token(0, TokenType::BIN_OP)) {
//         BinOp op = set_op(peek().value().value);
//         auto [lbp, rbp] = get_binding_power(op);
//
//         if (lbp < min_rbp) break;
//         next();
//
//         auto rhs = parse_expr_impl(rbp);
//         auto node = std::make_unique<NodeBinaryExpr>();
//
//         node->op = op;
//         node->loc = lhs->loc;
//         node->var_count += lhs->var_count + rhs->var_count;
//         node->lhs = std::move(lhs);
//         node->rhs = std::move(rhs);
//
//         lhs = std::move(node);
//     }
//     return lhs;
// }
std::unique_ptr<NodeBinaryExpr> Parser::parse_expr_impl(const float min_rbp) {
    #pragma clang diagnostic ignored "-Wswitch"

    if (!peek().has_value()) {
        std::println(stderr, "Error: Invalid expression.");
        exit(EXIT_FAILURE);
    }

    auto lhs = std::make_unique<NodeBinaryExpr>();
    if (peek().value().type == TokenType::DELIM_LPAREN) {
        next(); // eat '('
        lhs = parse_expr_impl(0);
        next(); // eat ')'
    }
    if (validate_token(0, TokenType::DELIM_SEMI)) return lhs;
    if (!peek(1).has_value() && peek().value().type != TokenType::DELIM_SEMI) {
        std::println(stderr, "[{}:{}] Error: Missing `;`.", lhs->loc.line,lhs->loc.col);
        exit(EXIT_FAILURE);
    }
    auto tok = peek().value();

    switch (tok.type) {
        case TokenType::LIT_INT: {
            lhs->atom.emplace<NodeIntLiteral>(NodeIntLiteral({ 
                .value = std::stoi(tok.value), 
                .loc = tok.loc 
            }));
            break;
        }
        case TokenType::VAR_IDENT: {
            lhs->atom.emplace<NodeIdentifier>(NodeIdentifier({ 
                .name = std::move(tok.value),
                .loc = tok.loc 
            }));
        }
    }
    lhs->var_count++;
    next();

    while (validate_token(0, TokenType::BIN_OP)) {
        BinOp op = set_op(peek().value().value);
        auto [lbp, rbp] = get_binding_power(op);

        if (lbp < min_rbp) break;
        next();

        auto rhs = parse_expr_impl(rbp);
        auto node = std::make_unique<NodeBinaryExpr>();

        node->op = op;
        node->loc = lhs->loc;
        node->var_count += lhs->var_count + rhs->var_count;
        node->lhs = std::move(lhs);
        node->rhs = std::move(rhs);

        lhs = std::move(node);
    }
    return lhs;
}

NodeStmtExit Parser::parse_stmt_exit(const TokenType ttype) {
    NodeStmtExit exit;
    exit.loc.line = peek().value().loc.line;
    exit.loc.col = peek().value().loc.col;

    switch (ttype) {
        case TokenType::VAR_IDENT: {
            exit.exit_code->m_node = NodeIdentifier({
                .name = std::move(peek().value().value),
                .loc = { peek().value().loc.line, peek().value().loc.col }
            });
            break;
        }
        default: { exit.exit_code = parse_expr(); }
    }
    return exit;
}

NodeProgram&& Parser::create_program() {
    while (peek().has_value()) {
        const auto peeked = peek().value();
        next();

        // std::println("Create program: {}", peeked.value);
        switch (peeked.type) {
            case TokenType::KW_LET:
            case TokenType::KW_MUT:{
                if (!validate_token(0, TokenType::VAR_IDENT)) {
                    std::println(
                        "[{}:{}] Error: Missing declaration identifier after `let`.",
                        peeked.loc.line, peeked.loc.col);
                    exit(EXIT_FAILURE);
                }
                if (!validate_token(1, TokenType::OP_EQUALS)) {
                    std::println("[{}:{}] Error: Missing `=` after variable declaration `{}`",
                                 peeked.loc.line, peeked.loc.col, peek(0).value().value);
                    exit(EXIT_FAILURE);
                }
                m_program.main.emplace_back(parse_declaration(peeked.type));
                break;
            }
            case TokenType::KW_EXIT: {
                if (!peek(0).has_value()) {
                    std::println(stderr, "[{}:{}] Error: Missing statement or expression after `exit`.",
                                peeked.loc.line, peeked.loc.col);
                    exit(EXIT_FAILURE);
                }
                m_program.main.emplace_back(parse_stmt_exit(peeked.type));
                break;
            }
        }
    }

    m_program.var_table = std::move(m_var_table);
    return std::move(m_program);
}
