cmake_minimum_required(VERSION 3.20)
project(ASTDistance VERSION 0.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use vendored tree-sitter sources. Builds must not clone or fetch grammars
# during configure.
set(AST_DISTANCE_VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor")
set(tree-sitter_SOURCE_DIR "${AST_DISTANCE_VENDOR_DIR}/tree-sitter")
set(tree-sitter-rust_SOURCE_DIR "${AST_DISTANCE_VENDOR_DIR}/tree-sitter-rust")
set(tree-sitter-kotlin_SOURCE_DIR "${AST_DISTANCE_VENDOR_DIR}/tree-sitter-kotlin")
set(tree-sitter-cpp_SOURCE_DIR "${AST_DISTANCE_VENDOR_DIR}/tree-sitter-cpp")
set(tree-sitter-python_SOURCE_DIR "${AST_DISTANCE_VENDOR_DIR}/tree-sitter-python")
set(tree-sitter-typescript_SOURCE_DIR "${AST_DISTANCE_VENDOR_DIR}/tree-sitter-typescript")

foreach(required_tree_sitter_source
    "${tree-sitter_SOURCE_DIR}/lib/src/lib.c"
    "${tree-sitter_SOURCE_DIR}/lib/include/tree_sitter/api.h"
    "${tree-sitter-rust_SOURCE_DIR}/src/parser.c"
    "${tree-sitter-rust_SOURCE_DIR}/src/scanner.c"
    "${tree-sitter-kotlin_SOURCE_DIR}/src/parser.c"
    "${tree-sitter-kotlin_SOURCE_DIR}/src/scanner.c"
    "${tree-sitter-cpp_SOURCE_DIR}/src/parser.c"
    "${tree-sitter-cpp_SOURCE_DIR}/src/scanner.c"
    "${tree-sitter-python_SOURCE_DIR}/src/parser.c"
    "${tree-sitter-python_SOURCE_DIR}/src/scanner.c"
    "${tree-sitter-typescript_SOURCE_DIR}/typescript/src/parser.c"
    "${tree-sitter-typescript_SOURCE_DIR}/typescript/src/scanner.c"
    "${tree-sitter-typescript_SOURCE_DIR}/common/scanner.h"
)
    if(NOT EXISTS "${required_tree_sitter_source}")
        message(FATAL_ERROR "Missing vendored tree-sitter source: ${required_tree_sitter_source}")
    endif()
endforeach()

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

# Build tree-sitter-python parser
add_library(tree-sitter-python-parser STATIC
    ${tree-sitter-python_SOURCE_DIR}/src/parser.c
    ${tree-sitter-python_SOURCE_DIR}/src/scanner.c
)
target_include_directories(tree-sitter-python-parser PUBLIC
    ${tree-sitter_SOURCE_DIR}/lib/include
    ${tree-sitter-python_SOURCE_DIR}/src
)
target_compile_options(tree-sitter-python-parser PRIVATE -w)

# Build tree-sitter-typescript parser
add_library(tree-sitter-typescript-parser STATIC
    ${tree-sitter-typescript_SOURCE_DIR}/typescript/src/parser.c
    ${tree-sitter-typescript_SOURCE_DIR}/typescript/src/scanner.c
)
target_include_directories(tree-sitter-typescript-parser PUBLIC
    ${tree-sitter_SOURCE_DIR}/lib/include
    ${tree-sitter-typescript_SOURCE_DIR}/typescript/src
)
target_compile_options(tree-sitter-typescript-parser PRIVATE -w)

# Main executable
add_executable(ast_distance
    src/main.cpp
    src/ast_parser.cpp
    src/ast_normalizer.cpp
    src/similarity.cpp
    src/symbol_extractor.cpp
    src/symbol_analysis.cpp
    src/symbol_extraction.cpp
    src/deep_inventory.cpp
    src/transliteration_engine.cpp
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
    tree-sitter-python-parser
    tree-sitter-typescript-parser
)

add_custom_command(TARGET ast_distance POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -f ${CMAKE_CURRENT_SOURCE_DIR}/ast_distance
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ast_distance> ${CMAKE_CURRENT_SOURCE_DIR}/ast_distance
    COMMENT "Refreshing source-tree ast_distance executable"
)

# ----------------------------
# Tests
# ----------------------------
# Native algorithm and captured CLI regression tests are registered below.
include(CTest)

if(BUILD_TESTING)
    add_executable(kotlin_grammar_test tests/kotlin_grammar_test.cc)
    target_include_directories(kotlin_grammar_test PRIVATE include)
    target_compile_options(kotlin_grammar_test PRIVATE -UNDEBUG)
    target_link_libraries(kotlin_grammar_test PRIVATE tree-sitter-core tree-sitter-kotlin-parser)
    add_test(NAME kotlin_grammar_native COMMAND kotlin_grammar_test)
endif()

# Install
install(TARGETS ast_distance RUNTIME DESTINATION bin)

if(BUILD_TESTING)
    add_executable(extraction_review_test tests/extraction_review_test.cc src/symbol_analysis.cpp)
    target_include_directories(extraction_review_test PRIVATE include ${tree-sitter_SOURCE_DIR}/lib/include)
    target_compile_options(extraction_review_test PRIVATE -UNDEBUG)
    target_link_libraries(extraction_review_test PRIVATE tree-sitter-core tree-sitter-rust-parser tree-sitter-kotlin-parser tree-sitter-cpp-parser tree-sitter-python-parser tree-sitter-typescript-parser)
    add_test(NAME extraction_review_native COMMAND extraction_review_test)
endif()

if(BUILD_TESTING)
    add_test(NAME cli_report_capture
        COMMAND ${CMAKE_COMMAND} -DAST_DISTANCE=$<TARGET_FILE:ast_distance>
        -DTEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/cli-report-fixtures
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cli_report_test.cmake)
endif()

if(BUILD_TESTING)
    add_executable(logic_drift_test tests/logic_drift_test.cc)
    target_include_directories(logic_drift_test PRIVATE include ${tree-sitter_SOURCE_DIR}/lib/include)
    target_compile_options(logic_drift_test PRIVATE -UNDEBUG)
    target_link_libraries(logic_drift_test PRIVATE tree-sitter-core tree-sitter-rust-parser tree-sitter-kotlin-parser tree-sitter-cpp-parser tree-sitter-python-parser tree-sitter-typescript-parser)
    add_test(NAME normalized_logic_drift_native COMMAND logic_drift_test)
endif()
if(BUILD_TESTING)
    add_test(NAME normalized_logic_drift_cli
        COMMAND ${CMAKE_COMMAND} -DAST_DISTANCE=$<TARGET_FILE:ast_distance>
        -DTEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/logic-cli-fixtures
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/logic_cli_test.cmake)
endif()



if(BUILD_TESTING)
    add_test(NAME deep_inventory_cli
        COMMAND ${CMAKE_COMMAND} -DAST_DISTANCE=$<TARGET_FILE:ast_distance>
        -DTEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/deep-fixtures
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/deep_report_test.cmake)
endif()

if(BUILD_TESTING)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test-tmp")
    set_tests_properties(extraction_review_native PROPERTIES ENVIRONMENT "TMPDIR=${CMAKE_CURRENT_BINARY_DIR}/test-tmp")
endif()

if(BUILD_TESTING)
    add_executable(transliteration_engine_test tests/transliteration_engine_test.cc src/transliteration_engine.cpp)
    target_include_directories(transliteration_engine_test PRIVATE include ${tree-sitter_SOURCE_DIR}/lib/include)
    target_compile_options(transliteration_engine_test PRIVATE -UNDEBUG -Wall -Wextra -Wpedantic -Werror)
    target_link_libraries(transliteration_engine_test PRIVATE tree-sitter-core tree-sitter-rust-parser tree-sitter-kotlin-parser tree-sitter-cpp-parser tree-sitter-python-parser tree-sitter-typescript-parser)
    add_test(NAME transliteration_engine_native COMMAND transliteration_engine_test)
    add_test(NAME transliteration_engine_cli
        COMMAND ${CMAKE_COMMAND} -DAST_DISTANCE=$<TARGET_FILE:ast_distance>
        -DTEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/transliteration-fixtures
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/transliteration_cli_test.cmake)
endif()

if(BUILD_TESTING)
    add_test(NAME identity_pairing_cli
        COMMAND ${CMAKE_COMMAND} -DAST_DISTANCE=$<TARGET_FILE:ast_distance>
        -DTEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/identity-fixtures
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/identity_pairing_test.cmake)
endif()
