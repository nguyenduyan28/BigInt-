cmake_minimum_required(VERSION 3.10)
project(infinite_int_calc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic)

add_library(bigint STATIC
    src/bigint/BigInt.cpp
)
target_include_directories(bigint PUBLIC src)

add_library(parser STATIC
    src/parser/Lexer.cpp
    src/parser/Parser.cpp
)
target_include_directories(parser PUBLIC src)

add_executable(calc
    src/cli/main.cpp
)
target_link_libraries(calc PRIVATE parser bigint)

add_executable(parser_tests
    tests/test_parser.cpp
)
target_link_libraries(parser_tests PRIVATE parser)

add_executable(bigint_tests
    tests/test_bigint.cpp
)
target_link_libraries(bigint_tests PRIVATE bigint)
