# Tests CMakeLists.txt
# All test binaries will be output to build/bin/

# Helper function to add a test executable
function(add_coroutine_test TEST_NAME)
    # Tests are currently located in src/suspend/
    set(TEST_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/suspend/${TEST_NAME}.cpp")
    if(EXISTS "${TEST_SOURCE}")
        add_executable(${TEST_NAME} ${TEST_SOURCE})
        target_include_directories(${TEST_NAME} PRIVATE
            ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/src/${KOTLINX_COROUTINES_SRC_DIR}
        )
        # Link against core library so tests can exercise non-inline implementations.
        target_link_libraries(${TEST_NAME} PRIVATE pthread kotlinx-coroutines-core)
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    else()
        message(STATUS "Test source not found: ${TEST_SOURCE}")
    endif()
endfunction()

# Core coroutine tests
add_coroutine_test(test_job)
add_coroutine_test(test_cancellation)
add_coroutine_test(test_dispatchers)
add_coroutine_test(test_context)
add_coroutine_test(test_suspend)

# Cancellable tests
add_coroutine_test(test_cancellable)

# Suspension infrastructure tests
add_coroutine_test(test_suspension_core)
add_coroutine_test(test_suspension_infrastructure)
add_coroutine_test(test_suspension_infrastructure_simple)
add_coroutine_test(test_suspension_fixes)
add_coroutine_test(test_comprehensive_suspension)
add_coroutine_test(test_delay_dsl)
add_coroutine_test(test_dsl)
add_coroutine_test(test_plugin_canonical)
add_coroutine_test(test_flow_merge_smoke)
add_coroutine_test(test_channel_as_flow_smoke)
if(KOTLINX_BUILD_CLANG_SUSPEND_PLUGIN)
    target_compile_options(test_plugin_canonical PRIVATE -fplugin=$<TARGET_FILE:KotlinxSuspendPlugin>)
    add_dependencies(test_plugin_canonical KotlinxSuspendPlugin)
endif()
if(KOTLINX_ENABLE_STACKLESS)
    add_coroutine_test(test_stackless_plugin)
    if(KOTLINX_BUILD_CLANG_SUSPEND_PLUGIN)
        target_compile_options(test_stackless_plugin PRIVATE -fplugin=$<TARGET_FILE:KotlinxSuspendPlugin>)
        add_dependencies(test_stackless_plugin KotlinxSuspendPlugin)
    endif()
endif()

# Add gc_bridge tests if directory exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/gc_bridge")
    add_subdirectory(gc_bridge)
endif()
