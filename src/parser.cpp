#include "parser.hpp"
#include "lexer.hpp"
#include "syntax-tree.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <optional>
#include <print>
#include <utility>
#include <vector>

Parser::Parser(std::vector<Token> &&toks) noexcept
    : m_tokens(std::move(toks)), m_program({}) {};

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
    // Assignment (right associative)
    case BinOp::EQ:      { return {1, 0.9}; }

    //logical
    case BinOp::LG_OR:   { return {2, 2.1}; }
    case BinOp::LG_AND:  { return {3, 3.1}; }

    //Bitwise
    case BinOp::BW_OR:   { return {4, 4.1}; }
    case BinOp::BW_XOR:     { return {5, 5.1}; }
    case BinOp::BW_AND:  { return {6, 6.1}; }

    case BinOp::EQUIV:   [[fallthrough]];
    case BinOp::NEQUIV:  { return {7, 7.1}; }

    case BinOp::LT:      [[fallthrough]];
    case BinOp::GT:      [[fallthrough]];
    case BinOp::LTE:     [[fallthrough]];
    case BinOp::GTE:     { return {8, 8.1}; }

    case BinOp::SUB:     [[fallthrough]];
    case BinOp::ADD:     { return {9, 9.1}; }
    case BinOp::DIV:     [[fallthrough]];
    case BinOp::MUL:     { return {10, 10.1}; }
    // Power (right associative)
    case BinOp::PWR:     { return {11, 10.9}; }

    default: assert(false && "Error: Unknown BinOp; Cannot get binding power.");
    }
}

BinOp Parser::set_op(const std::string &optype) {
    if (optype == "+")  { return BinOp::ADD; }
    if (optype == "-")  { return BinOp::SUB; }
    if (optype == "*")  { return BinOp::MUL; }
    if (optype == "**") { return BinOp::PWR; }
    if (optype == "/")  { return BinOp::DIV; }
    if (optype == "=")  { return BinOp::EQ; }
    if (optype == "<")  { return BinOp::LT; }
    if (optype == ">")  { return BinOp::GT; }
    if (optype == "<=") { return BinOp::LTE; }
    if (optype == ">=") { return BinOp::GTE; }
    if (optype == "==") { return BinOp::EQUIV; }
    if (optype == "!=") { return BinOp::NEQUIV; }
    if (optype == "|")  { return BinOp::BW_OR; }
    if (optype == "||") { return BinOp::LG_OR; }
    if (optype == "&")  { return BinOp::BW_AND; }
    if (optype == "&&") { return BinOp::LG_AND; }
    if (optype == "^")  { return BinOp::BW_XOR; }
    return BinOp::NIL_;
}

