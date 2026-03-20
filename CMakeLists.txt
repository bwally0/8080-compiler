cmake_minimum_required(VERSION 3.16)

project(bmlc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSION OFF)

add_executable(bmlc
    src/main.cpp
    src/lexer/lexer.cpp
    src/parser/parser.cpp
    src/parser/ast.cpp
    src/semantic/symbol_table.cpp
    src/semantic/semantic_analyzer.cpp
)

target_include_directories(bmlc
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)
