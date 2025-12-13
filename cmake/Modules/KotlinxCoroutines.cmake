# KotlinxCoroutines.cmake
#
# Creates the kotlinx::coroutines INTERFACE library that provides:
#   - Header files for coroutine primitives
#   - Automatic IR transformation for suspend functions
#   - Runtime library linking
#
# Usage:
#   include(KotlinxCoroutines)
#   target_link_libraries(my_app PRIVATE kotlinx::coroutines)
#   kxs_enable_suspend(my_app)  # Enable suspend transformation
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
if(EXISTS "${_KXS_MODULE_DIR}/../../include/kotlinx")
    # Source tree: cmake/Modules -> project root
    get_filename_component(_KXS_ROOT_DIR "${_KXS_MODULE_DIR}/../.." ABSOLUTE)
    set(_KXS_INCLUDE_DIR "${_KXS_ROOT_DIR}/include")
elseif(EXISTS "${_KXS_MODULE_DIR}/../../../include/kotlinx")
    # Installed: lib/cmake/KotlinxCoroutines -> prefix
    get_filename_component(_KXS_ROOT_DIR "${_KXS_MODULE_DIR}/../../.." ABSOLUTE)
    set(_KXS_INCLUDE_DIR "${_KXS_ROOT_DIR}/include")
else()
    # Fallback - assume we're in source tree
    get_filename_component(_KXS_ROOT_DIR "${_KXS_MODULE_DIR}/../.." ABSOLUTE)
    set(_KXS_INCLUDE_DIR "${_KXS_ROOT_DIR}/include")
endif()

# Include the transform module
include(KotlinxCoroutineTransform)

#[============================================================================[
  Create the kotlinx::coroutines INTERFACE library
#]============================================================================]
if(NOT TARGET kotlinx::coroutines)
    add_library(kotlinx_coroutines INTERFACE)
    add_library(kotlinx::coroutines ALIAS kotlinx_coroutines)

    # Header files
    target_include_directories(kotlinx_coroutines INTERFACE
        $<BUILD_INTERFACE:${_KXS_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
    )

    # Compile definitions
    target_compile_definitions(kotlinx_coroutines INTERFACE
        KXS_COROUTINES_ENABLED=1
    )

    # C++20 for coroutine support
    target_compile_features(kotlinx_coroutines INTERFACE
        cxx_std_20
    )

    # Custom property to mark targets for transformation
    define_property(TARGET PROPERTY KXS_COROUTINE_TRANSFORM
        BRIEF_DOCS "Enable KXS coroutine IR transformation"
        FULL_DOCS "When set to ON, the target's source files will be compiled to LLVM IR, transformed to add coroutine dispatch, and then compiled to object files."
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

#[============================================================================[
  INTERFACE library for runtime-only (no transformation)

  Use this when you only need the headers and runtime, not IR transform.
  For example, code that CALLS suspend functions but doesn't DEFINE them.
#]============================================================================]
if(NOT TARGET kotlinx::coroutines_headers)
    add_library(kotlinx_coroutines_headers INTERFACE)
    add_library(kotlinx::coroutines_headers ALIAS kotlinx_coroutines_headers)

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
    kxs_enable_suspend(name)
#]============================================================================]
macro(kxs_add_executable TARGET_NAME)
    cmake_parse_arguments(KXS "" "" "SOURCES" ${ARGN})

    add_executable(${TARGET_NAME} ${KXS_SOURCES})
    target_link_libraries(${TARGET_NAME} PRIVATE kotlinx::coroutines)
    kxs_enable_suspend(${TARGET_NAME})
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
    kxs_enable_suspend(${TARGET_NAME})
endmacro()

# Note: Installation is handled in the main CMakeLists.txt when building
# the full kotlinx.coroutines-cpp project. The kxs_install() function
# has been removed to avoid duplication.
