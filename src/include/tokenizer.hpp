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
    T_TYPE_INT,
    T_TYPE_STR,
    T_TYPE_BYTE,
    T_DELIM_SEMI,
    T_KW_EXIT,
    T_KW_RETURN,
    PARSE_ERROR,
};

typedef struct Token {
    TokenType type;
    std::variant<int, char, std::string> value;
} Token;

class Tokenizer {
  private:
    [[nodiscard]] Token parse_token(std::string &buf) noexcept;

  public:
    [[nodiscard]] std::vector<Token> tokenize(std::ifstream file);
};
