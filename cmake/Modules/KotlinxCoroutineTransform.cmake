# KotlinxCoroutineTransform.cmake
#
# CMake module for transforming C++ coroutines with suspend points into
# computed-goto state machines matching Kotlin/Native's LLVM IR pattern.
#
# Usage:
#   include(KotlinxCoroutineTransform)
#   kxs_enable_coroutine_transform(target)
#
# This module provides:
#   kxs_enable_coroutine_transform(<target>) - Enable IR transformation for a target
#   kxs_transform_ir(<input> <output>)       - Transform a single .ll file
#
# The transformation:
#   1. Finds calls to __kxs_suspend_point(i32 <id>)
#   2. Replaces them with computed-goto dispatch (indirectbr + blockaddress)
#   3. Generates resume labels for each suspend point
#
# This matches Kotlin/Native's coroutine lowering pattern exactly.

cmake_minimum_required(VERSION 3.18)

#[============================================================================[
  Internal: Transform LLVM IR text to inject coroutine dispatch
#]============================================================================]
function(_kxs_transform_ir_impl INPUT_FILE OUTPUT_FILE)
    set(_SCRIPT "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/kxs_transform_ir.cmake")
    if(NOT EXISTS "${_SCRIPT}")
        get_filename_component(_MODULE_DIR "${CMAKE_CURRENT_LIST_FILE}" DIRECTORY)
        set(_SCRIPT "${_MODULE_DIR}/kxs_transform_ir.cmake")
    endif()
    if(NOT EXISTS "${_SCRIPT}")
        message(FATAL_ERROR "_kxs_transform_ir_impl: Script not found: ${_SCRIPT}")
    endif()

    execute_process(
        COMMAND ${CMAKE_COMMAND}
            -DINPUT_FILE=${INPUT_FILE}
            -DOUTPUT_FILE=${OUTPUT_FILE}
            -P "${_SCRIPT}"
        RESULT_VARIABLE _RES
    )
    if(NOT _RES EQUAL 0)
        message(FATAL_ERROR "kxs_transform_ir failed for ${INPUT_FILE}")
    endif()
endfunction()

#[============================================================================[
  Public: Transform a single LLVM IR file

  kxs_transform_ir(<input.ll> <output.ll>)
#]============================================================================]
function(kxs_transform_ir INPUT_FILE OUTPUT_FILE)
    if(NOT EXISTS "${INPUT_FILE}")
        message(FATAL_ERROR "kxs_transform_ir: Input file does not exist: ${INPUT_FILE}")
    endif()

    _kxs_transform_ir_impl("${INPUT_FILE}" "${OUTPUT_FILE}")
endfunction()

#[============================================================================[
  Public: Enable coroutine transformation for a target

  kxs_enable_coroutine_transform(<target>)

  This sets up custom commands to:
  1. Compile .cpp to .ll (LLVM IR text)
  2. Transform the IR with suspend point lowering
  3. Compile transformed .ll to .o
#]============================================================================]
function(kxs_enable_coroutine_transform TARGET)
    if(NOT TARGET ${TARGET})
        message(FATAL_ERROR "kxs_enable_coroutine_transform: ${TARGET} is not a target")
    endif()

    # Get target sources
    get_target_property(TARGET_SOURCES ${TARGET} SOURCES)

    # Get this module's directory for the transform script
    set(KXS_MODULE_DIR "${CMAKE_CURRENT_FUNCTION_LIST_DIR}")
    set(KXS_TRANSFORM_SCRIPT "${KXS_MODULE_DIR}/kxs_transform_ir.cmake")

    # For each .cpp source, set up the transformation pipeline
    foreach(SOURCE ${TARGET_SOURCES})
        get_filename_component(SOURCE_EXT "${SOURCE}" EXT)
        if(SOURCE_EXT STREQUAL ".cpp" OR SOURCE_EXT STREQUAL ".cxx" OR SOURCE_EXT STREQUAL ".cc")
            get_filename_component(SOURCE_NAME "${SOURCE}" NAME_WE)
            get_filename_component(SOURCE_ABS "${SOURCE}" ABSOLUTE)

            set(LL_FILE "${CMAKE_CURRENT_BINARY_DIR}/${SOURCE_NAME}.ll")
            set(TRANSFORMED_LL "${CMAKE_CURRENT_BINARY_DIR}/${SOURCE_NAME}.kxs.ll")
            set(OBJ_FILE "${CMAKE_CURRENT_BINARY_DIR}/${SOURCE_NAME}.kxs.o")

            # Step 1: Compile to LLVM IR
            add_custom_command(
                OUTPUT "${LL_FILE}"
                COMMAND ${CMAKE_CXX_COMPILER}
                    -emit-llvm -S
                    "$<TARGET_PROPERTY:${TARGET},COMPILE_OPTIONS>"
                    "$<TARGET_PROPERTY:${TARGET},INCLUDE_DIRECTORIES>"
                    "${SOURCE_ABS}"
                    -o "${LL_FILE}"
                DEPENDS "${SOURCE_ABS}"
                COMMENT "[KXS] Emitting LLVM IR: ${SOURCE_NAME}.ll"
                VERBATIM
            )

            # Step 2: Transform IR (using cmake -P)
            add_custom_command(
                OUTPUT "${TRANSFORMED_LL}"
                COMMAND ${CMAKE_COMMAND}
                    -DINPUT_FILE="${LL_FILE}"
                    -DOUTPUT_FILE="${TRANSFORMED_LL}"
                    -P "${KXS_TRANSFORM_SCRIPT}"
                DEPENDS "${LL_FILE}" "${KXS_TRANSFORM_SCRIPT}"
                COMMENT "[KXS] Transforming: ${SOURCE_NAME}.ll -> ${SOURCE_NAME}.kxs.ll"
                VERBATIM
            )

            # Step 3: Compile transformed IR to object
            add_custom_command(
                OUTPUT "${OBJ_FILE}"
                COMMAND ${CMAKE_CXX_COMPILER}
                    -c "${TRANSFORMED_LL}"
                    -o "${OBJ_FILE}"
                DEPENDS "${TRANSFORMED_LL}"
                COMMENT "[KXS] Compiling transformed IR: ${SOURCE_NAME}.kxs.o"
                VERBATIM
            )

            # Add the object file to the target
            target_sources(${TARGET} PRIVATE "${OBJ_FILE}")
        endif()
    endforeach()

    message(STATUS "[KXS] Enabled coroutine transform for target: ${TARGET}")
endfunction()

#[============================================================================[
  Standalone script mode - for cmake -P invocation
#]============================================================================]
if(CMAKE_SCRIPT_MODE_FILE)
    if(DEFINED INPUT_FILE AND DEFINED OUTPUT_FILE)
        _kxs_transform_ir_impl("${INPUT_FILE}" "${OUTPUT_FILE}")
    endif()
endif()
