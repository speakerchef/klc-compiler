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
#include <variant>
#include <vector>

enum class TokenType {
    DELIM_SEMI,
    KW_EXIT,
    KW_RETURN,
    KW_LET,
    KW_INT,
    LIT_INT,
    LIT_STR,
    VAR_INT,
    UNCLASSED_VAR_DEC,
    OP_EQUALS,
    OP_PLUS,
    OP_MINUS,
};

typedef struct Token {
    TokenType type;
    std::variant<int, std::string> value;
} Token;

class Tokenizer {
  public:
    Tokenizer(std::ofstream &&os, std::ifstream &&is);
    ~Tokenizer();

    [[nodiscard]] std::optional<Token> peek(size_t offset) const;
    std::optional<Token> consume();
    void tokenize();

    inline std::vector<Token> get_tokens() const {
        return m_tokens;
    }

  private:
    std::vector<Token> m_tokens;
    size_t m_token_ptr = 0;
    std::ifstream m_ifs;
    std::ofstream m_ofs;

    /*==========================================================*/
    [[nodiscard]] Token classify_token(std::string &buf) noexcept;

};
