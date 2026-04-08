#include "parser.hpp"
#include "lexer.hpp"
#include "syntax-tree.hpp"

#include <cmath>
#include <cstddef>
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
    if (m_tokens.empty() ||
        m_tok_ptr >= m_tokens.size()) {
        return std::nullopt;
    }
    return m_tokens.at(m_tok_ptr++);
}

std::optional<Token> Parser::peek(const size_t offset = 0) const {
    if (m_tokens.empty() ||
        (offset + m_tok_ptr) >= m_tokens.size()) {
        return std::nullopt;
    }
    return m_tokens.at(m_tok_ptr + offset);
}

// Quick debug utility
void Parser::prnt_tok_seq(const size_t range = 0) {
    for (size_t i = 0; i <= range; ++i) {
        std::println("Token at {}: {}", i, peek(i).value().value);
    }
}

bool Parser::validate_token(const size_t offset,
                            const TokenType ttype = TokenType::NIL_,
                            const Op bop = Op::NIL_) const
{
    if (ttype == TokenType::NIL_ && bop == Op::NIL_) {
        assert(false && "Error: You forgot to specify a type to validate_token()");
    }
    if (bop != Op::NIL_) {
        return peek(offset).has_value() &&
        (bop == set_op(peek(offset).value().value));
    }
    return (peek(offset).has_value() &&
    peek(offset).value().type == ttype);
}

void Parser::check_semi() const {
    if (!validate_token(0, TokenType::DELIM_SEMI)) {
        std::println(stderr, "[{}:{}] Error: Expected `;`.",
                     peek().value().loc.line, peek().value().loc.col);
        exit(EXIT_FAILURE);
    }
}

std::tuple<float, float> Parser::get_infix_bpower(const Op op) {
    switch (op) {
    // Assignment (right associative)
    case Op::EQ:       return { 1, 0.9 };

    //logical
    case Op::LG_OR:    return { 2, 2.1 };
    case Op::LG_AND:   return { 3, 3.1 };

    //Bitwise
    case Op::BW_OR:    return { 4, 4.1 };
    case Op::BW_XOR:   return { 5, 5.1 };
    case Op::BW_AND:   return { 6, 6.1 };

    case Op::EQUIV:   [[fallthrough]];
    case Op::NEQUIV:   return { 7, 7.1 };

    case Op::LT:      [[fallthrough]];
    case Op::GT:      [[fallthrough]];
    case Op::LTE:     [[fallthrough]];
    case Op::GTE:      return { 8, 8.1 };

    case Op::LSL:     [[fallthrough]];
    case Op::LSR:      return { 9, 9.1 };

    case Op::SUB:     [[fallthrough]];
    case Op::ADD:      return { 10, 10.1 };
    case Op::MOD:     [[fallthrough]];
    case Op::DIV:     [[fallthrough]];
    case Op::MUL:      return { 11, 11.1 };

    // Power (right associative)
    case Op::PWR:      return { 12, 11.9 };
    default: assert(false && "Error: Unknown infix Op; Cannot get binding power.");
    }
}

float Parser::get_prefix_bpower(const Op op) {
    switch (op) {
    case Op::SUB:     [[fallthrough]];
    case Op::ADD:     return 13.f;

    case Op::BW_NOT:  [[fallthrough]];
    case Op::LG_NOT:  [[fallthrough]];
    case Op::INC:     [[fallthrough]];
    case Op::DEC:     return 14.f;
    default: {
        std::println(stderr, "[{}:{}] Error: Invalid prefix operator `{}`.", 
                    peek().value().loc.line,
                    peek().value().loc.col,
                    peek().value().value);
        exit(EXIT_FAILURE);
    }
    }
}

float Parser::get_postfix_bpower(const Op op) {
    switch (op) {
    case Op::INC:     [[fallthrough]];
    case Op::DEC:     return 15.f;
    default: {
        std::println(stderr, "[{}:{}] Error: Invalid postfix operator `{}`.",
                    peek().value().loc.line,
                    peek().value().loc.col,
                    peek().value().value);
        exit(EXIT_FAILURE);
    }
    }
}

