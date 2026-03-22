#pragma once

#include "syntax-tree.hpp"
#include "tokenizer.hpp"
#include <cstddef>
#include <fstream>

class CodeGenerator {
  public:
    CodeGenerator(std::ofstream &ofs, std::vector<std::pair<TokenType, std::string>> cstack, SyntaxTree stree);
    ~CodeGenerator();

    void emit();
  private:
    std::ofstream m_os{};
    size_t m_stack_sz = 0;
    std::vector<std::pair<TokenType, std::string>> m_call_stack{};
    SyntaxTree m_stree{};
};
