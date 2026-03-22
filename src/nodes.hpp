#pragma once

#include "include/utils.hpp"

struct NodeExit {
    int exit_code;
};

struct NodeIntVar {
    int value;
    std::string ident;
    bool is_mutable;
};

