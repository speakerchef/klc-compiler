#pragma once

#include "syntax-tree.hpp"
#include <cstddef>
#include <fstream>

class CodeGenerator {
  private:
    std::ofstream m_os{};
    size_t m_stack_sz = 0;

  public:
    CodeGenerator(std::ofstream &ofs);
    ~CodeGenerator();

    void emit(const SyntaxNode &node);
};
