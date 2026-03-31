#include "parser.hpp"
#include "lexer.hpp"
#include "syntax-tree.hpp"

#include <cstdio>
#include <cstdlib>
#include <iterator>
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
    if (bop != BinOp::NIL_) {
        return peek(offset).has_value() && (bop == set_op(peek(offset).value().value));
    }
    return (peek(offset).has_value() && peek(offset).value().type == ttype);
}

void Parser::check_semi() const {
    if (!validate_token(0, TokenType::DELIM_SEMI)) {
        std::println(stderr, "[{}:{}] Error: Expected `;`.", 
                     peek().value().loc.line, peek().value().loc.col);
        exit(EXIT_FAILURE);
    }
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
    case BinOp::BW_XOR:  { return {5, 5.1}; }
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
    #pragma clang diagnostic ignored "-Wswitch"

    const NodeScope& loc_scp, const bool is_reassign) {

    if (!validate_token(0, TokenType::VAR_IDENT)) {
        std::println(
            "[{}:{}] Error: Expected identifier after `let`.",
            peek().value().loc.line, peek().value().loc.col);
        std::println("This is what failed: {}", peek().value().value);
        exit(EXIT_FAILURE);
    }
    if (!validate_token(1, TokenType::NIL_, BinOp::EQ)) {
        std::println("[{}:{}] Error: Expected `=` after declaration `{}`",
            peek().value().loc.line, peek().value().loc.col, peek().value().value);
        exit(EXIT_FAILURE);
    }

    NodeVarDeclaration dec{};
    if (is_reassign) {
        dec.kind = VarType::MUT;
        dec.ident.name = peek().value().value;
        next(); // eat ident
    } else {
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
            .loc = peek().value().loc
        });

        if (m_program.main.var_table.contains(dec.ident.name) || loc_scp.var_table.contains(dec.ident.name)) {
            std::println(stderr, "[{}:{}] Error: Re-definition of `{}`.",
                peek().value().loc.line, peek().value().loc.col - 1, dec.ident.name);
            exit(EXIT_FAILURE);
        }
        next(); // eat ident
    }
    next(); // eat `=`
    dec.value = parse_expr(false);

    check_semi();

    return dec;
}

std::unique_ptr<SyntaxNode> Parser::parse_expr(const bool chk_for_paren) {
    if (chk_for_paren){
        if (!validate_token(0, TokenType::DELIM_LPAREN)) {
            m_tok_ptr--;
            std::println(stderr, "[{}:{}] Error: Missing `(`.", peek().value().loc.line, peek().value().loc.col);
            exit(EXIT_FAILURE);
        }
    }
    return std::make_unique<SyntaxNode>(std::move(*parse_expr_impl(0)));
}

