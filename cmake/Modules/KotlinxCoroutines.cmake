# KotlinxCoroutines.cmake
#
# Creates the kotlinx::coroutines INTERFACE library that provides:
#   - Header files for coroutine primitives
#   - Mandatory LLVM injection for suspend definitions
#   - Runtime library linking
#
# Usage:
#   include(KotlinxCoroutines)
#   target_link_libraries(my_app PRIVATE kotlinx::coroutines)
#   kxs_enable_suspend_dsl(my_app)  # Construct frames and inject resume dispatch
#
# The INTERFACE library propagates:
#   - INTERFACE_INCLUDE_DIRECTORIES: Header paths
#   - INTERFACE_COMPILE_DEFINITIONS: KXS_COROUTINES_ENABLED
#   - INTERFACE_LINK_LIBRARIES: Runtime dependencies
#
# Custom properties (CMake 3.19+):
#   - KXS_COROUTINE_TRANSFORM: Set to ON to enable IR transformation

cmake_minimum_required(VERSION 3.19)

#[============================================================================[
  Determine paths - works from both source tree and installed location
#]============================================================================]
get_filename_component(_KXS_MODULE_DIR "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)

# Check if we're in source tree (cmake/Modules/) or installed (lib/cmake/KotlinxCoroutines/)
if(EXISTS "${_KXS_MODULE_DIR}/../../src/kotlinx")
    # Source tree: cmake/Modules -> project root
    get_filename_component(_KXS_ROOT_DIR "${_KXS_MODULE_DIR}/../.." ABSOLUTE)
    set(_KXS_INCLUDE_DIR "${_KXS_ROOT_DIR}/src")
elseif(EXISTS "${_KXS_MODULE_DIR}/../../../include/kotlinx")
    # Installed: lib/cmake/KotlinxCoroutines -> prefix
    get_filename_component(_KXS_ROOT_DIR "${_KXS_MODULE_DIR}/../../.." ABSOLUTE)
    set(_KXS_INCLUDE_DIR "${_KXS_ROOT_DIR}/include")
else()
    message(FATAL_ERROR "KotlinxCoroutines.cmake requires source-tree src/kotlinx headers or installed include/kotlinx headers")
endif()

# Include the transform module
include("${_KXS_MODULE_DIR}/KotlinxCoroutineTransform.cmake")

#[============================================================================[
  Create the kotlinx::coroutines INTERFACE library
#]============================================================================]
if(NOT TARGET kotlinx::coroutines)
    add_library(kotlinx_coroutines INTERFACE)
    add_library(kotlinx::coroutines ALIAS kotlinx_coroutines)
    set_target_properties(kotlinx_coroutines PROPERTIES EXPORT_NAME coroutines)

    # Header files
    target_include_directories(kotlinx_coroutines INTERFACE
        $<BUILD_INTERFACE:${_KXS_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
    )

    # Compile definitions
    target_compile_definitions(kotlinx_coroutines INTERFACE
        KXS_COROUTINES_ENABLED=1
    )

    # C++20 language features used by the port and suspend authoring DSL.
    target_compile_features(kotlinx_coroutines INTERFACE
        cxx_std_20
    )

    # Ordinary applications consume the translated C++ runtime through the
    # public package target, including its transitive thread dependency.
    if(TARGET kotlinx-coroutines-core)
        target_link_libraries(kotlinx_coroutines INTERFACE kotlinx-coroutines-core)
    elseif(TARGET kotlinx::kotlinx-coroutines-core)
        target_link_libraries(kotlinx_coroutines INTERFACE kotlinx::kotlinx-coroutines-core)
    endif()

    # Custom property to mark targets for transformation
    define_property(TARGET PROPERTY KXS_COROUTINE_TRANSFORM
        BRIEF_DOCS "Enable KXS coroutine IR transformation"
        FULL_DOCS "The KotlinxCoroutinePass LLVM plugin lowers persistent frame-label stores and resume dispatch inside Clang before optimization."
    )

    message(STATUS "[KXS] Created kotlinx::coroutines INTERFACE library")
endif()

#[============================================================================[
  Public: Enable suspend transformation for a target

  kxs_enable_suspend(<target>)

  This enables the IR transformation pipeline for targets that use
  suspend functions. Call this AFTER adding all sources to the target.
#]============================================================================]
function(kxs_enable_suspend TARGET)
    if(NOT TARGET ${TARGET})
        message(FATAL_ERROR "kxs_enable_suspend: ${TARGET} is not a target")
    endif()

    # Mark the target for transformation
    set_target_properties(${TARGET} PROPERTIES
        KXS_COROUTINE_TRANSFORM ON
    )

    # Apply the transformation
    kxs_enable_coroutine_transform(${TARGET})
endfunction()

# Kotlin-like annotated definitions: construct retained frames inside Clang,
# then inject their saved-address dispatch before optimization.
function(kxs_enable_suspend_dsl TARGET)
    kxs_enable_suspend("${TARGET}")
    kxs_enable_suspend_frontend("${TARGET}")
endfunction()

#[============================================================================[
  INTERFACE library for runtime-only (no transformation)

  Use this when you only need the headers and runtime, not IR transform.
  For example, code that CALLS suspend functions but doesn't DEFINE them.
#]============================================================================]
if(NOT TARGET kotlinx::coroutines_headers)
    add_library(kotlinx_coroutines_headers INTERFACE)
    add_library(kotlinx::coroutines_headers ALIAS kotlinx_coroutines_headers)
    set_target_properties(kotlinx_coroutines_headers PROPERTIES EXPORT_NAME coroutines_headers)

    target_include_directories(kotlinx_coroutines_headers INTERFACE
        $<BUILD_INTERFACE:${_KXS_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
    )

    target_compile_features(kotlinx_coroutines_headers INTERFACE
        cxx_std_20
    )
endif()

#[============================================================================[
  Convenience macro for the common pattern:

  kxs_add_executable(name SOURCES src1.cpp src2.cpp)

  Equivalent to:
    add_executable(name src1.cpp src2.cpp)
    target_link_libraries(name PRIVATE kotlinx::coroutines)
    kxs_enable_suspend_dsl(name)
#]============================================================================]
macro(kxs_add_executable TARGET_NAME)
    cmake_parse_arguments(KXS "" "" "SOURCES" ${ARGN})

    add_executable(${TARGET_NAME} ${KXS_SOURCES})
    target_link_libraries(${TARGET_NAME} PRIVATE kotlinx::coroutines)
    kxs_enable_suspend_dsl(${TARGET_NAME})
endmacro()

#[============================================================================[
  Convenience macro for libraries with suspend functions:

  kxs_add_library(name SOURCES src1.cpp src2.cpp)
#]============================================================================]
macro(kxs_add_library TARGET_NAME)
    cmake_parse_arguments(KXS "" "TYPE" "SOURCES" ${ARGN})

    if(NOT KXS_TYPE)
        set(KXS_TYPE STATIC)
    endif()

    add_library(${TARGET_NAME} ${KXS_TYPE} ${KXS_SOURCES})
    target_link_libraries(${TARGET_NAME} PUBLIC kotlinx::coroutines)
    kxs_enable_suspend_dsl(${TARGET_NAME})
endmacro()

# Note: Installation is handled in the main CMakeLists.txt when building
# the full kotlinx.coroutines-cpp project. The kxs_install() function
# has been removed to avoid duplication.
