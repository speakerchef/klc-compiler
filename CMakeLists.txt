cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(klc-compiler)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

add_executable(klc
    ./src/main.cpp
    ./src/lexer.cpp
    ./src/parser.cpp
    ./src/syntax-tree.cpp
    ./src/code-generator.cpp
)

add_library(klc_lib
    ./src/lexer.cpp
    ./src/parser.cpp
    ./src/syntax-tree.cpp
    ./src/code-generator.cpp
)

target_include_directories(klc_lib PUBLIC src)

target_link_libraries(klc PRIVATE klc_lib)

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(klc-tests ./tests/klc-tests.cpp)
target_link_libraries(klc-tests PRIVATE klc_lib GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(klc-tests)

target_compile_options(klc PRIVATE -Wall -Wextra -Wconversion -Wpedantic -Wimplicit-fallthrough)
target_compile_options(klc-tests PRIVATE -Wall -Wextra -Wconversion -Wpedantic -Wimplicit-fallthrough)
