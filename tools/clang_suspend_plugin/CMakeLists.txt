cmake_minimum_required(VERSION 3.16)
project(kotlinx_suspend_plugin LANGUAGES CXX)

# This plugin is Apple/Clang-first. It is optional and only built when
# KOTLINX_BUILD_CLANG_SUSPEND_PLUGIN=ON is set at the top level.

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM CONFIG)
find_package(Clang CONFIG)

if(NOT LLVM_FOUND OR NOT Clang_FOUND)
    message(WARNING "LLVM/Clang development packages not found; skipping KotlinxSuspendPlugin. "
                    "Install LLVM (e.g. via Homebrew) and set LLVM_DIR/Clang_DIR.")
    set(KOTLINX_SUSPEND_PLUGIN_SKIP TRUE)
endif()

if(NOT KOTLINX_SUSPEND_PLUGIN_SKIP)
    list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
    include(AddLLVM)

    add_llvm_library(KotlinxSuspendPlugin MODULE
        KotlinxSuspendPlugin.cpp
    )

    target_include_directories(KotlinxSuspendPlugin PRIVATE
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
    )

    target_compile_definitions(KotlinxSuspendPlugin PRIVATE
        ${LLVM_DEFINITIONS}
    )

    llvm_map_components_to_libnames(LLVM_LIBS support)

    target_link_libraries(KotlinxSuspendPlugin PRIVATE
        ${LLVM_LIBS}
        clangAST
        clangBasic
        clangFrontend
        clangSerialization
        clangSema
        clangTooling
    )

    set_target_properties(KotlinxSuspendPlugin PROPERTIES
        PREFIX ""
    )
endif()