bool Parser::is_assign_or_unary_op(const Op op) {
    switch (op) {
    case Op::INC:     [[fallthrough]];
    case Op::DEC:     [[fallthrough]];
    case Op::EQ:      [[fallthrough]];
    case Op::ADD_EQ:  [[fallthrough]];
    case Op::SUB_EQ:  [[fallthrough]];
    case Op::MUL_EQ:  [[fallthrough]];
    case Op::DIV_EQ:  [[fallthrough]];
    case Op::MOD_EQ:  [[fallthrough]];
    case Op::PWR_EQ:  [[fallthrough]];
    case Op::AND_EQ:  [[fallthrough]];
    case Op::OR_EQ:   [[fallthrough]];
    case Op::XOR_EQ:  [[fallthrough]];
    case Op::LSL_EQ:  [[fallthrough]];
    case Op::LSR_EQ:  return true;
    default:          return false;
    }
}

Fix Parser::get_fix() {
    m_tok_ptr--; // move back to get context on prev token
    if (
        validate_token(0, TokenType::VAR_IDENT) ||
        validate_token(0, TokenType::LIT_INT)
        ){
        m_tok_ptr++;
        return Fix::POSTFIX;
    }

    if (
        validate_token(2, TokenType::VAR_IDENT) ||
        validate_token(2, TokenType::LIT_INT) ||
        validate_token(2, TokenType::KW_EXIT)
        ){
        m_tok_ptr++;
        return Fix::PREFIX;
    }

    std::println(stderr, "[{}:{}] Error: Invalid unary expression `{}`.",
                 peek().value().loc.line,
                 peek().value().loc.line,
                 peek().value().value);
    exit(EXIT_FAILURE);
}

Op Parser::set_op(const std::string &optype) {
    if (optype == "+")       return Op::ADD;
    if (optype == "-")       return Op::SUB;
    if (optype == "*")       return Op::MUL;
    if (optype == "/")       return Op::DIV;
    if (optype == "%")       return Op::MOD;
    if (optype == "**")      return Op::PWR;
    if (optype == "++")      return Op::INC;
    if (optype == "--")      return Op::DEC;
    if (optype == "=")       return Op::EQ;
    if (optype == "+=")      return Op::ADD_EQ;
    if (optype == "-=")      return Op::SUB_EQ;
    if (optype == "*=")      return Op::MUL_EQ;
    if (optype == "/=")      return Op::DIV_EQ;
    if (optype == "%=")      return Op::MOD_EQ;
    if (optype == "**=")     return Op::PWR_EQ;
    if (optype == "&=")      return Op::AND_EQ;
    if (optype == "|=")      return Op::OR_EQ;
    if (optype == "^=")      return Op::XOR_EQ;
    if (optype == "<<=")     return Op::LSL_EQ;
    if (optype == ">>=")     return Op::LSR_EQ;
    if (optype == "<")       return Op::LT;
    if (optype == ">")       return Op::GT;
    if (optype == "<<")      return Op::LSL;
    if (optype == ">>")      return Op::LSR;
    if (optype == "<=")      return Op::LTE;
    if (optype == ">=")      return Op::GTE;
    if (optype == "==")      return Op::EQUIV;
    if (optype == "!=")      return Op::NEQUIV;
    if (optype == "|")       return Op::BW_OR;
    if (optype == "||")      return Op::LG_OR;
    if (optype == "~")       return Op::BW_NOT;
    if (optype == "&")       return Op::BW_AND;
    if (optype == "!")       return Op::LG_NOT;
    if (optype == "&&")      return Op::LG_AND;
    if (optype == "^")       return Op::BW_XOR;
    return Op::NIL_;
}

