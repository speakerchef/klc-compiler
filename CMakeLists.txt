cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(klc-compiler)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(klc
    ./src/klc-compiler.cpp
)


target_compile_options(klc PRIVATE -Wall -Wextra -Wconversion -Wpedantic)
