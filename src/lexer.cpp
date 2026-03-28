#include "lexer.hpp"
#include "include/utils.hpp"
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <print>
#include <sstream>
#include <string>
#include <vector>

/* */
Lexer::Lexer(const std::string& path) noexcept : m_ifs(path)
{
    if (!m_ifs.is_open()) {
        std::println(stderr, ERR_FILE);
        exit(EXIT_FAILURE);
    }
}

/* */
std::optional<Token> Lexer::peek(const size_t offset) const {
    if (m_tokens.empty() || 
        (m_token_ptr + offset) >= m_tokens.size()) {
        return std::nullopt;
    }
    return m_tokens.at(m_token_ptr + offset);
}

/* */
std::optional<Token> Lexer::consume() {
    if (m_tokens.empty() || 
        (m_token_ptr) >= m_tokens.size()) {
        return std::nullopt;
    }
    return m_tokens.at(m_token_ptr++);
}

/* */
std::vector<Token> Lexer::tokenize() {
    char ch;
    std::string buf;
    size_t line_cnt = 1;
    size_t col_cnt = 1;
    size_t tok_idx = 0;

    // Build tokens list
    while (m_ifs.get(ch)) {
        if (std::isalnum(ch)) {
            col_cnt++;
            buf.push_back(ch);
        } else if (std::isspace(ch)) {
            col_cnt++;
            if (!buf.empty()) {
                m_tokens.emplace_back(classify_token(buf));
                m_tokens.at(tok_idx).loc = { line_cnt, col_cnt };

                tok_idx++;
                buf.clear();
            }
            if (ch == '\n') line_cnt++, col_cnt = 0;
        } else if (std::ispunct(ch)) { // Operators & Symbols
            col_cnt++;
            if (!buf.empty()) {
                m_tokens.emplace_back(classify_token(buf));
                m_tokens.at(tok_idx).loc = { line_cnt, col_cnt };
                tok_idx++;
                buf.clear();
            }
            std::string o{ch};
            m_tokens.emplace_back(classify_token(o));
            m_tokens.at(tok_idx).loc = { line_cnt, col_cnt };
            tok_idx++;
        }
    }
    for (const Token &tok : m_tokens) {
        // std::println("Token value: {}", tok.value);
    }

    return m_tokens;
}

/* */
Token Lexer::classify_token(const std::string &buf) noexcept {
    Token tok{};

    if (buf.find_first_not_of("0123456789"))   { tok.type = TokenType::LIT_INT; }
    else if (buf.find_first_not_of("+-*/"))    { tok.type = TokenType::BIN_OP; }

    else if (buf == ";")    { tok.type = TokenType::DELIM_SEMI; } 
    else if (buf == "exit") { tok.type = TokenType::KW_EXIT; } 
    else if (buf == "let")  { tok.type = TokenType::KW_LET; } 
    else if (buf == "=")    { tok.type = TokenType::OP_EQUALS; } 
    else if (buf == "(")    { tok.type = TokenType::DELIM_LPAREN; } 
    else if (buf == ")")    { tok.type = TokenType::DELIM_RPAREN; } 
    else                    { tok.type = TokenType::VAR_IDENT; }

    tok.value = buf;
    return tok;
}

