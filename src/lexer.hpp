/*
 * Author: Sohan Nair
 * Date: No idea
 *
 */

#pragma once

#include <cassert>
#include <cctype>
#include <cstddef>
#include <fstream>
#include <string>
#include <vector>

enum class TokenType {
    DELIM_SEMI,
    KW_EXIT,
    KW_RETURN,
    KW_LET,
    KW_MUT,
    KW_INT,
    BIN_OP,
    OP_EQUALS,
    OP_ADD,
    OP_SUB,
    OP_MULT,
    OP_DIV,
    DELIM_LPAREN,
    DELIM_RPAREN,
    LIT_INT,
    VAR_IDENT,
};

struct LocData{
    size_t line;
    size_t col;
};

struct Token {
    TokenType type;
    std::string value;
    LocData loc;
};

class Lexer {
  public:
    Lexer(const std::string &path) noexcept;

    [[nodiscard]] std::optional<Token> peek(size_t offset) const;
    std::optional<Token> consume();
    std::vector<Token> tokenize();

  private:
    std::vector<Token> m_tokens;
    size_t m_token_ptr = 0;
    std::ifstream m_ifs;
    std::ofstream m_ofs;

    /*==========================================================*/
    [[nodiscard]] Token classify_token(std::string &buf) noexcept;
};