NodeVarDeclaration Parser::parse_declaration(const TokenType ttype,
    NodeScope& loc_scp, const bool is_reassign)
{
    if (!validate_token(0, TokenType::VAR_IDENT)) {
        std::println(
            "[{}:{}] Error: Expected identifier after `let`.",
            peek().value().loc.line,
            peek().value().loc.col);
        exit(EXIT_FAILURE);
    }
    if (!is_assign_or_unary_op(set_op(peek(1).value().value))) {
        std::println(
            "[{}:{}] Error: Expected `=` or valid operator after declaration `{}`",
            peek().value().loc.line,
            peek().value().loc.col,
            peek(1).value().value);
        exit(EXIT_FAILURE);
    }

    NodeVarDeclaration dec{};
    if (is_reassign) {
        dec.kind = VarType::MUT;
        dec.ident.name = peek().value().value;
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
            default: assert(false && "Error: Unknown variable declaration type.");
        }
        dec.ident = NodeIdentifier({
            .name = std::move(peek().value().value),
            .loc = peek().value().loc
        });
        if (m_program.main.var_table.contains(dec.ident.name) ||
            loc_scp.var_table.contains(dec.ident.name)) {
            std::println(stderr, "[{}:{}] Error: Re-definition of `{}`.",
                        peek().value().loc.line,
                        peek().value().loc.col - 1,
                        dec.ident.name);
            exit(EXIT_FAILURE);
        }
    }
    dec.value = parse_expr(false);
    check_semi();
    return dec;
}

std::unique_ptr<SyntaxNode> Parser::parse_expr(const bool chk_for_paren) {
    if (chk_for_paren){
        if (!validate_token(0, TokenType::DELIM_LPAREN)) {
            m_tok_ptr--;
            std::println(stderr, "[{}:{}] Error: Missing `(`.",
                         peek().value().loc.line,
                         peek().value().loc.col);
            exit(EXIT_FAILURE);
        }
    }
    return std::make_unique<SyntaxNode>(
        std::move(*parse_expr_impl(0))
    );
}

