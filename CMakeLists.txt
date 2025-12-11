cmake_minimum_required(VERSION 3.16)
project(kotlinx-coroutines-cpp VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =============================================================================
# Build Output Directory Structure
# =============================================================================
# All build artifacts go into build/ directory:
#   build/
#   ├── bin/          - Executables (test binaries, tools)
#   ├── lib/          - Static/shared libraries
#   ├── obj/          - Object files (via CMakeFiles, automatic)
#   └── ...           - CMake generated files
#
# To build out-of-source (recommended):
#   mkdir build && cd build && cmake .. && make
# =============================================================================

# Set output directories for all build artifacts
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Also set for multi-config generators (Visual Studio, Xcode)
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
endforeach()

# Enable testing
enable_testing()

# Add compile options
add_compile_options(-Wall -Wextra -Wpedantic)

# Kotlin Native runtime detection (for GC bridge weak-linking)
option(KOTLIN_NATIVE_RUNTIME_AVAILABLE "Link with Kotlin Native runtime for GC coordination" OFF)
if(KOTLIN_NATIVE_RUNTIME_AVAILABLE)
    add_compile_definitions(KOTLIN_NATIVE_RUNTIME_AVAILABLE=1)
endif()

# Include directories
include_directories(include)

# Add subdirectories
add_subdirectory(kotlinx-coroutines-core)

# Add test directory
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    add_subdirectory(tests)
endif()

# -----------------------------------------------------------------------------
# Optional compiler tooling (Clang suspend DSL plugin)
# -----------------------------------------------------------------------------
option(KOTLINX_BUILD_CLANG_SUSPEND_PLUGIN "Build Clang suspend DSL plugin (Apple-focused)" OFF)
if(KOTLINX_BUILD_CLANG_SUSPEND_PLUGIN)
    add_subdirectory(tools/clang_suspend_plugin)
endif()

# Install headers
install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)

# Print build configuration
message(STATUS "")
message(STATUS "=== kotlinx.coroutines-cpp Build Configuration ===")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Kotlin Native Runtime: ${KOTLIN_NATIVE_RUNTIME_AVAILABLE}")
message(STATUS "Binary Output: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "Library Output: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
message(STATUS "=================================================")
message(STATUS "")
