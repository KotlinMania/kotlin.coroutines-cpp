# Marker cleanup for state machines already lowered by Clang.
# Runtime dispatch, frame fields and Result handling remain in the source.
# See docs/suspension/IR_SUSPEND_LOWERING_SPEC.md for the Kotlin/Native contract.

cmake_minimum_required(VERSION 3.18)

function(_kxs_transform_ir_impl INPUT_FILE OUTPUT_FILE)
    execute_process(
        COMMAND "${CMAKE_COMMAND}"
            "-DINPUT_FILE=${INPUT_FILE}"
            "-DOUTPUT_FILE=${OUTPUT_FILE}"
            -P "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/kxs_transform_ir.cmake"
        RESULT_VARIABLE _KXS_RESULT
    )
    if(NOT _KXS_RESULT EQUAL 0)
        message(FATAL_ERROR "kxs_transform_ir failed for ${INPUT_FILE}")
    endif()
endfunction()

function(kxs_transform_ir INPUT_FILE OUTPUT_FILE)
    _kxs_transform_ir_impl("${INPUT_FILE}" "${OUTPUT_FILE}")
endfunction()

# Wrap CMake's real compile command, preserving its toolchain, target/source
# options, transitive usage requirements, generated sources and dependency files.
# Each source keeps CMake's unique object path and is compiled only once.
function(kxs_enable_coroutine_transform TARGET)
    if(NOT TARGET "${TARGET}")
        message(FATAL_ERROR "kxs_enable_coroutine_transform: ${TARGET} is not a target")
    endif()
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "^(AppleClang|Clang)$")
        message(FATAL_ERROR "KXS IR cleanup requires Clang")
    endif()
    if(NOT CMAKE_GENERATOR MATCHES "^(Ninja|Unix Makefiles)")
        message(FATAL_ERROR "KXS compiler launcher requires Ninja or Unix Makefiles")
    endif()
    get_target_property(_KXS_ENABLED "${TARGET}" KXS_COROUTINE_TRANSFORM_ENABLED)
    if(_KXS_ENABLED)
        return()
    endif()

    find_package(Python3 3.8 REQUIRED COMPONENTS Interpreter)
    get_target_property(_KXS_PREVIOUS_LAUNCHER "${TARGET}" CXX_COMPILER_LAUNCHER)
    if(NOT _KXS_PREVIOUS_LAUNCHER)
        set(_KXS_PREVIOUS_LAUNCHER "")
    endif()
    set(_KXS_LAUNCHER "${Python3_EXECUTABLE}"
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/kxs_compile.py"
        "--compiler=${CMAKE_CXX_COMPILER}" "--")
    if(_KXS_PREVIOUS_LAUNCHER)
        list(APPEND _KXS_LAUNCHER ${_KXS_PREVIOUS_LAUNCHER})
    endif()
    set_property(TARGET "${TARGET}" PROPERTY CXX_COMPILER_LAUNCHER "${_KXS_LAUNCHER}")
    set_property(TARGET "${TARGET}" PROPERTY KXS_COROUTINE_TRANSFORM_ENABLED ON)
    message(STATUS "[KXS] Enabled marker cleanup for target: ${TARGET}")
endfunction()

if(CMAKE_SCRIPT_MODE_FILE AND DEFINED INPUT_FILE AND DEFINED OUTPUT_FILE)
    _kxs_transform_ir_impl("${INPUT_FILE}" "${OUTPUT_FILE}")
endif()
