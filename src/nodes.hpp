#pragma once

#include "include/utils.hpp"
#include "tokenizer.hpp"

struct NodeExit {
    int exit_code;
};

struct NodeIntVar {
    int value;
    std::string ident;
    bool is_mutable;
};

struct NodeExpr {
    TokenType op_type;
    std::string expr;
};
