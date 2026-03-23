#pragma once

#include "syntax-tree.hpp"
#include "tokenizer.hpp"
#include "vector"
#include <cstddef>
#include <cstdlib>
#include <optional>

class Parser {
  public:
    Parser(std::vector<Token> toks);
    ~Parser();

  private:
    std::vector<Token> m_tokens;
    size_t m_token_ptr = 0;
    SyntaxTree syntax_tree{};
    void parse_tokens();

    [[nodiscard]] std::optional<Token> peek(size_t offset) const;
    [[maybe_unused]] std::optional<Token> consume();
};

