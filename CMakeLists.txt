cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(klc-compiler)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

add_executable(klc
    ./src/klc-compiler.cpp
    ./src/lexer.cpp
    ./src/parser.cpp
    ./src/syntax-tree.cpp
    ./src/code-generator.cpp
)


target_compile_options(klc PRIVATE -Wall -Wextra -Wconversion -Wpedantic -Wimplicit-fallthrough)
