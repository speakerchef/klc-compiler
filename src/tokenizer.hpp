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
    LIT_INT,
    LIT_STR,
    DELIM_SEMI,
    KW_EXIT,
    KW_RETURN,
    OP_EQUALS,
    OP_PLUS,
    OP_MINUS,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_POWER,
    CLASS_ERROR,
};

typedef struct Token {
    TokenType type;
    std::variant<int, char, std::string> value;
} Token;

class Tokenizer {
  private:
    [[nodiscard]] Token classify_token(std::string &buf) noexcept;

  public:
    [[nodiscard]] std::vector<Token> tokenize(std::ifstream file);
};
