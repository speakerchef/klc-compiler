#pragma once

#include "syntax-tree.hpp"
#include <cstddef>
#include <fstream>

class CodeGenerator {
  public:
    CodeGenerator(NodeProgram&& prog) noexcept;
    ~CodeGenerator();

    void emit();

  private:
    std::ofstream m_os;
    size_t m_stack_sz = 0;
    NodeProgram m_program;
};
