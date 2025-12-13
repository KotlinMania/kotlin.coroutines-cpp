cmake_minimum_required(VERSION 3.16)
project(kotlinx-coroutines-cpp VERSION 1.0.0 LANGUAGES CXX C)

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

# Stackless Coroutine Support (Experimental/Legacy)
option(KOTLINX_ENABLE_STACKLESS "Enable stackless coroutine implementation" OFF)
if(KOTLINX_ENABLE_STACKLESS)
    add_compile_definitions(KOTLINX_ENABLE_STACKLESS=1)
endif()

# -----------------------------------------------------------------------------
# Source layout root
#
# The coroutine package tree is migrating from `src/kotlinx/coroutines/**`
# to `src/kotlinx/coroutine/**`. Pass this down to src/CMakeLists.txt so we
# don't have to touch build files during the move.
# -----------------------------------------------------------------------------
set(KOTLINX_COROUTINES_SRC_DIR "kotlinx/coroutines" CACHE STRING
    "Coroutine sources root relative to src/ (kotlinx/coroutine or kotlinx/coroutines)")

# Add subdirectories
add_subdirectory(src)
add_subdirectory(debugging)

# Add test directory
# Add test directory
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    add_subdirectory(tests)
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/tests")
    add_subdirectory(src/tests)
endif()

# -----------------------------------------------------------------------------
# Optional compiler tooling (Clang suspend DSL plugin)
# -----------------------------------------------------------------------------
option(KOTLINX_BUILD_CLANG_SUSPEND_PLUGIN "Build Clang suspend DSL plugin (Apple-focused)" ON)
if(KOTLINX_BUILD_CLANG_SUSPEND_PLUGIN)
    add_subdirectory(src/kotlinx/coroutines/tools/clang_suspend_plugin)
endif()

# -----------------------------------------------------------------------------
# kxs-inject: LLVM IR transformation tool (builds vendored LLVM)
# -----------------------------------------------------------------------------
option(KOTLINX_BUILD_KXS_INJECT "Build kxs-inject IR transformation tool" OFF)
if(KOTLINX_BUILD_KXS_INJECT)
    add_subdirectory(src/kotlinx/coroutines/tools/kxs_inject)
endif()

# =============================================================================
# CMake Package: kotlinx::coroutines
# =============================================================================
# This creates an installable CMake package that provides:
#   - kotlinx::coroutines       INTERFACE library with headers + IR transform
#   - kotlinx::coroutines_headers  Headers only (for callers, not definers)
#   - kxs_enable_suspend()      Enable IR transformation for a target
#
# Usage after installation:
#   find_package(KotlinxCoroutines REQUIRED)
#   target_link_libraries(my_app PRIVATE kotlinx::coroutines)
#   kxs_enable_suspend(my_app)
# =============================================================================

# Add our modules to the path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

# Include the package definition
include(KotlinxCoroutines)

# Install headers
install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)

# Install CMake modules for find_package support
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/KotlinxCoroutines.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/KotlinxCoroutineTransform.cmake"
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/kxs_transform_ir.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/KotlinxCoroutines"
)

# Generate package version file
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/KotlinxCoroutinesConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Generate package config file
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/KotlinxCoroutinesConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KotlinxCoroutinesConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/KotlinxCoroutines"
)

# Install package config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KotlinxCoroutinesConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/KotlinxCoroutinesConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/KotlinxCoroutines"
)

# Export targets for find_package
install(TARGETS kotlinx_coroutines kotlinx_coroutines_headers
    EXPORT KotlinxCoroutinesTargets
)

install(EXPORT KotlinxCoroutinesTargets
    FILE KotlinxCoroutinesTargets.cmake
    NAMESPACE kotlinx::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/KotlinxCoroutines"
)

# -----------------------------------------------------------------------------
# Lint targets (don't require compilation)
# -----------------------------------------------------------------------------
find_package(Python3 COMPONENTS Interpreter)
if(Python3_FOUND)
    add_custom_target(lint
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/lint_codebase.py
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running lint analysis..."
    )
    add_custom_target(lint-summary
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/lint_codebase.py --summary
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running lint summary..."
    )
    add_custom_target(lint-json
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/lint_codebase.py --json
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running lint (JSON output)..."
    )
    add_custom_target(analyze-imports
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/extract_import_mismatches.py
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Analyzing import mismatches..."
    )
    add_custom_target(analyze-packages
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/analyze_packages.py
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Analyzing package/namespace alignment..."
    )
    add_custom_target(analyze-symbols
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/analyze_symbols.py
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Analyzing symbol definitions..."
    )
    add_custom_target(analyze-duplicates
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/analyze_symbols.py --duplicates
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Finding duplicate symbol definitions..."
    )
    add_custom_target(analyze-stubs
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/analyze_symbols.py --stubs
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Finding stub implementations..."
    )
endif()

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
