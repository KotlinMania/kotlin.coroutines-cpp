cmake_minimum_required(VERSION 3.18)
project(kxs_inject LANGUAGES CXX C)

# Prefer an installed LLVM development package (e.g. Homebrew). A vendored
# source checkout remains an optional fallback, not an installation requirement.
find_package(LLVM CONFIG QUIET)
if(LLVM_FOUND)
    message(STATUS "Building kxs-inject with LLVM ${LLVM_PACKAGE_VERSION}")
else()
    get_filename_component(KXS_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../.." ABSOLUTE)
    set(LLVM_SRC_DIR "${KXS_PROJECT_ROOT}/vendor/llvm-project/llvm")
    if(NOT EXISTS "${LLVM_SRC_DIR}/CMakeLists.txt")
        message(WARNING "LLVM development package and vendored sources are unavailable; skipping kxs-inject. Set LLVM_DIR to an installed LLVM CMake package.")
        return()
    endif()
    set(LLVM_TARGETS_TO_BUILD "AArch64" CACHE STRING "" FORCE)
    set(LLVM_ENABLE_PROJECTS "" CACHE STRING "" FORCE)
    foreach(KXS_COMPONENT TOOLS UTILS EXAMPLES TESTS BENCHMARKS DOCS)
        set(LLVM_BUILD_${KXS_COMPONENT} OFF CACHE BOOL "" FORCE)
        set(LLVM_INCLUDE_${KXS_COMPONENT} OFF CACHE BOOL "" FORCE)
    endforeach()
    add_subdirectory("${LLVM_SRC_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/llvm" EXCLUDE_FROM_ALL)
    set(LLVM_INCLUDE_DIRS "${LLVM_SRC_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}/llvm/include")
endif()

add_executable(kxs-inject kxs_inject.cpp)
target_compile_features(kxs-inject PRIVATE cxx_std_20)
target_include_directories(kxs-inject SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
separate_arguments(KXS_LLVM_DEFINITIONS NATIVE_COMMAND "${LLVM_DEFINITIONS}")
target_compile_options(kxs-inject PRIVATE ${KXS_LLVM_DEFINITIONS})
if(TARGET LLVM)
    target_link_libraries(kxs-inject PRIVATE LLVM)
else()
    llvm_map_components_to_libnames(KXS_LLVM_LIBRARIES IRReader BitWriter)
    target_link_libraries(kxs-inject PRIVATE ${KXS_LLVM_LIBRARIES})
endif()
install(TARGETS kxs-inject RUNTIME DESTINATION bin)

get_filename_component(KXS_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../.." ABSOLUTE)
find_package(Python3 3.8 QUIET COMPONENTS Interpreter)
if(Python3_FOUND)
    enable_testing()
    add_test(NAME test_kxs_inject
        COMMAND "${Python3_EXECUTABLE}" "${KXS_PROJECT_ROOT}/src/tests/ir/test_kxs_inject.py"
            --tool "$<TARGET_FILE:kxs-inject>"
            --root "${KXS_PROJECT_ROOT}"
            --headers "${KXS_PROJECT_ROOT}/src"
            --compiler "${CMAKE_CXX_COMPILER}"
            --work-dir "${CMAKE_CURRENT_BINARY_DIR}/ir-regression")
    set_tests_properties(test_kxs_inject PROPERTIES TIMEOUT 120)
endif()