std::unique_ptr<NodeExpr> Parser::parse_expr_impl(const float min_rbp) {
    #pragma clang diagnostic ignored "-Wswitch"
    auto lhs = std::make_unique<NodeExpr>();
    if (peek().value().type == TokenType::DELIM_LPAREN) {
        next(); // '('
        lhs = parse_expr_impl(0); // subexpression

        if (!validate_token(0, TokenType::DELIM_RPAREN)) {
            std::println(stderr, "[{}:{}] Error: Missing `)`.",
                         peek().value().loc.line,
                         peek().value().loc.col);
            exit(EXIT_FAILURE);
        }
    }

    if (validate_token(0, TokenType::DELIM_SEMI)) return lhs;
    if (validate_token(0, TokenType::DELIM_COMMA)) return lhs; // arg lists
    if (validate_token(1, TokenType::DELIM_LCURLY)) return lhs; // for scopes

    auto [type, value, loc] = peek().value();

    switch (type) {
    case TokenType::LIT_INT: {
        lhs->atom.emplace<NodeIntLiteral>(
            NodeIntLiteral({
                .value = std::stoi(value),
                .loc = loc
            })
        );
        lhs->var_count++;
        next(); // eat lit
        break;
    }
    case TokenType::VAR_IDENT: {
        lhs->atom.emplace<NodeIdentifier>(
            NodeIdentifier({
                .name = std::move(value),
                .loc = loc
            })
        );
        lhs->var_count++;
        next(); // eat ident
        break;
    }
    case TokenType::OP: {
        const Op op = set_op(value);

        switch(op) {
        case Op::SUB:{
            lhs->is_negative = true;
            float bp = get_prefix_bpower(op);
            next();
            auto rhs = parse_expr_impl(bp);
            auto node = std::make_unique<NodeExpr>();

            node->op = op;
            node->loc = lhs->loc;
            node->var_count += lhs->var_count + rhs->var_count;
            node->is_negative = lhs->is_negative;
            node->lhs = std::move(lhs);
            node->rhs = std::move(rhs);

            lhs = std::move(node);
            break;
        }
        case Op::ADD:{
            lhs->is_positive = true;
            float bp = get_prefix_bpower(op);
            next();
            auto rhs = parse_expr_impl(bp);
            auto node = std::make_unique<NodeExpr>();

            node->op = op;
            node->loc = lhs->loc;
            node->var_count += lhs->var_count + rhs->var_count;
            node->is_positive = lhs->is_positive;
            node->lhs = std::move(lhs);
            node->rhs = std::move(rhs);

            lhs = std::move(node);
            break;
        }
        case Op::INC: [[fallthrough]];
        case Op::DEC:{
            lhs->fix = Fix::PREFIX;
            float bp = get_prefix_bpower(op);
            next();
            auto rhs = parse_expr_impl(bp);
            auto node = std::make_unique<NodeExpr>();

            node->op = op;
            node->loc = lhs->loc;
            node->var_count += lhs->var_count + rhs->var_count;
            node->fix = lhs->fix;
            node->lhs = std::move(lhs);
            node->rhs = std::move(rhs);

            lhs = std::move(node);
            break;
        }
        }
        break;
        }
        default: next();
    }

    if (
        !validate_token(0, TokenType::OP) &&
        !validate_token(0, TokenType::DELIM_SEMI) &&
        !validate_token(0, TokenType::DELIM_RPAREN) &&
        !validate_token(0, TokenType::DELIM_COMMA)
        ) {
        std::println(
            stderr, "[{}:{}] Error: Expected binary operator before `{}`.",
            peek().value().loc.line,
            peek().value().loc.col,
            peek().value().value);
        exit(EXIT_FAILURE);
    }

    while (validate_token(0, TokenType::OP)) {
        Op op = set_op(peek().value().value);
        bool is_assign = false;
        float lbp = INFINITY, rbp = INFINITY;

        // deconstruct combo assign op
        switch (op) {
        case Op::ADD_EQ: {
            op = Op::ADD;
            break;
        }
        case Op::SUB_EQ: {
            op = Op::SUB;
            break;
        }
        case Op::MUL_EQ: {
            op = Op::MUL;
            break;
        }
        case Op::DIV_EQ: {
            op = Op::DIV;
            break;
        }
        case Op::PWR_EQ: {
            op = Op::PWR;
            break;
        }
        case Op::MOD_EQ: {
            op = Op::MOD;
            break;
        }
        case Op::AND_EQ: {
            op = Op::BW_AND;
            break;
        }
        case Op::OR_EQ: {
            op = Op::BW_OR;
            break;
        }
        case Op::XOR_EQ: {
            op = Op::BW_XOR;
            break;
        }
        case Op::LSL_EQ: {
            op = Op::LSL;
            break;
        }
        case Op::LSR_EQ: {
            op = Op::LSR;
            break;
        }
        case Op::DEC: [[fallthrough]];
        case Op::INC: {
            lhs->fix = Fix::POSTFIX;
            lbp = get_postfix_bpower(op);
            break;
        }
        }

        //Binary ops
        if (lbp == INFINITY)
            std::tie(lbp, rbp) = get_infix_bpower(op);
        if (lbp < min_rbp) break;
        next(); // eat op

        auto rhs = parse_expr_impl(rbp);
        auto node = std::make_unique<NodeExpr>();

        node->op = op;
        node->loc = lhs->loc;
        node->var_count += lhs->var_count + rhs->var_count;
        node->fix = lhs->fix;
        node->lhs = std::move(lhs);
        node->rhs = std::move(rhs);

        lhs = std::move(node);
    }
    return lhs;
}

NodeStmtIf Parser::parse_stmt_if(NodeScope& loc_scp) {
    const auto loc = peek().value().loc;
    std::unique_ptr<SyntaxNode> cond { parse_expr(true) };

    if (!validate_token(1, TokenType::DELIM_LCURLY)) {
        std::println(stderr, "[{}:{}] Error: Missing `{{`.",
                        peek(1).value().loc.line,
                        peek(1).value().loc.col);
        exit(EXIT_FAILURE);
    }

    return NodeStmtIf {
        .cond = std::move(cond),
        .scope = parse_stmt(false, loc_scp),
        .n_elif = parse_stmt_elif(loc_scp),
        .n_else = parse_stmt_else(loc_scp),
        .loc = loc
    };
}

