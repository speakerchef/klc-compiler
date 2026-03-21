#pragma once

#include "nodes.hpp"
#include <fstream>

class CodeGenerator {
  private:
    std::ofstream m_os{};

  public:
    CodeGenerator(std::ofstream &ofs);
    void emit(const SyntaxNode &node);
};
