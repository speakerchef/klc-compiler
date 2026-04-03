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
    KW_FN,
    KW_LET,
    KW_MUT,
    KW_IF,
    KW_ELIF,
    KW_ELSE,
    KW_WHILE,
    KW_FOR,
    OP,
    DELIM_LPAREN,
    DELIM_RPAREN,
    DELIM_LCURLY,
    DELIM_RCURLY,
    DELIM_LSQUARE,
    DELIM_RSQUARE,
    LIT_INT,
    VAR_IDENT,
    NIL_,
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
    explicit Lexer(const std::string &path) noexcept;

    [[nodiscard]] std::optional<Token> peek(size_t offset) const;
    std::optional<Token> consume();
    std::vector<Token> tokenize();

  private:
    std::vector<Token> m_tokens;
    size_t m_token_ptr = 0;
    std::ifstream m_ifs;
    std::ofstream m_ofs;

    /*==========================================================*/
    [[nodiscard]] static Token classify_token(const std::string &buf);
    [[nodiscard]] static bool is_numeric(const std::string& buf);
    [[nodiscard]] static bool is_op(const std::string& buf);
    friend class LexerTests;
};