std::vector<NodeStmtElif> Parser::parse_stmt_elif(NodeScope& loc_scp) {
    std::vector<NodeStmtElif> elif_vec{};
    if (!validate_token(1, TokenType::KW_ELIF)) return elif_vec;

    next(); // eat '}'
    while (validate_token(0, TokenType::KW_ELIF)) {
        NodeStmtElif elif {};
        const auto loc = next().value().loc; // eat 'elif'
        elif.cond = parse_expr(true);

        if (!validate_token(1, TokenType::DELIM_LCURLY)) {
            std::println(stderr, "[{}:{}] Error: Missing `{{`.",
                         peek(1).value().loc.line,
                         peek(1).value().loc.col);
            exit(EXIT_FAILURE);
        }

        elif.scope = parse_stmt(false, loc_scp);
        elif.loc = loc;
        elif_vec.emplace_back(std::move(elif));

        if (!validate_token(0, TokenType::DELIM_RCURLY)) {
            std::println(
                stderr, "[{}:{}] Error: Missing `}}`.",
                peek().value().loc.line,
                peek().value().loc.col);
            exit(EXIT_FAILURE);
        }
        next(); // eat '}'
    }

    m_tok_ptr--; // allow for else check
    return elif_vec;
}

std::optional<NodeStmtElse> Parser::parse_stmt_else(NodeScope& loc_scp) {
    next(); // eat '}' from prev scope

    if (!validate_token(0, TokenType::KW_ELSE)) return std::nullopt;
    if (!validate_token(1, TokenType::DELIM_LCURLY)) {
        std::println(
            stderr, "[{}:{}] Error: Missing `{{`.",
            peek(1).value().loc.line,
            peek(1).value().loc.col);
        exit(EXIT_FAILURE);
    }
    const auto loc = next().value().loc; // eat `else`
    NodeScope else_scp = parse_stmt(false, loc_scp);

    if (!validate_token(0, TokenType::DELIM_RCURLY)) {
        std::println(stderr, "[{}:{}] Error: Missing `}}`.",
                     peek().value().loc.line,
                     peek().value().loc.col);
        exit(EXIT_FAILURE);
    }
    next(); // '}'
    return NodeStmtElse {
        .scope = std::move(else_scp),
        .loc = loc,
    };
}

NodeStmtExit Parser::parse_stmt_exit(const TokenType ttype,
    const std::unordered_map<std::string, NodeVarDeclaration*>& loc_scp) {
    if (!peek(0).has_value()) {
        std::println(
            stderr, "[{}:{}] Error: Missing statement or expression after `exit`.",
            peek().value().loc.line,
            peek().value().loc.col);
        exit(EXIT_FAILURE);
    }
    NodeStmtExit exit_;
    exit_.loc.line = peek().value().loc.line;
    exit_.loc.col = peek().value().loc.col;
    exit_.exit_code = parse_expr(false);
    return exit_;
}

NodeStmtWhile Parser::parse_stmt_while(NodeScope& loc_scp) {
    std::unique_ptr<SyntaxNode> cond { parse_expr(true) };

    if (!validate_token(1, TokenType::DELIM_LCURLY)) {
        std::println(
            stderr, "[{}:{}] Error: Invalid conditional.",
                     peek(1).value().loc.line,
                     peek(1).value().loc.col
        );
        exit(EXIT_FAILURE);
    }
    return NodeStmtWhile {
        .cond = std::move(cond),
        .scope = parse_stmt(false, loc_scp),
        .loc = peek().value().loc,
    };
}

