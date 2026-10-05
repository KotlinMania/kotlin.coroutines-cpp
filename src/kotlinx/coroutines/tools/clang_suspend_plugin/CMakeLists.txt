cmake_minimum_required(VERSION 3.16)
project(kotlinx_suspend_plugin LANGUAGES C CXX)

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
    add_library(KotlinxSuspendPlugin MODULE
            KotlinxSuspendPlugin.cpp
            SuspendFunctionAnalyzer.cpp
    )

    target_include_directories(KotlinxSuspendPlugin SYSTEM PRIVATE
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
    )

    # Reliance on imported targets for include directories
    # target_link_libraries handles the dependencies.

    target_compile_definitions(KotlinxSuspendPlugin PUBLIC
        ${LLVM_DEFINITIONS}
        "-D__STDC_CONSTANT_MACROS"
        "-D__STDC_FORMAT_MACROS"
        "-D__STDC_LIMIT_MACROS"
    )

    if (LLVM_LINK_LLVM_DYLIB)
        target_compile_definitions(KotlinxSuspendPlugin PUBLIC "LLVM_LINK_LLVM_DYLIB")
    endif()

    # Share Clang/LLVM's runtime registries with the compiler loading the plugin.
    # A second static copy can register the same command-line options twice.
    if(TARGET clang-cpp AND TARGET LLVM)
        target_link_libraries(KotlinxSuspendPlugin PRIVATE clang-cpp LLVM)
    else()
        target_link_libraries(KotlinxSuspendPlugin PRIVATE
            clangAST clangAnalysis clangBasic clangFrontend clangLex
            clangSema clangSerialization clangTooling LLVMSupport)
    endif()

    set_target_properties(KotlinxSuspendPlugin PROPERTIES
        PREFIX ""
    )

    include(CTest)
    if(BUILD_TESTING AND TARGET clang-cpp AND TARGET LLVM)
        add_executable(kxs_analyzer_test SuspendFunctionAnalyzerTest.cpp SuspendFunctionAnalyzer.cpp)
        target_include_directories(kxs_analyzer_test SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
        target_compile_options(kxs_analyzer_test PRIVATE -UNDEBUG -Wall -Wextra -Werror)
        target_link_libraries(kxs_analyzer_test PRIVATE clang-cpp LLVM)
        add_test(NAME kxs_analyzer_liveness COMMAND kxs_analyzer_test)
    endif()
    if(TARGET kotlinx-coroutines-core)
        find_package(Python3 3.8 QUIET COMPONENTS Interpreter)
    endif()
    if(TARGET kotlinx-coroutines-core AND Python3_FOUND)
        get_filename_component(KXS_PLUGIN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../.." ABSOLUTE)
        find_program(KXS_PLUGIN_CLANG NAMES clang++ HINTS "${LLVM_TOOLS_BINARY_DIR}" NO_DEFAULT_PATH)
        if(KXS_PLUGIN_CLANG)
            add_test(NAME kxs_plugin_handoff
                COMMAND "${Python3_EXECUTABLE}" "${KXS_PLUGIN_ROOT}/src/tests/ir/test_suspend_plugin.py"
                    --root "${KXS_PLUGIN_ROOT}" --plugin "$<TARGET_FILE:KotlinxSuspendPlugin>"
                    --plugin-compiler "${KXS_PLUGIN_CLANG}" --compiler "${CMAKE_CXX_COMPILER}"
                    --library "$<TARGET_FILE:kotlinx-coroutines-core>"
                    "--library-link-options=${CMAKE_CXX_FLAGS} ${CMAKE_EXE_LINKER_FLAGS}"
                    --work-dir "${CMAKE_CURRENT_BINARY_DIR}/handoff-regression")
        endif()
    endif()
endif()