NodeVarDeclaration Parser::parse_declaration(const TokenType ttype,
    std::unordered_map<std::string, SyntaxNode*>& loc_scp, const bool is_prog) {

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
        default: { assert(false && "Error: Unknown variable declaration type."); }
    }

    dec.ident = NodeIdentifier({
        .name = std::move(peek().value().value),
        .loc = next().value().loc
    });
    next(); // eat `=`

    std::println("ident: {}", dec.ident.name);
    dec.value = parse_expr();

    if (m_program.main.var_table.contains(dec.ident.name) || loc_scp.contains(dec.ident.name)) {
        std::println(stderr, "[{}:{}] Error: Re-definition of `{}`.",
            dec.loc.line, dec.loc.col, dec.ident.name);
        exit(EXIT_FAILURE);
    }
    if (is_prog) {
        m_program.main.var_table.insert({ dec.ident.name, dec.value.get() });
        std::get<NodeBinaryExpr>(m_program.main.var_table.at(dec.ident.name)->m_node).print();
    } else {
        loc_scp.insert({ dec.ident.name, dec.value.get() });
        std::get<NodeBinaryExpr>(loc_scp.at(dec.ident.name)->m_node).print();
    }
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
    if (validate_token(1, TokenType::DELIM_LCURLY)) return lhs; // for scopes
    auto [type, value, loc] = peek().value();

    switch (type) {
        case TokenType::LIT_INT: {
            lhs->atom.emplace<NodeIntLiteral>(NodeIntLiteral({ 
                .value = std::stoi(value),
                .loc = loc
            }));
            lhs->var_count++;
            break;
        }
        case TokenType::VAR_IDENT: {
            lhs->atom.emplace<NodeIdentifier>(NodeIdentifier({ 
                .name = std::move(value),
                .loc = loc
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

NodeStmtIf Parser::parse_stmt_if(NodeScope& loc_scp) {
    if (!peek().has_value()) {
        std::println(stderr, "Error: Invalid conditional.");
        exit(EXIT_FAILURE);
    }
    if (!validate_token(0, TokenType::DELIM_LPAREN)) {
        std::println(stderr, "[{}:{}]Error: Missing `(`.", peek().value().loc.line, peek().value().loc.col);
        exit(EXIT_FAILURE);
    }

    std::unique_ptr<SyntaxNode> cond { parse_expr() };
    if (!validate_token(0, TokenType::DELIM_RPAREN)) {
        std::println(stderr, "[{}:{}]Error: Missing `)`.", peek().value().loc.line, peek().value().loc.col);
        exit(EXIT_FAILURE);
    }
    if (!validate_token(1, TokenType::DELIM_LCURLY)) {
        std::println("Token here is: {}", peek(1).value().value);
        std::println(stderr, "[{}:{}]Error: Invalid conditional.", peek(1).value().loc.line, peek(1).value().loc.col);
        exit(EXIT_FAILURE);
    }
    return NodeStmtIf {
        .cond = std::move(cond),
        .scope = parse_stmt(false, loc_scp),
    };
}

NodeStmtElif Parser::parse_stmt_elif(NodeScope& loc_scp) {
    if (!peek().has_value()) {
        std::println(stderr, "Error: Invalid conditional.");
        exit(EXIT_FAILURE);
    }
    if (!validate_token(0, TokenType::DELIM_LPAREN)) {
        std::println(stderr, "[{}:{}]Error: Missing `(`.", peek().value().loc.line, peek().value().loc.col);
        exit(EXIT_FAILURE);
    }

    std::unique_ptr<SyntaxNode> cond { parse_expr() };
    if (!validate_token(0, TokenType::DELIM_RPAREN)) {
        std::println(stderr, "[{}:{}]Error: Missing `)`.", peek().value().loc.line, peek().value().loc.col);
        exit(EXIT_FAILURE);
    }
    if (!validate_token(1, TokenType::DELIM_LCURLY)) {
        std::println("Token here is: {}", peek(1).value().value);
        std::println(stderr, "[{}:{}]Error: Invalid conditional.", peek(1).value().loc.line, peek(1).value().loc.col);
        exit(EXIT_FAILURE);
    }
    return NodeStmtElif {
        .cond = std::move(cond),
        .scope = parse_stmt(false, loc_scp),
    };
}

NodeStmtElse Parser::parse_stmt_else(NodeScope& loc_scp) {
    if (!peek().has_value()) {
        std::println(stderr, "Error: Invalid conditional.");
        exit(EXIT_FAILURE);
    }
    if (!validate_token(0, TokenType::DELIM_LCURLY)) {
        std::println("Token here is: {}", peek().value().value);
        std::println(stderr, "[{}:{}]Error: Invalid conditional.", peek().value().loc.line, peek().value().loc.col);
        exit(EXIT_FAILURE);
    }
    return NodeStmtElse {
        .scope = parse_stmt(false, loc_scp),
    };
}

NodeStmtExit Parser::parse_stmt_exit(const TokenType ttype,
    const std::unordered_map<std::string, SyntaxNode*>& loc_scp) {
    if (!peek(0).has_value()) {
        std::println(stderr, "[{}:{}] Error: Missing statement or expression after `exit`.",
                    peek().value().loc.line, peek().value().loc.col);
        exit(EXIT_FAILURE);
    }
    NodeStmtExit exit_;
    exit_.loc.line = peek().value().loc.line;
    exit_.loc.col = peek().value().loc.col;

    switch (ttype) {
        case TokenType::VAR_IDENT: {
            if (!loc_scp.contains(peek().value().value) &&
                !m_program.main.var_table.contains(peek().value().value)) {
                std::println(stderr, "[{}:{}]Error: Unknown symbol `{}`",
                    peek().value().loc.line, peek().value().loc.col,
                    peek().value().value);
                exit(EXIT_FAILURE);
            }

            exit_.exit_code = std::make_unique<SyntaxNode>(NodeIdentifier{
                .name = std::move(peek().value().value),
                .loc = { peek().value().loc.line, peek().value().loc.col }
            });
            next(); // eat identifier
            break;
        }
        default: { exit_.exit_code = parse_expr(); }
    }
    return exit_;
}

NodeScope Parser::parse_stmt(const bool is_prog,
       NodeScope& loc_scp) {
    NodeScope scope{};
    const auto clause_func = [&] () -> bool {
        return is_prog ? peek().has_value()
                       : !validate_token(0, TokenType::DELIM_RCURLY) && peek().has_value();
    };

    scope.var_table.insert(loc_scp.var_table.begin(), loc_scp.var_table.end());

    while (clause_func()) {
        const auto peeked = peek().value();
        next();

        switch (peeked.type) {
        case TokenType::DELIM_LCURLY: { // raw explicit scope
            auto [stmts, var_table, loc] = std::move(parse_stmt(false, scope));
            scope.var_table.insert(var_table.begin(), var_table.end());
            scope.stmts.insert(scope.stmts.end(), std::make_move_iterator(stmts.begin()), std::make_move_iterator(stmts.end()));
            break;
        }
        case TokenType::KW_LET:
        case TokenType::KW_MUT:{
            scope.stmts.emplace_back(parse_declaration(peeked.type, scope.var_table, is_prog));
            break;
        }
        case TokenType::KW_EXIT: {
            scope.stmts.emplace_back(parse_stmt_exit(peek().value().type, scope.var_table));
            break;
        }
        case TokenType::KW_IF: {
            //NOTE: always pass the scope var_table separately in all 
            //cases where scopes are involved
            auto if_res = std::move(parse_stmt_if(scope));
            scope.var_table.insert(if_res.scope.var_table.begin(), if_res.scope.var_table.end());
            scope.stmts.emplace_back(std::move(if_res));
            break;
        }
        case TokenType::KW_ELIF: {
            // if ((loc_scp.stmts.empty() ||
            //     (loc_scp.stmts.back().get_node_type() != NodeType::STMT_IF)) &&
            //     (loc_scp.stmts.back().get_node_type() != NodeType::STMT_ELIF)) {
            //     std::println(stderr, "[{}:{}]Error: `elif` must accompany an `if` conditional.",
            //                  peeked.loc.line, peeked.loc.col, scope.stmts.back().get_node_type() == NodeType::STMT_IF);
            //     exit(EXIT_FAILURE);
            // }
            auto elif_res = std::move(parse_stmt_elif(scope));
            scope.var_table.insert(elif_res.scope.var_table.begin(), elif_res.scope.var_table.end());
            scope.stmts.emplace_back(std::move(elif_res));
            break;
        }
        case TokenType::KW_ELSE: {
            // if ((loc_scp.stmts.empty() ||
            //     (loc_scp.stmts.back().get_node_type() != NodeType::STMT_IF)) &&
            //     (loc_scp.stmts.back().get_node_type() != NodeType::STMT_ELIF)) {
            //     std::println(stderr, "[{}:{}]Error: `else` must accompany an `if` or `elif` conditional.",
            //                  peeked.loc.line, peeked.loc.col, scope.stmts.back().get_node_type() == NodeType::STMT_IF);
            //     exit(EXIT_FAILURE);
            // }
            //NOTE: Make this more robust:
            // m_tok_ptr--;
            auto else_res = std::move(parse_stmt_else(scope));
            scope.var_table.insert(else_res.scope.var_table.begin(), else_res.scope.var_table.end());
            scope.stmts.emplace_back(std::move(else_res));
            break;
        }
        }
    }
    return scope;
}

NodeProgram&& Parser::create_program() {
    m_program.main = parse_stmt(true, m_program.main) ;
    return std::move(m_program);
}
