# Mandatory LLVM coroutine injection. The frontend supplies frame-field and
# resume-block identities; KotlinxCoroutinePass constructs stores and dispatch.
# See docs/suspension/IR_SUSPEND_LOWERING_SPEC.md for the Kotlin/Native contract.

cmake_minimum_required(VERSION 3.18)

function(_kxs_transform_ir_impl INPUT_FILE OUTPUT_FILE)
    if(NOT KXS_INJECT_EXECUTABLE)
        find_program(KXS_INJECT_EXECUTABLE NAMES kxs-inject REQUIRED)
    endif()
    execute_process(
        COMMAND "${CMAKE_COMMAND}"
            "-DINPUT_FILE=${INPUT_FILE}"
            "-DOUTPUT_FILE=${OUTPUT_FILE}"
            "-DKXS_INJECT_EXECUTABLE=${KXS_INJECT_EXECUTABLE}"
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

# Load the coroutine module pass into Clang. All lowering and optimization use
# the compiler's own LLVM values; no serialized IR or compiler launcher is used.
function(kxs_enable_coroutine_transform TARGET)
    if(NOT TARGET "${TARGET}")
        message(FATAL_ERROR "kxs_enable_coroutine_transform: ${TARGET} is not a target")
    endif()
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "^(AppleClang|Clang)$")
        message(FATAL_ERROR "KXS IR injection requires Clang")
    endif()
    get_target_property(_KXS_ENABLED "${TARGET}" KXS_COROUTINE_TRANSFORM_ENABLED)
    if(_KXS_ENABLED)
        return()
    endif()

    if(TARGET KotlinxCoroutinePass)
        get_target_property(_KXS_LLVM_VERSION KotlinxCoroutinePass KXS_LLVM_VERSION)
        if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL _KXS_LLVM_VERSION)
            message(FATAL_ERROR
                "KotlinxCoroutinePass uses LLVM ${_KXS_LLVM_VERSION}, but ${CMAKE_CXX_COMPILER} is ${CMAKE_CXX_COMPILER_VERSION}. Use the Clang from that LLVM development package, or build the plugin against your compiler's LLVM package.")
        endif()
        set(_KXS_PLUGIN "$<TARGET_FILE:KotlinxCoroutinePass>")
        add_dependencies("${TARGET}" KotlinxCoroutinePass)
        get_target_property(_KXS_PLUGIN_DIR KotlinxCoroutinePass LIBRARY_OUTPUT_DIRECTORY)
        if(NOT _KXS_PLUGIN_DIR)
            get_target_property(_KXS_PLUGIN_DIR KotlinxCoroutinePass BINARY_DIR)
        endif()
        # OBJECT_DEPENDS accepts concrete paths, not generator expressions.
        if(CMAKE_CONFIGURATION_TYPES)
            foreach(_KXS_CONFIG IN LISTS CMAKE_CONFIGURATION_TYPES)
                string(TOUPPER "${_KXS_CONFIG}" _KXS_CONFIG_UPPER)
                get_target_property(_KXS_CONFIG_DIR KotlinxCoroutinePass
                    "LIBRARY_OUTPUT_DIRECTORY_${_KXS_CONFIG_UPPER}")
                if(NOT _KXS_CONFIG_DIR STREQUAL _KXS_PLUGIN_DIR)
                    message(FATAL_ERROR "Set all KotlinxCoroutinePass LIBRARY_OUTPUT_DIRECTORY_<CONFIG> properties to the same directory for dependency tracking")
                endif()
            endforeach()
        endif()
        set(_KXS_PLUGIN_DEPENDENCY "${_KXS_PLUGIN_DIR}/KotlinxCoroutinePass${CMAKE_SHARED_MODULE_SUFFIX}")
    else()
        if(NOT KXS_LLVM_PASS_PLUGIN)
            find_file(KXS_LLVM_PASS_PLUGIN
                NAMES KotlinxCoroutinePass.dylib KotlinxCoroutinePass.so
                HINTS "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../.." REQUIRED)
        endif()
        if(NOT EXISTS "${KXS_LLVM_PASS_PLUGIN}")
            message(FATAL_ERROR "KotlinxCoroutinePass not found: ${KXS_LLVM_PASS_PLUGIN}")
        endif()
        if(KXS_LLVM_PASS_PLUGIN_VERSION AND
           NOT CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL KXS_LLVM_PASS_PLUGIN_VERSION)
            message(FATAL_ERROR "KotlinxCoroutinePass requires Clang ${KXS_LLVM_PASS_PLUGIN_VERSION}; selected compiler is ${CMAKE_CXX_COMPILER_VERSION}")
        endif()
        set(_KXS_PLUGIN "${KXS_LLVM_PASS_PLUGIN}")
        set(_KXS_PLUGIN_DEPENDENCY "${KXS_LLVM_PASS_PLUGIN}")
    endif()
    target_compile_options("${TARGET}" PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:-fpass-plugin=${_KXS_PLUGIN}>")
    # Rebuild affected objects when the lowering implementation changes.
    get_target_property(_KXS_SOURCES "${TARGET}" SOURCES)
    foreach(_KXS_SOURCE IN LISTS _KXS_SOURCES)
        set_property(SOURCE "${_KXS_SOURCE}" TARGET_DIRECTORY "${TARGET}"
            APPEND PROPERTY OBJECT_DEPENDS "${_KXS_PLUGIN_DEPENDENCY}")
    endforeach()
    set_property(TARGET "${TARGET}" PROPERTY KXS_COROUTINE_TRANSFORM_ENABLED ON)
    message(STATUS "[KXS] Enabled in-compiler coroutine lowering for target: ${TARGET}")
endfunction()

# Enable automatic frame construction and local retention in the frontend,
# together with the mandatory LLVM address injection stage.
function(kxs_enable_suspend_frontend TARGET)
    kxs_enable_coroutine_transform("${TARGET}")
    get_target_property(_KXS_FRONTEND_ENABLED "${TARGET}" KXS_SUSPEND_FRONTEND_ENABLED)
    if(_KXS_FRONTEND_ENABLED)
        return()
    endif()
    if(TARGET KotlinxSuspendPlugin)
        get_target_property(_KXS_FRONTEND_VERSION KotlinxSuspendPlugin KXS_LLVM_VERSION)
        get_target_property(_KXS_FRONTEND_DIR KotlinxSuspendPlugin LIBRARY_OUTPUT_DIRECTORY)
        if(NOT _KXS_FRONTEND_DIR)
            get_target_property(_KXS_FRONTEND_DIR KotlinxSuspendPlugin BINARY_DIR)
        endif()
        set(_KXS_FRONTEND "$<TARGET_FILE:KotlinxSuspendPlugin>")
        set(_KXS_FRONTEND_DEPENDENCY "${_KXS_FRONTEND_DIR}/KotlinxSuspendPlugin${CMAKE_SHARED_MODULE_SUFFIX}")
        add_dependencies("${TARGET}" KotlinxSuspendPlugin)
    else()
        find_file(KXS_CLANG_SUSPEND_PLUGIN NAMES KotlinxSuspendPlugin.so KotlinxSuspendPlugin.dylib
            HINTS "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../.." REQUIRED)
        if(NOT EXISTS "${KXS_CLANG_SUSPEND_PLUGIN}")
            message(FATAL_ERROR "KotlinxSuspendPlugin not found: ${KXS_CLANG_SUSPEND_PLUGIN}")
        endif()
        set(_KXS_FRONTEND "${KXS_CLANG_SUSPEND_PLUGIN}")
        set(_KXS_FRONTEND_DEPENDENCY "${KXS_CLANG_SUSPEND_PLUGIN}")
        set(_KXS_FRONTEND_VERSION "${KXS_LLVM_PASS_PLUGIN_VERSION}")
    endif()
    if(_KXS_FRONTEND_VERSION AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL _KXS_FRONTEND_VERSION)
        message(FATAL_ERROR "KotlinxSuspendPlugin requires Clang ${_KXS_FRONTEND_VERSION}; selected compiler is ${CMAKE_CXX_COMPILER_VERSION}")
    endif()
    target_compile_options("${TARGET}" PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:SHELL:-Xclang -load -Xclang '${_KXS_FRONTEND}' -Xclang -add-plugin -Xclang kotlinx-suspend>")
    get_target_property(_KXS_FRONTEND_SOURCES "${TARGET}" SOURCES)
    foreach(_KXS_SOURCE IN LISTS _KXS_FRONTEND_SOURCES)
        set_property(SOURCE "${_KXS_SOURCE}" TARGET_DIRECTORY "${TARGET}" APPEND PROPERTY
            OBJECT_DEPENDS "${_KXS_FRONTEND_DEPENDENCY}")
    endforeach()
    set_property(TARGET "${TARGET}" PROPERTY KXS_SUSPEND_FRONTEND_ENABLED ON)
endfunction()

if(CMAKE_SCRIPT_MODE_FILE AND DEFINED INPUT_FILE AND DEFINED OUTPUT_FILE)
    _kxs_transform_ir_impl("${INPUT_FILE}" "${OUTPUT_FILE}")
endif()
