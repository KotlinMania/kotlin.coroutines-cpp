cmake_minimum_required(VERSION 3.20)
project(ASTDistance VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Fetch tree-sitter
include(FetchContent)

FetchContent_Declare(
    tree-sitter
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter.git
    GIT_TAG v0.22.6
)

FetchContent_Declare(
    tree-sitter-rust
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-rust.git
    GIT_TAG v0.21.2
)

FetchContent_Declare(
    tree-sitter-kotlin
    GIT_REPOSITORY https://github.com/fwcd/tree-sitter-kotlin.git
    GIT_TAG 0.3.8
)

FetchContent_Declare(
    tree-sitter-cpp
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-cpp.git
    GIT_TAG v0.22.3
)

FetchContent_MakeAvailable(tree-sitter tree-sitter-rust tree-sitter-kotlin tree-sitter-cpp)

# Build tree-sitter core as a library
# Disable warnings for third-party C code (they use old-style prototypes)
add_library(tree-sitter-core STATIC
    ${tree-sitter_SOURCE_DIR}/lib/src/lib.c
)
target_include_directories(tree-sitter-core PUBLIC
    ${tree-sitter_SOURCE_DIR}/lib/include
    ${tree-sitter_SOURCE_DIR}/lib/src
)
target_compile_options(tree-sitter-core PRIVATE -w)

# Build tree-sitter-rust parser
add_library(tree-sitter-rust-parser STATIC
    ${tree-sitter-rust_SOURCE_DIR}/src/parser.c
    ${tree-sitter-rust_SOURCE_DIR}/src/scanner.c
)
target_include_directories(tree-sitter-rust-parser PUBLIC
    ${tree-sitter_SOURCE_DIR}/lib/include
    ${tree-sitter-rust_SOURCE_DIR}/src
)
target_compile_options(tree-sitter-rust-parser PRIVATE -w)

# Build tree-sitter-kotlin parser
add_library(tree-sitter-kotlin-parser STATIC
    ${tree-sitter-kotlin_SOURCE_DIR}/src/parser.c
    ${tree-sitter-kotlin_SOURCE_DIR}/src/scanner.c
)
target_include_directories(tree-sitter-kotlin-parser PUBLIC
    ${tree-sitter_SOURCE_DIR}/lib/include
    ${tree-sitter-kotlin_SOURCE_DIR}/src
)
target_compile_options(tree-sitter-kotlin-parser PRIVATE -w)

# Build tree-sitter-cpp parser
add_library(tree-sitter-cpp-parser STATIC
    ${tree-sitter-cpp_SOURCE_DIR}/src/parser.c
    ${tree-sitter-cpp_SOURCE_DIR}/src/scanner.c
)
target_include_directories(tree-sitter-cpp-parser PUBLIC
    ${tree-sitter_SOURCE_DIR}/lib/include
    ${tree-sitter-cpp_SOURCE_DIR}/src
)
target_compile_options(tree-sitter-cpp-parser PRIVATE -w)

# Main executable
add_executable(ast_distance
    src/main.cpp
    src/ast_parser.cpp
    src/ast_normalizer.cpp
    src/similarity.cpp
    src/symbol_analysis.cpp
)

target_include_directories(ast_distance PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${tree-sitter_SOURCE_DIR}/lib/include
)

target_link_libraries(ast_distance PRIVATE
    tree-sitter-core
    tree-sitter-rust-parser
    tree-sitter-kotlin-parser
    tree-sitter-cpp-parser
)

# Install
install(TARGETS ast_distance RUNTIME DESTINATION bin)
