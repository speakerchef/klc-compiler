#pragma once

#include "tokenizer.hpp"
#include "vector"
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <optional>

class Parser {
  private:
    std::vector<Token> m_tokens;
    size_t m_token_ptr = 0;
    std::ofstream &m_osref;

    [[nodiscard]] std::optional<Token> peek(size_t offset) const;
    [[maybe_unused]] std::optional<Token> take(size_t offset);

  public:
    Parser(std::vector<Token> toks, std::ofstream &ofref);
    ~Parser();
    void parse_tokens();
};