std::unique_ptr<NodeBinaryExpr> Parser::parse_expr_impl(const float min_rbp) {
    #pragma clang diagnostic ignored "-Wswitch"

    auto lhs = std::make_unique<NodeBinaryExpr>();
    if (peek().value().type == TokenType::DELIM_LPAREN) {
        next(); // '('
        lhs = parse_expr_impl(0); // subexpression

        if (!validate_token(0, TokenType::DELIM_RPAREN)) {
            std::println(stderr, "[{}:{}] Error: Missing `)`.", peek().value().loc.line, peek().value().loc.col);
            exit(EXIT_FAILURE);
        }
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

    if (!validate_token(0, TokenType::BIN_OP) && !validate_token(0, TokenType::DELIM_SEMI) 
        && !validate_token(0, TokenType::DELIM_RPAREN)){
        std::println(stderr, "[{}:{}] Error: Expected binary operator before `{}`.", 
                     peek().value().loc.line, peek().value().loc.col, peek().value().value);
        exit(EXIT_FAILURE);
    }

    while (validate_token(0, TokenType::BIN_OP)) {
        const BinOp op = set_op(peek().value().value);

        if (op == BinOp::EQ) {
            std::println(stderr, "[{}:{}] Error: Operator `=` not allowed in expression.", 
                         peek().value().loc.line, peek().value().loc.col);
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
    std::unique_ptr<SyntaxNode> cond { parse_expr(true) };

    if (!validate_token(1, TokenType::DELIM_LCURLY)) {
        std::println(stderr, "[{}:{}] Error: .", peek(1).value().loc.line, peek(1).value().loc.col);
        exit(EXIT_FAILURE);
    }

    return NodeStmtIf {
        .cond = std::move(cond),
        .scope = parse_stmt(false, loc_scp),
    };
}

NodeStmtElif Parser::parse_stmt_elif(NodeScope& loc_scp) {
    std::unique_ptr cond { parse_expr(true) };

    if (!validate_token(1, TokenType::DELIM_LCURLY)) {
        std::println("Token here is: {}", peek(1).value().value);
        std::println(stderr, "[{}:{}] Error: Missing `{{`.", peek(1).value().loc.line, peek(1).value().loc.col);
        exit(EXIT_FAILURE);
    }
    return NodeStmtElif {
        .cond = std::move(cond),
        .scope = parse_stmt(false, loc_scp),
        .loc = peek().value().loc,
    };
}

NodeStmtElse Parser::parse_stmt_else(NodeScope& loc_scp) {
    if (!peek().has_value()) {
        std::println(stderr, "Error: Invalid conditional.");
        exit(EXIT_FAILURE);
    }
    next(); // eat `else`
    return NodeStmtElse {
        .scope = parse_stmt(false, loc_scp),
        .loc = peek().value().loc,
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
                std::println(stderr, "[{}:{}] Error: Unknown symbol `{}`",
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
        default: { exit_.exit_code = parse_expr(false); }
    }
    return exit_;
}

NodeStmtWhile Parser::parse_stmt_while(NodeScope& loc_scp) {
    std::unique_ptr<SyntaxNode> cond { parse_expr(true) };

    if (!validate_token(1, TokenType::DELIM_LCURLY)) {
        std::println(stderr, "[{}:{}] Error: Invalid conditional.", peek(1).value().loc.line, peek(1).value().loc.col);
        exit(EXIT_FAILURE);
    }
    return NodeStmtWhile {
        .cond = std::move(cond),
        .scope = parse_stmt(false, loc_scp),
        .loc = peek().value().loc,
    };
}

NodeScope Parser::parse_stmt(const bool is_prog, NodeScope& loc_scp) {
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
                auto [stmts, var_table, loc] = parse_stmt(false, scope);
                scope.stmts.insert(scope.stmts.end(),
                    std::make_move_iterator(stmts.begin()),
                    std::make_move_iterator(stmts.end()));
                break;
            }
            case TokenType::DELIM_RCURLY: {
                std::println(stderr, "[{}:{}] Error: Extraneous `}}`.", peeked.loc.line, peeked.loc.col);
                exit(EXIT_FAILURE);
            }
            case TokenType::KW_LET: [[fallthrough]];
            case TokenType::KW_MUT:{
                auto res = parse_declaration(peeked.type, scope, false);
                const std::string id_name = res.ident.name;
                scope.stmts.emplace_back(std::make_unique<SyntaxNode>(std::move(res)));
                if (is_prog) {
                    m_program.main.var_table.insert_or_assign(id_name, scope.stmts.back().get());
                    break;
                }
                scope.var_table.insert_or_assign(id_name, scope.stmts.back().get());
                break;
            }
            case TokenType::VAR_IDENT: {
                const bool in_loc_scp = scope.var_table.contains(peeked.value);
                const bool in_glb_scp = m_program.main.var_table.contains(peeked.value);

                if (in_loc_scp || in_glb_scp) {
                    const auto kind = in_loc_scp ? std::get_if<NodeVarDeclaration>(&scope.var_table.at(peeked.value)->m_node)->kind
                                                : std::get_if<NodeVarDeclaration>(&m_program.main.var_table.at(peeked.value)->m_node)->kind;

                    if (kind != VarType::MUT) {
                        std::println(stderr, "[{}:{}] Error: Cannot reassign variable of type `let`; "
                                            "Did you mean to use `mut`?",
                            peeked.loc.line, peeked.loc.col);
                        exit(EXIT_FAILURE);
                    }
                    m_tok_ptr--;

                    auto res = parse_declaration(peeked.type, scope, true);
                    const std::string id_name = res.ident.name;

                    scope.stmts.emplace_back(std::make_unique<SyntaxNode>(std::move(res)));

                    in_loc_scp ? scope.var_table.insert_or_assign(id_name, scope.stmts.back().get())
                            : m_program.main.var_table.insert_or_assign(id_name, scope.stmts.back().get());

                    break;
                }
                std::println(stderr, "[{}:{}] Error: Expected identifier.", peeked.loc.line, peeked.loc.col);
                exit(EXIT_FAILURE);
            }
            case TokenType::KW_EXIT: {
                scope.stmts.emplace_back(
                    std::make_unique<SyntaxNode>(
                        parse_stmt_exit(peek().value().type, scope.var_table)));
                break;
            }
            case TokenType::KW_IF: {
                auto[cond, scp, loc]  = parse_stmt_if(scope);
                scope.stmts.emplace_back(std::make_unique<SyntaxNode>(NodeStmtIf(std::move(cond), std::move(scp))));
                next(); // }

                while (validate_token(0, TokenType::KW_ELIF)) {
                    next();
                    auto[cond, scp, loc]  = parse_stmt_elif(scope);
                    scope.stmts.emplace_back(std::make_unique<SyntaxNode>(NodeStmtElif(std::move(cond), std::move(scp))));
                    next(); // }
                }

                if (validate_token(0, TokenType::KW_ELSE)) {
                    auto[scp, loc] = parse_stmt_else(scope);
                    scope.stmts.emplace_back(std::make_unique<SyntaxNode>(NodeStmtElse(std::move(scp))));
                    next(); // }
                }
                break;
            }
            case TokenType::KW_ELIF: [[fallthrough]];
            case TokenType::KW_ELSE: {
                std::println("Error value: {}", peek().value().value);
                std::println(stderr, "[{}:{}] Error: Expected accompanying `if` statement.",
                    peek().value().loc.line, peek().value().loc.col - 5);
                exit(EXIT_FAILURE);
            }
            case TokenType::KW_WHILE: {
                auto[cond, scp, loc]  = parse_stmt_while(scope);
                scope.stmts.emplace_back(std::make_unique<SyntaxNode>(NodeStmtWhile(std::move(cond), std::move(scp))));
                next(); // `}`
            }
        }
    }
    return scope;
}

NodeProgram&& Parser::create_program() {
    NodeScope empty{};
    m_program.main = parse_stmt(true, empty);
    return std::move(m_program);
}
