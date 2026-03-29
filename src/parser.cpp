#include "parser.hpp"
#include "lexer.hpp"
#include "syntax-tree.hpp"

#include <algorithm>
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

bool Parser::validate_token(const size_t offset, const TokenType ttype = TokenType::NIL_, const BinOp bop = BinOp::NIL_) const {
    if (ttype == TokenType::NIL_ && bop == BinOp::NIL_) {
        assert(false && "Error: You forgot to specify a type to validate_token()");
    }
    std::string bop_id{};
    if (bop != BinOp::NIL_) {
        return peek(offset).has_value() && (bop == set_op(peek(offset).value().value));
    }
    return (peek(offset).has_value() && peek(offset).value().type == ttype);
}

std::tuple<float, float> Parser::get_binding_power(const BinOp bop) {
    switch (bop) {
    case BinOp::SUB:  [[fallthrough]];
    case BinOp::ADD:  { return {1, 1.1}; }
    case BinOp::DIV:  [[fallthrough]];
    case BinOp::MUL:  { return {2, 2.1}; }
    }
}

BinOp Parser::set_op(const std::string &optype) {
    if (optype == "+")  { return BinOp::ADD; }
    if (optype == "-")  { return BinOp::SUB; }
    if (optype == "*")  { return BinOp::MUL; }
    if (optype == "/")  { return BinOp::DIV; }
    if (optype == "=")  { return BinOp::EQ; }
    if (optype == "<")  { return BinOp::LT; }
    if (optype == ">")  { return BinOp::GT; }
    if (optype == "<=") { return BinOp::LTE; }
    if (optype == ">=") { return BinOp::GTE; }
    if (optype == "==") { return BinOp::EQUIV; }
    return BinOp::NIL_;
}

NodeVarDeclaration Parser::parse_declaration(const TokenType ttype, NodeScope& scope) {
    #pragma clang diagnostic ignored "-Wswitch"
    if (!validate_token(0, TokenType::VAR_IDENT)) {
        std::println(
            "[{}:{}] Error: Missing declaration identifier after `let`.",
            peek().value().loc.line, peek().value().loc.col);
        exit(EXIT_FAILURE);
    }
    if (!validate_token(1, TokenType::NIL_, BinOp::EQ)) {
        std::println("[{}:{}] Error: Missing `=` after variable declaration `{}`",
            peek().value().loc.line, peek().value().loc.col, peek(0).value().value);
        exit(EXIT_FAILURE);
    }

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
    // m_var_table.insert({ dec.ident.name, dec.value.get() });
    scope.var_table.insert({ dec.ident.name, dec.value.get() });
    // std::println("ID IN MAP: {}", );
    // std::get<NodeBinaryExpr>(m_var_table.at(dec.ident.name)->m_node).print();
    std::get<NodeBinaryExpr>(scope.var_table.at(dec.ident.name)->m_node).print();
    std::println();

    return dec;
}

std::unique_ptr<SyntaxNode> Parser::parse_expr() {
    auto res = std::make_unique<SyntaxNode>(std::move(*parse_expr_impl(0)));
    // if (!validate_token(0, TokenType::DELIM_SEMI)) {
    //     std::println("VALU: {}", peek().value().value);
    //     std::println(stderr, "[{}:{}] Error: Missing `;`.", peek().value().loc.line - 1, // -1 as cur tok is on next line
    //         peek().value().loc.col);
    //     exit(EXIT_FAILURE);
    // }
    return res;
}

