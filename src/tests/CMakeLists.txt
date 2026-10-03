# Tests CMakeLists.txt
# All test binaries will be output to build/bin/

# Helper function to add a test executable
function(add_coroutine_test TEST_NAME)
    # Tests can be in src/suspend/, src/flow/terminal/, or src/ directly
    set(TEST_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/suspend/${TEST_NAME}.cpp")
    if(NOT EXISTS "${TEST_SOURCE}")
        set(TEST_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/flow/terminal/${TEST_NAME}.cpp")
    endif()
    if(NOT EXISTS "${TEST_SOURCE}")
        set(TEST_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/${TEST_NAME}.cpp")
    endif()
    if(EXISTS "${TEST_SOURCE}")
        add_executable(${TEST_NAME} ${TEST_SOURCE})
        target_include_directories(${TEST_NAME} PRIVATE
            ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/src/${KOTLINX_COROUTINES_SRC_DIR}
        )
        # Suppress library header warnings - these parameters exist for Kotlin API parity.
        # Tests include library headers, so they need the same suppressions as library code.
        # Test CODE should still be written to use parameters properly.
        target_compile_options(${TEST_NAME} PRIVATE
            -Wno-unused-parameter
            -Wno-unused-variable
            -Wno-unused-const-variable
            -Wno-unused-private-field
            -Wno-gnu-label-as-value  # Required for computed goto (K/N binary parity)
        )
        # Link against core library so tests can exercise non-inline implementations.
        target_link_libraries(${TEST_NAME} PRIVATE pthread kotlinx-coroutines-core)
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    else()
        message(STATUS "Test source not found: ${TEST_NAME}")
    endif()
endfunction()

# Core coroutine tests
add_coroutine_test(test_timeout)

# Transliterated Kotlin tests
add_coroutine_test(AsyncTest)
add_coroutine_test(JobTest)

# Suspension infrastructure and flow tests
add_coroutine_test(test_suspension_core)
add_coroutine_test(test_channel_as_flow_smoke)
add_coroutine_test(test_sync)
add_coroutine_test(test_combine_zip_smoke)
add_coroutine_test(test_share_hot_flow_smoke)
add_coroutine_test(test_transform_errors_smoke)
add_coroutine_test(test_collect_reduce_smoke)
add_coroutine_test(CountTest)
add_coroutine_test(LastTest)
add_coroutine_test(SingleTest)
add_coroutine_test(ToCollectionTest)
add_coroutine_test(ReduceTest)
add_coroutine_test(FoldTest)
add_coroutine_test(LaunchInTest)
add_coroutine_test(HangTest)
add_coroutine_test(CollectLatestTest)
if(TARGET test_plugin_canonical AND KOTLINX_BUILD_CLANG_SUSPEND_PLUGIN)
    target_compile_options(test_plugin_canonical PRIVATE -fplugin=$<TARGET_FILE:KotlinxSuspendPlugin>)
    add_dependencies(test_plugin_canonical KotlinxSuspendPlugin)
endif()

# Add gc_bridge tests if directory exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/gc_bridge")
    add_subdirectory(gc_bridge)
endif()

# Exercise standalone cleanup and the actual CMake compile/link pipeline.
if(CMAKE_CXX_COMPILER_ID MATCHES "^(AppleClang|Clang)$")
    find_package(Python3 3.8 QUIET COMPONENTS Interpreter)
    if(Python3_FOUND)
        get_filename_component(KXS_TEST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
        add_test(NAME test_ir_pipeline
            COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/ir/test_ir_pipeline.py"
                --root "${KXS_TEST_ROOT}"
                --modules "${KXS_TEST_ROOT}/cmake/Modules"
                --headers "${KXS_TEST_ROOT}/src"
                --core-test "${CMAKE_CURRENT_SOURCE_DIR}/src/test_suspension_core.cpp"
                --library "$<TARGET_FILE:kotlinx-coroutines-core>"
                --work-dir "${CMAKE_CURRENT_BINARY_DIR}/ir-regression"
                --compiler "${CMAKE_CXX_COMPILER}"
                --cmake "${CMAKE_COMMAND}")
        set_tests_properties(test_ir_pipeline PROPERTIES TIMEOUT 120)
    endif()
endif()
