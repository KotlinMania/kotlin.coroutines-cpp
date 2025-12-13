cmake_minimum_required(VERSION 3.16)
project(kxs_inject LANGUAGES CXX C)

# This tool transforms LLVM IR to inject coroutine state machine dispatch.
# It builds against vendored LLVM - no external LLVM installation required.

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default to Release build for faster compilation
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Path to vendored LLVM - resolve to absolute path
# kxs_inject is at: src/kotlinx/coroutines/tools/kxs_inject
# Project root is 5 levels up
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../.." ABSOLUTE)
set(LLVM_SRC_DIR "${PROJECT_ROOT}/vendor/llvm-project/llvm")

if(NOT EXISTS "${LLVM_SRC_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Vendored LLVM not found at ${LLVM_SRC_DIR}")
endif()

message(STATUS "Using vendored LLVM from: ${LLVM_SRC_DIR}")

# Build LLVM as a subdirectory with minimal components
# We only need: Core, IRReader, BitReader, BitWriter, Support
set(LLVM_TARGETS_TO_BUILD "AArch64" CACHE STRING "" FORCE)
set(LLVM_ENABLE_PROJECTS "" CACHE STRING "" FORCE)
set(LLVM_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(LLVM_BUILD_UTILS OFF CACHE BOOL "" FORCE)
set(LLVM_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LLVM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LLVM_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(LLVM_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(LLVM_INCLUDE_TOOLS OFF CACHE BOOL "" FORCE)
set(LLVM_INCLUDE_UTILS OFF CACHE BOOL "" FORCE)
set(LLVM_INCLUDE_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LLVM_INCLUDE_TESTS OFF CACHE BOOL "" FORCE)
set(LLVM_INCLUDE_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(LLVM_INCLUDE_DOCS OFF CACHE BOOL "" FORCE)
set(LLVM_ENABLE_ZLIB OFF CACHE BOOL "" FORCE)
set(LLVM_ENABLE_ZSTD OFF CACHE BOOL "" FORCE)
set(LLVM_ENABLE_LIBXML2 OFF CACHE BOOL "" FORCE)
set(LLVM_ENABLE_TERMINFO OFF CACHE BOOL "" FORCE)
set(LLVM_ENABLE_LIBEDIT OFF CACHE BOOL "" FORCE)

# Add LLVM as subdirectory
add_subdirectory(${LLVM_SRC_DIR} ${CMAKE_CURRENT_BINARY_DIR}/llvm EXCLUDE_FROM_ALL)

# Now build our tool
add_executable(kxs-inject
    kxs_inject.cpp
)

target_include_directories(kxs-inject PRIVATE
    ${LLVM_SRC_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}/llvm/include
)

target_compile_definitions(kxs-inject PRIVATE
    __STDC_CONSTANT_MACROS
    __STDC_FORMAT_MACROS
    __STDC_LIMIT_MACROS
)

# Link only the LLVM components we need
target_link_libraries(kxs-inject PRIVATE
    LLVMCore
    LLVMIRReader
    LLVMBitReader
    LLVMBitWriter
    LLVMAsmParser
    LLVMSupport
)

# Install
install(TARGETS kxs-inject RUNTIME DESTINATION bin)