std::unique_ptr<NodeBinaryExpr> Parser::parse_expr_impl(const float min_rbp) {
    #pragma clang diagnostic ignored "-Wswitch"

    if (!peek().has_value()) {
        std::println(stderr, "Error: Invalid expression.");
        exit(EXIT_FAILURE);
    }

    auto lhs = std::make_unique<NodeBinaryExpr>();
    if (peek().value().type == TokenType::DELIM_LPAREN) { // subexpression
        next(); // eat '('
        lhs = parse_expr_impl(0);
        if (!validate_token(0, TokenType::DELIM_RPAREN)) {
            std::println(stderr, "[{}:{}]Error: Missing `)`.", peek().value().loc.line, peek().value().loc.col);
            exit(EXIT_FAILURE);
        }
        // next(); // eat ')'
    }
    if (validate_token(0, TokenType::DELIM_SEMI)) return lhs;
    if (validate_token(0, TokenType::DELIM_LCURLY)) return lhs; // for scopes
    const auto tok = peek().value();

    switch (tok.type) {
        case TokenType::LIT_INT: {
            lhs->atom.emplace<NodeIntLiteral>(NodeIntLiteral({ 
                .value = std::stoi(tok.value), 
                .loc = tok.loc 
            }));
            lhs->var_count++;
            break;
        }
        case TokenType::VAR_IDENT: {
            lhs->atom.emplace<NodeIdentifier>(NodeIdentifier({ 
                .name = std::move(tok.value),
                .loc = tok.loc 
            }));
            lhs->var_count++;
            break;
        }
    }
    next();
    //NOTE: This checking logic needs improvement
    if (!validate_token(0, TokenType::BIN_OP) && !validate_token(0, TokenType::DELIM_SEMI) 
        && !validate_token(0, TokenType::DELIM_RPAREN)){
        std::println(stderr, "[{}:{}]Error: Invalid binary operator `{}`.", 
                     peek().value().loc.line,
                     peek().value().loc.col,
                     peek().value().value);
        exit(EXIT_FAILURE);
    }

    while (validate_token(0, TokenType::BIN_OP)) {
        BinOp op = set_op(peek().value().value);
        if (op == BinOp::EQ) {
            std::println(stderr, "[{}:{}]Error: Operator `=` not allowed in expression.", peek().value().loc.line, 
                         peek().value().loc.col);
        }
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

NodeStmtIf Parser::parse_stmt_if() {
    if (!peek().has_value()) {
        std::println(stderr, "Error: Invalid conditional.");
        exit(EXIT_FAILURE);
    }
    if (!validate_token(0, TokenType::DELIM_LPAREN)) {
        std::println(stderr, "[{}:{}]Error: Missing `(`.", peek().value().loc.line, peek().value().loc.col);
        exit(EXIT_FAILURE);
    }

    std::unique_ptr<SyntaxNode> cond { parse_expr() };
    if (!validate_token(0, TokenType::DELIM_LCURLY)) {
        std::println("Token here is: {}", peek().value().value);
        std::println(stderr, "[{}:{}]Error: Invalid conditional.", peek().value().loc.line, peek().value().loc.col);
        exit(EXIT_FAILURE);
    }
    return NodeStmtIf {
        .cond = std::move(cond),
        .scope = parse_stmt(false),
    };
}

NodeStmtExit Parser::parse_stmt_exit(const TokenType ttype) {
    if (!peek(0).has_value()) {
        std::println(stderr, "[{}:{}] Error: Missing statement or expression after `exit`.",
                    peek().value().loc.line, peek().value().loc.col);
        exit(EXIT_FAILURE);
    }
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
std::unique_ptr<NodeScope> Parser::parse_stmt(const bool is_prog) {
    // is_prog = true;
    auto scope = std::make_unique<NodeScope>();
    const auto clause_func = [&] () -> bool {
        return is_prog ? peek().has_value()
                       : !validate_token(0, TokenType::DELIM_RCURLY);
    };

    while (clause_func()) {
        const auto peeked = peek().value();
        next();

        switch (peeked.type) {
            case TokenType::KW_LET:
            case TokenType::KW_MUT:{
                scope->stmts.emplace_back(parse_declaration(peeked.type, *scope));
                break;
            }
            case TokenType::KW_EXIT: {
                scope->stmts.emplace_back(parse_stmt_exit(peeked.type));
                break;
            }
            case TokenType::KW_IF: {
                std::println("in iffff");
                scope->stmts.emplace_back(parse_stmt_if());
                break;
            }
        }
    }
    return scope;
}

NodeProgram&& Parser::create_program() {
    const auto res{ parse_stmt(true) };
    m_program.main = std::move(*(res));
    //NOTE: This is lowkey invalid i think (use after move)
    m_program.var_table = std::move(parse_stmt(true)->var_table);
    return std::move(m_program);
}
