#include "lexer.hpp"
#include "include/utils.hpp"
#include <algorithm>
#include <cassert>
#include <cctype>
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
            if (ch == '\n') { line_cnt++, col_cnt = 0; }
        } else if (std::ispunct(ch)) { // Operators & Symbols
            col_cnt++;

            if (ch == '_') {
                buf.push_back(ch);
                continue;
            }
            if (!buf.empty()) {
                m_tokens.emplace_back(classify_token(buf));
                m_tokens.at(tok_idx).loc = { line_cnt, col_cnt };
                tok_idx++;
                buf.clear();
            }
            // double-char bin ops
            const auto fix_char = [&](const char c1, const char c2) {
                col_cnt++;
                m_tokens.emplace_back(classify_token({c1, c2}));
                m_tokens.at(tok_idx).loc = { line_cnt, col_cnt };
                tok_idx++;
            };
            switch (ch) {
                case '-': {
                    char op; m_ifs.get(op);
                    if (op == ch) {
                        fix_char(ch, ch);
                        continue;
                    }
                    if (op == '=') {
                        fix_char(ch, op);
                        continue;
                    }
                    m_ifs.unget();
                    break;
                }
                case '+': {
                    char op; m_ifs.get(op);
                    if (op == ch) {
                        fix_char(ch, ch);
                        continue;
                    }
                    if (op == '=') {
                        fix_char(ch, op);
                        continue;
                    }
                    m_ifs.unget();
                    break;
                }
                case '=': {
                    char op; m_ifs.get(op);
                    if (op == ch) {
                        fix_char(ch, ch);
                        continue;
                    }
                    m_ifs.unget();
                    break;
                }
                case '!': {
                    char op; m_ifs.get(op);
                    if (op == '=') {
                        fix_char(ch, op);
                        continue;
                    }
                    m_ifs.unget();
                    break;
                }
                case '>': {
                    char op; m_ifs.get(op);
                    if (op == '=') {
                        fix_char(ch, op);
                        continue;
                    }
                    if (op == '>') {
                        char op2;
                        m_ifs.get(op2);
                        if (op2 == '=') {
                            col_cnt += 2;
                            m_tokens.emplace_back(classify_token({ch, ch, op2}));
                            m_tokens.at(tok_idx).loc = { line_cnt, col_cnt };
                            tok_idx++;
                            continue;
                        }
                        m_ifs.unget();
                        fix_char(ch, op);
                        continue;
                    }
                    m_ifs.unget();
                    break;
                }
                case '<': {
                    char op; m_ifs.get(op);
                    if (op == '=') {
                        fix_char(ch, op);
                        continue;
                    }
                    if (op == '<') {
                        char op2;
                        m_ifs.get(op2);
                        if (op2 == '=') {
                            col_cnt += 2;
                            m_tokens.emplace_back(classify_token({ch, ch, op2}));
                            m_tokens.at(tok_idx).loc = { line_cnt, col_cnt };
                            tok_idx++;
                            continue;
                        }
                        m_ifs.unget();

                        fix_char(ch, op);
                        continue;
                    }
                    m_ifs.unget();
                    break;
                }
                case '&': {
                    char op; m_ifs.get(op);
                    if (op == ch) {
                        fix_char(ch, ch);
                        continue;
                    }
                    if (op == '=') {
                        fix_char(ch, op);
                        continue;
                    }
                    m_ifs.unget();
                    break;
                }
                case '|': {
                    char op; m_ifs.get(op);
                    if (op == ch) {
                        fix_char(ch, ch);
                        continue;
                    }
                    if (op == '=') {
                        fix_char(ch, op);
                        continue;
                    }
                    m_ifs.unget();
                    break;
                }
                case '*': {
                    char op; m_ifs.get(op);
                    if (op == ch) {
                        char op2;
                        m_ifs.get(op2);
                        if (op2 == '=') {
                            col_cnt += 2;
                            m_tokens.emplace_back(classify_token({ch, ch, op2}));
                            m_tokens.at(tok_idx).loc = { line_cnt, col_cnt };
                            tok_idx++;
                            continue;
                        }
                        m_ifs.unget();

                        fix_char(ch, ch);
                        continue;
                    }
                    if (op == '=') {
                        fix_char(ch, op);
                        continue;
                    }
                    m_ifs.unget();
                    break;
                }
                case '/': [[fallthrough]];
                case '%': [[fallthrough]];
                case '^': {
                    char op; m_ifs.get(op);
                    if (op == '=') {
                        fix_char(ch, op);
                        continue;
                    }
                    m_ifs.unget();
                    break;
                }
            }
            std::string o{ch};
            m_tokens.emplace_back(classify_token(o));
            m_tokens.at(tok_idx).loc = { line_cnt, col_cnt };
            tok_idx++;
        }
    }
    // for (const auto& tok : m_tokens) {
    //     std::println("{}", tok.value);
    // }

    return m_tokens;
}

bool Lexer::is_numeric(const std::string& buf) {
    return std::all_of(buf.begin(), buf.end(), ::isdigit);
}

bool Lexer::is_op(const std::string& buf) {
    return buf.find_first_not_of("+-*/<>=|&^!%");
}

/* */
Token Lexer::classify_token(const std::string &buf) {
    Token tok{};

    if (is_numeric(buf))        { tok.type = TokenType::LIT_INT; }
    else if (is_op(buf))        { tok.type = TokenType::OP; }
    else if (buf == "fn")       { tok.type = TokenType::KW_FN; } 
    else if (buf == "return")   { tok.type = TokenType::KW_RETURN; } 
    else if (buf == "exit")     { tok.type = TokenType::KW_EXIT; } 
    else if (buf == "let")      { tok.type = TokenType::KW_LET; } 
    else if (buf == "mut")      { tok.type = TokenType::KW_MUT; } 
    else if (buf == "if")       { tok.type = TokenType::KW_IF; }
    else if (buf == "elif")     { tok.type = TokenType::KW_ELIF; }
    else if (buf == "else")     { tok.type = TokenType::KW_ELSE; }
    else if (buf == "while")    { tok.type = TokenType::KW_WHILE; }
    else if (buf == "for")      { tok.type = TokenType::KW_FOR; }
    else if (buf == ";")        { tok.type = TokenType::DELIM_SEMI; } 
    else if (buf == "(")        { tok.type = TokenType::DELIM_LPAREN; }
    else if (buf == ")")        { tok.type = TokenType::DELIM_RPAREN; }
    else if (buf == "{")        { tok.type = TokenType::DELIM_LCURLY; }
    else if (buf == "}")        { tok.type = TokenType::DELIM_RCURLY; }
    else if (buf == "[")        { tok.type = TokenType::DELIM_LSQUARE; }
    else if (buf == "]")        { tok.type = TokenType::DELIM_RSQUARE; }
    else if (buf == ",")        { tok.type = TokenType::DELIM_COMMA; }
    else                        { tok.type = TokenType::VAR_IDENT; }

    tok.value = buf;
    return tok;
}