NodeFunc Parser::parse_stmt_fn(NodeScope& loc_scp) {
    NodeFunc func {};
    if (!validate_token(0, TokenType::VAR_IDENT)) {
        std::println(
            stderr, "[{}:{}] Error: Expected identifier after function declaration.",
            peek().value().loc.line,
            peek().value().loc.col);
    }
    if (!validate_token(1, TokenType::DELIM_LPAREN)) {
        std::println(
            stderr, "[{}:{}] Error: Expected '(' after function declaration.",
            peek().value().loc.line,
            peek().value().loc.col);
    }
    func.ident = NodeIdentifier({
        .name = peek().value().value,
        .loc = peek().value().loc
    });
    func.loc = peek().value().loc;
    next(); // eat ident
    next(); // eat '('

    while (!validate_token(1, TokenType::DELIM_LCURLY)) {
        const auto [type, value, loc] = peek().value();

        if (type == TokenType::DELIM_COMMA) {
            next();
            continue;
        }
        if (type != TokenType::VAR_IDENT) {
            std::println(
                stderr, "[{}:{}] Error: Expected identifier in function argument initializer.",
                loc.line, loc.col);
            exit(EXIT_FAILURE);
        }
        func.args.emplace_back(
            NodeIdentifier( value, loc )
        );
        next();
    }
    func.scope = parse_stmt(false, loc_scp);
    next(); // eat '}'
    return func;
}

