#include "lexer.hpp"
#include "include/utils.hpp"
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <print>
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
std::optional<Token> Lexer::peek(size_t offset) const {
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

    // Build tokens list
    while (m_ifs.get(ch)) {
        if (std::isalnum(ch)) {
            buf.push_back(ch);
        } 
        else if (std::isspace(ch)) {
            if (!buf.empty()) {
                m_tokens.emplace_back(this->classify_token(buf));
                buf.clear();
            }
        } 
        else if (std::ispunct(ch)){ // Operators & Symbols
            if (!buf.empty()) {
                m_tokens.emplace_back(this->classify_token(buf));
                buf.clear();
            }
            std::string o{ch};
            m_tokens.emplace_back(this->classify_token(o));
        }
    }
    for (const Token &tok : m_tokens) {
        std::println("Token value: {}", tok.value);
    }

    return m_tokens;
}

/* */
Token Lexer::classify_token(std::string &buf) noexcept {
    Token tok{};

    if (buf.find_first_not_of("0123456789"))   { tok.type = TokenType::LIT_INT; }
    else if (buf.find_first_not_of("+-*/"))    { tok.type = TokenType::BIN_OP; }

    // else if (buf == ";")    { tok.type = TokenType::DELIM_SEMI; } 
    // else if (buf == "exit") { tok.type = TokenType::KW_EXIT; } 
    // else if (buf == "let")  { tok.type = TokenType::KW_LET; } 
    // else if (buf == "=")    { tok.type = TokenType::OP_EQUALS; } 
    // else if (buf == "(")    { tok.type = TokenType::DELIM_LPAREN; } 
    // else if (buf == ")")    { tok.type = TokenType::DELIM_RPAREN; } 
    // else                    { tok.type = TokenType::VAR_IDENT; }
    else if (buf == ";")    { tok.type = TokenType::DELIM_SEMI; } 
    else if (buf == "exit") { tok.type = TokenType::KW_EXIT; } 
    else if (buf == "let")  { tok.type = TokenType::KW_LET; } 
    else if (buf == "=")    { tok.type = TokenType::OP_EQUALS; } 
    else if (buf == "(")    { tok.type = TokenType::DELIM_LPAREN; } 
    else if (buf == ")")    { tok.type = TokenType::DELIM_RPAREN; } 
    else                    {
        tok.type = TokenType::VAR_IDENT; 
        std::println("VAR IDENT");
    }




    tok.value = buf;
    return tok;
}

