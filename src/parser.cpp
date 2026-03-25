#include "parser.hpp"
#include "syntax-tree.hpp"
#include "lexer.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <optional>
#include <utility>
#include <vector>


Parser::Parser(std::vector<Token>&& toks) noexcept: 
    m_tokens(std::move(toks))
{
};
std::optional<Token> Parser::next() {
    if (m_tokens.empty() || m_tok_ptr >= m_tokens.size()) {
        return std::nullopt;
    }
    return m_tokens.at(m_tok_ptr++);
}
std::optional<Token> Parser::peek(size_t offset = 0) const {
    if (m_tokens.empty() || (offset + m_tok_ptr) >= m_tokens.size()) {
        return std::nullopt;
    }
    return m_tokens.at(m_tok_ptr + offset);
}

bool Parser::validate_token(const size_t offset, const TokenType ttype) const {
    return (peek(offset).has_value() && peek(offset).value().type == ttype);
}

std::tuple<float, float> Parser::get_binding_power(BinOp bop) const { switch (bop) {
        case BinOp::SUB:
        case BinOp::ADD:  { return {1, 1.1}; }
        case BinOp::DIV:
        case BinOp::MULT: { return {2, 2.1}; }
    }
}

BinOp Parser::set_op(const std::string& optype) const {
    if (optype == "+") {
        return BinOp::ADD;
    }
    else if (optype == "-") {
        return BinOp::SUB;
    }
    else if (optype == "*") {
        return BinOp::MULT;
    }
    else if (optype == "/") {
        return BinOp::DIV;
    }
    std::println(stderr, "Error: Invalid binary operator.");
    exit(EXIT_FAILURE);
}

NodeVarDeclaration Parser::parse_declaration(VarType vtype) {
    NodeVarDeclaration dec{};
    dec.value = std::make_unique<SyntaxNode>();
    switch (vtype) {

        case VarType::MUT: {
            // dec->kind = VarType::MUT;
            // dec->ident = NodeIdentifier({ .name = std::move(peek().value().value) });
            // dec->value->expr->m_node = parse_expr(0);
            dec.kind = VarType::MUT;
            dec.ident = NodeIdentifier({ .name = std::move(next().value().value) });
            next(); // eat `=`
            std::println("ident: {}", dec.ident.name);
            auto res = std::move(*parse_expr(0));
            res.print();
            // std::println();
            std::fflush(stdout);
             
            dec.value->m_node.emplace<NodeBinaryExpr>(std::move(res));

            // auto node = std::move((std::get<std::unique_ptr<NodeBinaryExpr>>( dec.value->expr->m_node )));
            // auto res = std::move(std::get<std::unique_ptr<NodeBinaryExpr>>(dec.value->expr->m_node));
            // res->print(0);

            // size_t cur_pos = 3; // pos of 1st value in expr
            //
            // while (peek(cur_pos).has_value() && peek(cur_pos).value().type != TokenType::DELIM_SEMI) {
            //     auto cur_tok = peek(cur_pos).value();
            //     switch (cur_tok.type) {
            //         case TokenType::VAR_IDENT: {
            //             dec.value->expr->m_node = NodeIdentifier({ .name = std::move(cur_tok.value) });
            //             cur_pos++;
            //             break;
            //         }
            //
            //         case TokenType::OP_ADD:  
            //         case TokenType::OP_SUB:  
            //         case TokenType::OP_MULT:  
            //         case TokenType::OP_DIV:  {
            //             dec.value->expr->m_node = parse_expr(0);
            //             cur_pos += 2;
            //             break;
            //         }
            //         default: { cur_pos++; }
            //     }
            // }
            break;
        }
    }

    // return std::move(*dec);
    return dec;
}

// NodeBinaryExpr* Parser::parse_expr(float min_rbp) {
std::unique_ptr<NodeBinaryExpr> Parser::parse_expr(float min_rbp) {
    if (!peek().has_value()) {
        std::println(stderr, "Error: Invalid expression.");
        exit(EXIT_FAILURE);
    }
    BinOp op;
    auto lhs = std::make_unique<NodeBinaryExpr>();
    // auto lhs = new NodeBinaryExpr();
    if (peek().value().type == TokenType::DELIM_LPAREN) {
        next();
        lhs = parse_expr(0);
    }
    lhs->atom = next().value().value;

    while (peek().has_value() && peek().value().type == TokenType::BIN_OP) {
        op = set_op(peek().value().value);

        auto [lbp, rbp] = get_binding_power(op);

        if (lbp < min_rbp) {
            break;
        }
        next();

        auto rhs = parse_expr(rbp);
        auto node = std::make_unique<NodeBinaryExpr>();
        // auto node = new NodeBinaryExpr();
        node->op = op;
        node->lhs = std::move(lhs);
        node->rhs = std::move(rhs);
        // node->lhs = (lhs);
        // node->rhs = (rhs);

        lhs = std::move(node);
        // lhs = (node);
    }
    return lhs;
}

NodeProgram&& Parser::create_program() {
    while (peek().has_value()) {
        auto peeked = peek().value();



        switch (peeked.type) {
            case TokenType::KW_LET: {
                if (!validate_token(1, TokenType::VAR_IDENT)) {
                    std::println("Error: Missing declaration identifier after `let`.");
                    exit(EXIT_FAILURE);
                }
                if (!validate_token(2, TokenType::OP_EQUALS)) {
                    std::println("Error: Missing `=` after variable declaration?");
                    exit(EXIT_FAILURE);
                }
                next();

                parse_declaration(VarType::MUT);
            }
        }
    }
}
