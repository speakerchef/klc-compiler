#pragma once

#include "syntax-tree.hpp"
#include <cstddef>
#include <fstream>
#include <unordered_map>

class CodeGenerator {
  public:
    CodeGenerator(std::ofstream &&ofs, std::vector<SyntaxNode> &&cstack, std::unordered_map<std::string, SyntaxNode> &&var_table);
    ~CodeGenerator();

    void emit();

  private:
    std::ofstream m_os{};
    size_t m_stack_sz = 0;
    std::vector<SyntaxNode> m_called_nodes{};
    std::unordered_map<std::string, SyntaxNode> m_var_table{};
};