NodeScope Parser::parse_stmt(const bool is_prog, NodeScope& loc_scp) {
    NodeScope scope{};
    const auto clause_func = [&] () -> bool {
        return is_prog
        ? peek().has_value()
        : !validate_token(0, TokenType::DELIM_RCURLY) &&
        peek().has_value();
    };
    scope.var_table.insert(loc_scp.var_table.begin(),
                           loc_scp.var_table.end());
    scope.fn_table.insert(std::make_move_iterator(loc_scp.fn_table.begin()),
                           std::make_move_iterator(loc_scp.fn_table.end()));

    while (clause_func()) {
        const auto peeked = peek().value();
        next();

        switch (peeked.type) {
            case TokenType::DELIM_LCURLY: { // raw explicit scope
                auto [stmts, var_table, fn_table, loc] = parse_stmt(false, scope);
                scope.stmts.insert(
                    scope.stmts.end(),
                    std::make_move_iterator(stmts.begin()),
                    std::make_move_iterator(stmts.end())
                );
                break;
            }
            case TokenType::DELIM_RCURLY: {
                std::println(stderr, "[{}:{}] Error: Extraneous `}}`.",
                             peeked.loc.line, peeked.loc.col);
                exit(EXIT_FAILURE);
            }
            case TokenType::KW_LET: [[fallthrough]];
            case TokenType::KW_MUT:{
                auto res = parse_declaration(peeked.type, scope, false);
                const std::string id_name = res.ident.name;

                scope.stmts.emplace_back(
                    std::make_unique<SyntaxNode>(std::move(res))
                );
                if (is_prog) {
                    m_program.main.var_table
                        .insert_or_assign(id_name,
                            &std::get<NodeVarDeclaration>(scope.stmts.back()->m_node)
                        );
                    break;
                }
                scope.var_table
                        .insert_or_assign(id_name,
                            &std::get<NodeVarDeclaration>(scope.stmts.back()->m_node)
                        );
                break;
            }
            case TokenType::VAR_IDENT: {

                const bool var_in_loc_scp = scope.var_table.contains(peeked.value);
                const bool var_in_glb_scp = m_program.main.var_table.contains(peeked.value);

                bool is_call = false;
                if (peek().value().type == TokenType::DELIM_LPAREN) is_call = true;

                if (var_in_loc_scp || var_in_glb_scp) {
                    const auto kind = var_in_loc_scp
                        ? (scope.var_table.at(peeked.value))->kind
                        : (m_program.main.var_table.at(peeked.value))->kind;

                    if (kind != VarType::MUT) {
                        std::println(
                            stderr, "[{}:{}] Error: Cannot reassign variable of type `let`; "
                                    "Did you mean to use `mut`?",
                            peeked.loc.line, peeked.loc.col
                        );
                        exit(EXIT_FAILURE);
                    }
                    m_tok_ptr--;

                    auto res = parse_declaration(peeked.type, scope, true);
                    const std::string id = res.ident.name;

                    scope.stmts.emplace_back(
                        std::make_unique<SyntaxNode>(std::move(res))
                    );

                    var_in_loc_scp
                        ? scope.var_table
                        .insert_or_assign(id,
                            &std::get<NodeVarDeclaration>(scope.stmts.back()->m_node)
                        )
                        : m_program.main.var_table
                        .insert_or_assign(id,
                            &std::get<NodeVarDeclaration>(scope.stmts.back()->m_node)
                        );

                    break;
                }
                if (is_call) {
                    const bool fn_in_loc_scp = scope.fn_table.contains(peeked.value);
                    const auto& fn = fn_in_loc_scp
                                    ? scope.fn_table.at(peeked.value)
                                    : m_program.main.fn_table.at(peeked.value);
                    scope.stmts.emplace_back(
                        std::make_unique<SyntaxNode>(
                            NodeCall({
                                .ident = fn->ident,
                                .args = [&]() -> std::vector<std::unique_ptr<SyntaxNode>> {
                                    next(); // eat '('

                                    std::vector<std::unique_ptr<SyntaxNode>> args{};
                                    while (!validate_token(0, TokenType::DELIM_RPAREN)) {
                                        if (validate_token(0, TokenType::DELIM_COMMA)) next();
                                        args.emplace_back(parse_expr(false));
                                    }
                                    if (peek().value().type != TokenType::DELIM_RPAREN) {
                                        std::println(
                                            stderr, "[{}:{}] Error: Expected `)`.",
                                            peek().value().loc.line, peek().value().loc.col);
                                        exit(EXIT_FAILURE);
                                    }
                                    if (args.size() != fn->args.size()) {
                                        std::println(stderr, "[{}:{}] Error: Expected {} arguments "
                                                     "for call to function `{}`; Received {}.",
                                                     fn->loc.line, fn->loc.col, fn->args.size(),
                                                     fn->ident.name, args.size());
                                    }
                                    next(); // eat ')'
                                    check_semi();
                                    return args;
                                }(),
                            })
                        )
                    );
                    break;
                }
                std::println(stderr, "[{}:{}] Error: Expected identifier.",
                             peeked.loc.line, peeked.loc.col);
                exit(EXIT_FAILURE);
            }
            case TokenType::KW_EXIT: {
                scope.stmts.emplace_back(
                    std::make_unique<SyntaxNode>(
                        parse_stmt_exit(peek().value().type, scope.var_table)));
                break;
            }
            case TokenType::KW_IF: {
                scope.stmts.emplace_back(
                    std::make_unique<SyntaxNode>(parse_stmt_if(scope))
                );
                break;
            }
            case TokenType::KW_ELIF: [[fallthrough]];
            case TokenType::KW_ELSE: {
                std::println("Error value: {}", peek().value().value);
                std::println(
                    stderr, "[{}:{}] Error: Expected accompanying `if` statement.",
                    peek().value().loc.line,
                    peek().value().loc.col - 5
                );
                exit(EXIT_FAILURE);
            }
            case TokenType::KW_WHILE: {
                auto[cond, scp, loc]  = parse_stmt_while(scope);
                scope.stmts.emplace_back(
                    std::make_unique<SyntaxNode>(
                        NodeStmtWhile(std::move(cond), std::move(scp))
                    ));
                next(); // `}`
                break;
            }
            case TokenType::KW_FN: {
                auto res = parse_stmt_fn(scope);
                const std::string id = res.ident.name;

                scope.stmts.emplace_back(
                        std::make_unique<SyntaxNode>( std::move(res) )
                    );
                if (is_prog) {
                    m_program.main.fn_table
                        .insert_or_assign(id,
                            ( &std::get<NodeFunc>(scope.stmts.back()->m_node) )
                        );
                    break;
                }
                scope.fn_table
                    .insert_or_assign(id,
                        ( &std::get<NodeFunc>(scope.stmts.back()->m_node) )
                    );
                break;
            }
        }
    }
    return scope;
}

NodeProgram&& Parser::create_program() {
    NodeScope empty{};
    m_program.main.stmts = parse_stmt(true, empty).stmts;
    return std::move(m_program);
}
