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
    if(NOT EXISTS "${TEST_SOURCE}")
        set(TEST_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/flow/${TEST_NAME}.cpp")
    endif()
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
        message(STATUS "Test source not found: ${TEST_NAME}")
    endif()
endfunction()

# Core coroutine tests
add_coroutine_test(test_timeout)
add_coroutine_test(test_long_saturated_math)
target_compile_options(test_long_saturated_math PRIVATE -UNDEBUG)
add_coroutine_test(test_native_exception_namespace)
add_coroutine_test(test_job_impl_initialization)
target_sources(test_job_impl_initialization PRIVATE src/test_job_impl_initialization_main.cpp)
target_compile_options(test_job_impl_initialization PRIVATE -UNDEBUG)

# Transliterated Kotlin tests
add_coroutine_test(AsyncTest)
add_coroutine_test(JobTest)
add_coroutine_test(DurationToMillisTest)
target_sources(DurationToMillisTest PRIVATE src/DurationToMillisTestMain.cpp)
add_coroutine_test(test_lock_free_list)
set_tests_properties(test_lock_free_list PROPERTIES TIMEOUT 30)

# Suspension infrastructure and flow tests
add_coroutine_test(test_suspension_core)
add_coroutine_test(test_continuation_dispatch)
set_tests_properties(test_continuation_dispatch PROPERTIES TIMEOUT 15)
add_coroutine_test(test_limit_suspension)
set_tests_properties(test_limit_suspension PROPERTIES TIMEOUT 15)
add_coroutine_test(test_logic_suspension)
set_tests_properties(test_logic_suspension PROPERTIES TIMEOUT 15)
add_coroutine_test(test_channel_as_flow_smoke)
add_coroutine_test(test_channel_consumption)
set_tests_properties(test_channel_consumption PROPERTIES TIMEOUT 15)
add_coroutine_test(test_select_arguments)
set_tests_properties(test_select_arguments PROPERTIES TIMEOUT 15)
add_coroutine_test(test_cancellable_start)
set_tests_properties(test_cancellable_start PROPERTIES TIMEOUT 15)
add_coroutine_test(BuildersTest)
set_tests_properties(BuildersTest PROPERTIES TIMEOUT 15)
add_coroutine_test(test_channel_rendezvous)
set_tests_properties(test_channel_rendezvous PROPERTIES TIMEOUT 15)
add_coroutine_test(test_sync)
add_coroutine_test(test_concurrent_native)
set_tests_properties(test_concurrent_native PROPERTIES TIMEOUT 15)
add_coroutine_test(test_combine_zip_smoke)
add_coroutine_test(test_share_hot_flow_smoke)
add_coroutine_test(test_sharing_suspension)
set_tests_properties(test_sharing_suspension PROPERTIES TIMEOUT 15)
add_coroutine_test(test_transform_errors_smoke)
add_coroutine_test(test_transform_suspension)
set_tests_properties(test_transform_suspension PROPERTIES TIMEOUT 15)
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
set_tests_properties(AsyncTest JobTest CollectLatestTest PROPERTIES TIMEOUT 30)
if(TARGET test_plugin_canonical AND KOTLINX_BUILD_CLANG_SUSPEND_PLUGIN)
    target_compile_options(test_plugin_canonical PRIVATE -fplugin=$<TARGET_FILE:KotlinxSuspendPlugin>)
    add_dependencies(test_plugin_canonical KotlinxSuspendPlugin)
endif()

# Native ABI execution requires the real toolchain; no standalone substitute.
if(KOTLIN_NATIVE_RUNTIME_AVAILABLE)
    add_subdirectory(src/gc_bridge)
endif()

# Exercise native injection and the actual CMake compile/link pipeline.
if(CMAKE_CXX_COMPILER_ID MATCHES "^(AppleClang|Clang)$")
    find_package(Python3 3.8 QUIET COMPONENTS Interpreter)
    if(Python3_FOUND)
        if(TARGET kxs-inject)
            set(KXS_TEST_INJECTOR "$<TARGET_FILE:kxs-inject>")
        else()
            if(NOT KXS_INJECT_EXECUTABLE)
                find_program(KXS_INJECT_EXECUTABLE NAMES kxs-inject REQUIRED)
            endif()
            set(KXS_TEST_INJECTOR "${KXS_INJECT_EXECUTABLE}")
        endif()
        if(TARGET KotlinxCoroutinePass)
            set(KXS_TEST_PASS_PLUGIN "$<TARGET_FILE:KotlinxCoroutinePass>")
        else()
            set(KXS_TEST_PASS_PLUGIN "${KXS_LLVM_PASS_PLUGIN}")
        endif()
        get_filename_component(KXS_TEST_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
        add_test(NAME test_ir_pipeline
            COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/ir/test_ir_pipeline.py"
                --root "${KXS_TEST_ROOT}"
                --modules "${KXS_TEST_ROOT}/cmake/Modules"
                --headers "${KXS_TEST_ROOT}/src"
                --core-test "${CMAKE_CURRENT_SOURCE_DIR}/src/test_suspension_core.cpp"
                --library "$<TARGET_FILE:kotlinx-coroutines-core>"
                "--library-link-options=${CMAKE_CXX_FLAGS} ${CMAKE_EXE_LINKER_FLAGS}"
                --work-dir "${CMAKE_CURRENT_BINARY_DIR}/ir-regression"
                --compiler "${CMAKE_CXX_COMPILER}"
                --injector "${KXS_TEST_INJECTOR}"
                --plugin "${KXS_TEST_PASS_PLUGIN}"
                --cmake "${CMAKE_COMMAND}")
        set_tests_properties(test_ir_pipeline PROPERTIES TIMEOUT 120)
    endif()
endif()
