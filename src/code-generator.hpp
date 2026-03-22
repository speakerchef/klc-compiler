#pragma once

#include "syntax-tree.hpp"
#include <cstddef>
#include <fstream>

class CodeGenerator {
  public:
    CodeGenerator(std::ofstream &ofs);
    ~CodeGenerator();

    void emit(const SyntaxNode &node);
  private:
    std::ofstream m_os{};
    size_t m_stack_sz = 0;
};
