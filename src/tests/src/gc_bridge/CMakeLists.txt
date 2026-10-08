# This optional host fixture runs the actual Kotlin/Native runtime.
set(KOTLINX_KONANC "" CACHE FILEPATH "Kotlin/Native compiler for the Native ABI contract")
set(KOTLINX_NATIVE_CLANGXX "" CACHE FILEPATH "Clang++ matching that Native compiler's LLVM")
if(NOT APPLE OR NOT CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
    message(FATAL_ERROR "The Native ABI host fixture currently requires macOS arm64")
endif()
foreach(GC_TOOL KOTLINX_KONANC KOTLINX_NATIVE_CLANGXX)
    if(NOT EXISTS "${${GC_TOOL}}")
        message(FATAL_ERROR "${GC_TOOL} must name the actual Native toolchain executable")
    endif()
endforeach()
set(GC_NATIVE_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/native_reference_contract.kexe")
add_custom_command(OUTPUT "${GC_NATIVE_OUTPUT}"
    COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/build_and_test_gc_bridge.sh"
        "${KOTLINX_KONANC}" "${KOTLINX_NATIVE_CLANGXX}" "${CMAKE_CURRENT_BINARY_DIR}" --build-only
    DEPENDS NativeReferenceContract.cpp NativeReferenceContract.kt build_and_test_gc_bridge.sh
        "${PROJECT_SOURCE_DIR}/src/kotlinx/coroutines/KotlinGCBridge.hpp"
        "${PROJECT_SOURCE_DIR}/src/kotlinx/coroutines/KotlinGCBridge.cpp"
        "${PROJECT_SOURCE_DIR}/src/kotlinx/coroutines/tools/kotlinc_native_ref/kotlin/native/Runtime.hpp"
        "${PROJECT_SOURCE_DIR}/src/kotlinx/coroutines/tools/kotlinc_native_ref/kotlin/native/Runtime.cpp"
    VERBATIM)
add_custom_target(native_reference_contract ALL DEPENDS "${GC_NATIVE_OUTPUT}")
add_test(NAME native_reference_contract COMMAND "${GC_NATIVE_OUTPUT}")
set_tests_properties(native_reference_contract PROPERTIES TIMEOUT 60)

set(TYPE_NAMES_NATIVE_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/native_type_names_contract.kexe")
add_custom_command(OUTPUT "${TYPE_NAMES_NATIVE_OUTPUT}"
    COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/build_native_type_names_contract.sh"
        "${KOTLINX_KONANC}" "${KOTLINX_NATIVE_CLANGXX}" "${CMAKE_CURRENT_BINARY_DIR}"
    DEPENDS NativeTypeNamesContract.cpp NativeTypeNamesContract.kt NativeTypeNamesUnpackaged.kt
        build_native_type_names_contract.sh
        "${PROJECT_SOURCE_DIR}/src/kotlinx/coroutines/KotlinGCBridge.hpp"
        "${PROJECT_SOURCE_DIR}/src/kotlinx/coroutines/KotlinGCBridge.cpp"
        "${PROJECT_SOURCE_DIR}/src/kotlinx/coroutines/tools/kotlinc_native_ref/kotlin/native/internal/TypeInfoNames.hpp"
        "${PROJECT_SOURCE_DIR}/src/kotlinx/coroutines/tools/kotlinc_native_ref/kotlin/native/internal/TypeInfoNames.cpp"
        "${PROJECT_SOURCE_DIR}/src/kotlinx/coroutines/tools/kotlinc_native_ref/kotlin/native/internal/TypeInfoNamesNative.cpp"
        "${PROJECT_SOURCE_DIR}/tmp/kotlin/kotlin-native/runtime/src/main/kotlin/kotlin/native/internal/TypeInfoNames.kt"
    VERBATIM)
add_custom_target(native_type_names_contract ALL DEPENDS "${TYPE_NAMES_NATIVE_OUTPUT}")
add_test(NAME native_type_names_contract COMMAND "${TYPE_NAMES_NATIVE_OUTPUT}")
set_tests_properties(native_type_names_contract PROPERTIES TIMEOUT 60)

set(ARRAY_NATIVE_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/native_array_contract.kexe")
find_package(Python3 3.8 REQUIRED COMPONENTS Interpreter)
if(NOT TARGET kxs_array_util_test)
    message(FATAL_ERROR "Native array comparison requires KOTLINX_BUILD_KXS_INJECT=ON")
endif()
add_custom_command(OUTPUT "${ARRAY_NATIVE_OUTPUT}"
    COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/build_native_array_contract.sh"
        "${KOTLINX_KONANC}" "${KOTLINX_NATIVE_CLANGXX}" "${CMAKE_CURRENT_BINARY_DIR}"
    DEPENDS NativeArrayUtilContract.cpp NativeArrayUtilContract.kt build_native_array_contract.sh
        "${PROJECT_SOURCE_DIR}/src/kotlinx/coroutines/KotlinGCBridge.hpp"
        "${PROJECT_SOURCE_DIR}/src/kotlinx/coroutines/KotlinGCBridge.cpp"
        "${PROJECT_SOURCE_DIR}/src/kotlinx/coroutines/tools/kotlinc_native_ref/kotlin/collections/NativeArrayUtil.hpp"
        "${PROJECT_SOURCE_DIR}/src/kotlinx/coroutines/tools/kotlinc_native_ref/kotlin/collections/NativeArrayUtil.cpp"
    VERBATIM)
add_custom_target(native_array_contract ALL DEPENDS "${ARRAY_NATIVE_OUTPUT}")
add_dependencies(native_array_contract kxs_array_util_test)
add_test(NAME native_array_contract
    COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/src/tests/ir/test_native_array_util.py"
        --cpp "$<TARGET_FILE:kxs_array_util_test>" --native "${ARRAY_NATIVE_OUTPUT}"
        --work-dir "${CMAKE_CURRENT_BINARY_DIR}/array-execution")
set_tests_properties(native_array_contract PROPERTIES TIMEOUT 120)

set(INT_ARRAY_NATIVE_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/native_int_array_contract.kexe")
if(NOT TARGET kxs_int_array_test)
    message(FATAL_ERROR "Native integer-array comparison requires KOTLINX_BUILD_KXS_INJECT=ON")
endif()
add_custom_command(OUTPUT "${INT_ARRAY_NATIVE_OUTPUT}"
    COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/build_native_int_array_contract.sh"
        "${KOTLINX_KONANC}" "${KOTLINX_NATIVE_CLANGXX}" "${CMAKE_CURRENT_BINARY_DIR}"
    DEPENDS NativeIntArrayContract.cpp NativeIntArrayContract.kt build_native_int_array_contract.sh
        "${PROJECT_SOURCE_DIR}/src/kotlinx/coroutines/KotlinGCBridge.hpp"
        "${PROJECT_SOURCE_DIR}/src/kotlinx/coroutines/KotlinGCBridge.cpp"
        "${PROJECT_SOURCE_DIR}/src/kotlinx/coroutines/tools/kotlinc_native_ref/kotlin/collections/NativeArrayUtil.hpp"
    VERBATIM)
add_custom_target(native_int_array_contract ALL DEPENDS "${INT_ARRAY_NATIVE_OUTPUT}")
add_dependencies(native_int_array_contract kxs_int_array_test)
add_test(NAME native_int_array_contract
    COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/src/tests/ir/test_native_int_array.py"
        --cpp "$<TARGET_FILE:kxs_int_array_test>" --native "${INT_ARRAY_NATIVE_OUTPUT}"
        --work-dir "${CMAKE_CURRENT_BINARY_DIR}/int-array-execution")
set_tests_properties(native_int_array_contract PROPERTIES TIMEOUT 120)
