# Tests CMakeLists.txt
# All test binaries will be output to build/bin/

# Helper function to add a test executable
function(add_coroutine_test TEST_NAME)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}.cpp")
        add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
        target_include_directories(${TEST_NAME} PRIVATE
            ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/src/${KOTLINX_COROUTINES_SRC_DIR}
        )
        # Link against core library so tests can exercise non-inline implementations.
        target_link_libraries(${TEST_NAME} PRIVATE pthread kotlinx-coroutines-core)
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endif()
endfunction()

# Core coroutine tests
add_coroutine_test(test_job)
add_coroutine_test(test_cancellation)
add_coroutine_test(test_dispatchers)
add_coroutine_test(test_context)
add_coroutine_test(test_suspend)

# Suspension infrastructure tests
add_coroutine_test(test_suspension_core)
add_coroutine_test(test_suspension_infrastructure)
add_coroutine_test(test_suspension_infrastructure_simple)
add_coroutine_test(test_suspension_fixes)
add_coroutine_test(test_comprehensive_suspension)

# Add gc_bridge tests if directory exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/gc_bridge")
    add_subdirectory(gc_bridge)
endif()
