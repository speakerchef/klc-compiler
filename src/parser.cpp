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

std::tuple<float, float> Parser::get_binding_power(const BinOp bop) const {
    switch (bop) {
    // case BinOp::EQ:   { return {0, 0.1}; }
    case BinOp::SUB:
    case BinOp::ADD:  { return {1, 1.1}; }
    case BinOp::DIV:
    case BinOp::MULT: { return {2, 2.1}; }
    }
}

BinOp Parser::set_op(const std::string &optype) const {
    if      (optype == "+") { return BinOp::ADD; }
    else if (optype == "-") { return BinOp::SUB; }
    else if (optype == "*") { return BinOp::MULT; }
    else if (optype == "/") { return BinOp::DIV; }
    // else if (optype == "=") { return BinOp::EQ; }
    std::println(stderr, "Error: Invalid binary operator.");
    exit(EXIT_FAILURE);
}

NodeVarDeclaration Parser::parse_declaration(const TokenType ttype) {
    NodeVarDeclaration dec{};
    dec.value = std::make_unique<SyntaxNode>();

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
        .loc = std::move(next().value().loc)
    });
    next(); // eat `=`

    std::println("ident: {}", dec.ident.name);
    dec.value->m_node = std::move(*parse_expr(0));
    m_var_table.insert({ dec.ident.name, dec.value.get() });

    // std::get<NodeBinaryExpr>(m_var_table.at(dec.ident.name)->m_node).print();
    // std::println();

    return dec;
}

std::unique_ptr<NodeBinaryExpr> Parser::parse_expr(const float min_rbp) {
    if (!peek().has_value()) {
        std::println(stderr, "Error: Invalid expression.");
        exit(EXIT_FAILURE);
    }
    auto tok = peek().value();


    auto lhs = std::make_unique<NodeBinaryExpr>();
    if (tok.type == TokenType::DELIM_LPAREN) {
        next();
        lhs = parse_expr(0);
    }
    lhs->atom = std::move( tok.value );
    lhs->loc = std::move( tok.loc );
    // std::println("Line: {}", lhs->loc.line);
    // std::println("Col: {}", lhs->loc.col);

    if (!peek(1).has_value() && tok.type != TokenType::DELIM_SEMI) {
        std::println(stderr, "[{}:{}] Error: Missing `;`.", lhs->loc.line,lhs->loc.col);
        exit(EXIT_FAILURE);
    }
    next();

    while (validate_token(0, TokenType::BIN_OP)) {
        BinOp op = set_op(peek().value().value);
        auto [lbp, rbp] = get_binding_power(op);

        if (lbp < min_rbp) { break; }
        next();

        auto rhs = parse_expr(rbp);
        auto node = std::make_unique<NodeBinaryExpr>();
        node->op = op;
        node->loc = std::move(lhs->loc);
        node->lhs = std::move(lhs);
        node->rhs = std::move(rhs);
        lhs = std::move(node);
    }
    return lhs;
}

NodeStmtExit Parser::parse_stmt_exit(const TokenType ttype) {
    NodeStmtExit exit;
    NodeBinaryExpr expr;

    exit.loc.line = peek().value().loc.line;
    exit.loc.col = peek().value().loc.col;
    exit.exit_code = std::make_unique<SyntaxNode>();

    switch (ttype) {
        case TokenType::VAR_IDENT: {
            exit.exit_code->m_node = NodeIdentifier({
                .name = std::move(peek().value().value),
                .loc = { peek().value().loc.line, peek().value().loc.col }
            });
            break;
        }
        default: {
            auto res = std::move(*parse_expr(0));
            std::print("Print at exit: ");
            res.print();
            std::println();
            exit.exit_code->m_node = std::move(res);
        }
    }
    return exit;
}

NodeProgram&& Parser::create_program() {
    while (peek().has_value()) {
        auto peeked = peek().value();
        next();

        std::println("Create program: {}", peeked.value);
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
