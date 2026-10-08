cmake_minimum_required(VERSION 3.18)
project(kxs_inject LANGUAGES CXX C)

# The plugin uses the selected compiler's LLVM development package.
# An unrelated source build cannot supply its runtime or registry.
find_package(LLVM CONFIG REQUIRED)
find_package(Threads REQUIRED)
message(STATUS "Building kxs-inject with LLVM ${LLVM_PACKAGE_VERSION}")

# Plugins must share the compiler's LLVM runtime and registry.
if(NOT TARGET LLVM)
    message(FATAL_ERROR "KotlinxCoroutinePass requires the compiler's shared LLVM development package")
endif()

add_executable(kxs-inject kxs_inject.cpp CoroutineInjection.cpp ../kotlinc_native_ref/IrToBitcode_coroutines.cpp ../kotlinc_native_ref/CodeGenerator.cpp ../kotlinc_native_ref/org/jetbrains/kotlin/ir/symbols/IrSymbol.cpp)
target_compile_features(kxs-inject PRIVATE cxx_std_20)
target_include_directories(kxs-inject SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
separate_arguments(KXS_LLVM_DEFINITIONS NATIVE_COMMAND "${LLVM_DEFINITIONS}")
target_compile_options(kxs-inject PRIVATE ${KXS_LLVM_DEFINITIONS})
target_link_libraries(kxs-inject PRIVATE LLVM)
install(TARGETS kxs-inject RUNTIME DESTINATION bin)

add_library(KotlinxCoroutinePass MODULE KotlinxCoroutinePass.cpp CoroutineInjection.cpp ../kotlinc_native_ref/IrToBitcode_coroutines.cpp ../kotlinc_native_ref/CodeGenerator.cpp ../kotlinc_native_ref/org/jetbrains/kotlin/ir/symbols/IrSymbol.cpp)
target_compile_features(KotlinxCoroutinePass PRIVATE cxx_std_20)
target_include_directories(KotlinxCoroutinePass SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_options(KotlinxCoroutinePass PRIVATE ${KXS_LLVM_DEFINITIONS})
target_link_libraries(KotlinxCoroutinePass PRIVATE LLVM)
set_target_properties(KotlinxCoroutinePass PROPERTIES PREFIX "")
set_property(TARGET KotlinxCoroutinePass PROPERTY KXS_LLVM_VERSION "${LLVM_PACKAGE_VERSION}")
install(TARGETS KotlinxCoroutinePass LIBRARY DESTINATION lib)

add_executable(kxs_codegen_test ../kotlinc_native_ref/IrToBitcodeCoroutinesTest.cpp ../kotlinc_native_ref/IrToBitcode_coroutines.cpp ../kotlinc_native_ref/CodeGenerator.cpp ../kotlinc_native_ref/org/jetbrains/kotlin/ir/symbols/IrSymbol.cpp)
target_include_directories(kxs_codegen_test SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_features(kxs_codegen_test PRIVATE cxx_std_20)
target_compile_options(kxs_codegen_test PRIVATE -UNDEBUG)
target_link_libraries(kxs_codegen_test PRIVATE LLVM)

# Translated LLVM callables and actual descriptor contracts used by source IR.
foreach(KXS_IR_TARGET kxs-inject KotlinxCoroutinePass kxs_codegen_test)
    target_link_libraries(${KXS_IR_TARGET} PRIVATE Threads::Threads)
    target_sources(${KXS_IR_TARGET} PRIVATE
        ../kotlinc_native_ref/org/jetbrains/kotlin/name/Name.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/name/FqName.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/name/FqNameUnsafe.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/name/ClassId.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/name/CallableId.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/name/SpecialNames.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/builtins/PrimitiveType.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/builtins/StandardNames.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/name/NativeRuntimeNames.cpp
        ../kotlinc_native_ref/KonanFqNames.cpp
        ../kotlinc_native_ref/InlineClasses.cpp
        ../kotlinc_native_ref/BinaryType.cpp
        ../kotlinc_native_ref/kotlin/Numbers.cpp
        ../kotlinc_native_ref/kotlin/collections/HashMapFunctions.cpp
        ../kotlinc_native_ref/DebugInfoC.cpp
        ../kotlinc_native_ref/Runtime.cpp
        ../kotlinc_native_ref/ContextUtils.cpp
        ../kotlinc_native_ref/LlvmAttributes.cpp
        ../kotlinc_native_ref/LlvmParamType.cpp
        ../kotlinc_native_ref/LlvmFunctionPrototype.cpp
        ../kotlinc_native_ref/LlvmCallable.cpp
        ../kotlinc_native_ref/LlvmUtils.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/ir/visitors/IrVisitorDispatch.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/ir/declarations/IrSymbolOwner.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/ir/declarations/IrValueDeclaration.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/ir/types/IrType.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/types/Variance.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/ir/symbols/IrValueSymbol.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/ir/symbols/IrClassifierSymbol.cpp
        ../kotlinc_native_ref/kotlin/concurrent/atomics/Atomics.cpp
        ../kotlinc_native_ref/kotlin/native/Runtime.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/descriptors/DeclarationDescriptor.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/descriptors/ValidateableDescriptor.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/descriptors/SourceElement.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/descriptors/SourceFile.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/descriptors/Visibility.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/descriptors/Visibilities.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/descriptors/DescriptorVisibility.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/descriptors/CallableMemberDescriptor.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/descriptors/Modality.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/descriptors/ClassKind.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/descriptors/ValueParameterDescriptor.cpp
        ../kotlinc_native_ref/org/jetbrains/kotlin/descriptors/annotations/AnnotatedImpl.cpp)
endforeach()

add_executable(kxs_numbers_test ../kotlinc_native_ref/NumbersContractTest.cpp
    ../kotlinc_native_ref/kotlin/Numbers.cpp
    ../kotlinc_native_ref/kotlin/collections/HashMapFunctions.cpp)
target_compile_features(kxs_numbers_test PRIVATE cxx_std_20)
target_compile_options(kxs_numbers_test PRIVATE -UNDEBUG)
add_test(NAME kxs_numbers_contract COMMAND kxs_numbers_test)

add_executable(kxs_grouping_test ../kotlinc_native_ref/GroupingContractTest.cpp)
target_compile_features(kxs_grouping_test PRIVATE cxx_std_20)
target_compile_options(kxs_grouping_test PRIVATE -UNDEBUG)
add_test(NAME kxs_grouping_contract COMMAND kxs_grouping_test)

add_executable(kxs_konan_primitive_test ../kotlinc_native_ref/KonanPrimitiveContractTest.cpp
    ../kotlinc_native_ref/InlineClasses.cpp
    ../kotlinc_native_ref/KonanFqNames.cpp
    ../kotlinc_native_ref/BinaryType.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/name/NativeRuntimeNames.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/name/Name.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/name/FqName.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/name/FqNameUnsafe.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/name/ClassId.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/builtins/PrimitiveType.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/builtins/StandardNames.cpp)
target_compile_features(kxs_konan_primitive_test PRIVATE cxx_std_20)
target_compile_options(kxs_konan_primitive_test PRIVATE -UNDEBUG)
add_test(NAME kxs_konan_primitive_contract COMMAND kxs_konan_primitive_test)

add_executable(kxs_primitive_type_test ../kotlinc_native_ref/PrimitiveTypeContractTest.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/builtins/PrimitiveType.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/builtins/StandardNames.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/name/Name.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/name/FqName.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/name/FqNameUnsafe.cpp)
target_compile_features(kxs_primitive_type_test PRIVATE cxx_std_20)
target_compile_options(kxs_primitive_type_test PRIVATE -UNDEBUG)
target_link_libraries(kxs_primitive_type_test PRIVATE Threads::Threads)
add_test(NAME kxs_primitive_type_contract COMMAND kxs_primitive_type_test)

add_executable(kxs_fq_name_test ../kotlinc_native_ref/FqNameContractTest.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/name/Name.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/name/FqName.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/name/FqNameUnsafe.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/name/ClassId.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/name/CallableId.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/name/SpecialNames.cpp)
target_compile_features(kxs_fq_name_test PRIVATE cxx_std_20)
target_compile_options(kxs_fq_name_test PRIVATE -UNDEBUG)
add_test(NAME kxs_fq_name_contract COMMAND kxs_fq_name_test)

add_executable(kxs_binary_type_test ../kotlinc_native_ref/BinaryTypeContractTest.cpp
    ../kotlinc_native_ref/BinaryType.cpp)
target_compile_features(kxs_binary_type_test PRIVATE cxx_std_20)
target_compile_options(kxs_binary_type_test PRIVATE -UNDEBUG)
add_test(NAME kxs_binary_type_contract COMMAND kxs_binary_type_test)

add_executable(kxs_debug_info_test ../kotlinc_native_ref/DebugInfoContractTest.cpp
    ../kotlinc_native_ref/DebugInfoC.cpp)
target_include_directories(kxs_debug_info_test SYSTEM PRIVATE ${LLVM_INCLUDE_DIRS})
target_compile_features(kxs_debug_info_test PRIVATE cxx_std_20)
target_compile_options(kxs_debug_info_test PRIVATE -UNDEBUG)
target_link_libraries(kxs_debug_info_test PRIVATE LLVM)
add_test(NAME kxs_debug_info_contract COMMAND kxs_debug_info_test)

add_executable(kxs_ir_name_test ../kotlinc_native_ref/NameContractTest.cpp
    ../kotlinc_native_ref/kotlin/native/Runtime.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/name/Name.cpp)
target_compile_features(kxs_ir_name_test PRIVATE cxx_std_20)
target_compile_options(kxs_ir_name_test PRIVATE -UNDEBUG)
enable_testing()
add_test(NAME kxs_ir_name_contract COMMAND kxs_ir_name_test)

add_executable(kxs_ir_type_operations_test ../kotlinc_native_ref/IrTypeOperationsContractTest.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/ir/types/IrType.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/types/Variance.cpp)
target_compile_features(kxs_ir_type_operations_test PRIVATE cxx_std_20)
target_compile_options(kxs_ir_type_operations_test PRIVATE -UNDEBUG)
add_test(NAME kxs_ir_type_operations_contract COMMAND kxs_ir_type_operations_test)

add_executable(kxs_class_kind_test ../kotlinc_native_ref/ClassKindContractTest.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/descriptors/ClassKind.cpp)
target_compile_features(kxs_class_kind_test PRIVATE cxx_std_20)
target_compile_options(kxs_class_kind_test PRIVATE -UNDEBUG)
add_test(NAME kxs_class_kind_contract COMMAND kxs_class_kind_test)

add_executable(kxs_descriptor_transport_test ../kotlinc_native_ref/DescriptorVisitorTransportTest.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/descriptors/SourceElement.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/descriptors/SourceFile.cpp)
target_compile_features(kxs_descriptor_transport_test PRIVATE cxx_std_20)
target_compile_options(kxs_descriptor_transport_test PRIVATE -UNDEBUG)
add_test(NAME kxs_descriptor_transport COMMAND kxs_descriptor_transport_test)

add_executable(kxs_visibility_test ../kotlinc_native_ref/VisibilityContractTest.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/descriptors/Visibility.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/descriptors/Visibilities.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/descriptors/DescriptorVisibility.cpp)
target_compile_features(kxs_visibility_test PRIVATE cxx_std_20)
target_compile_options(kxs_visibility_test PRIVATE -UNDEBUG)
add_test(NAME kxs_visibility_contract COMMAND kxs_visibility_test)

add_executable(kxs_array_iterator_test ../kotlinc_native_ref/ArrayIteratorContractTest.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/name/Name.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/descriptors/SourceElement.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/descriptors/SourceFile.cpp)
target_compile_features(kxs_array_iterator_test PRIVATE cxx_std_20)
target_compile_options(kxs_array_iterator_test PRIVATE -UNDEBUG)
add_test(NAME kxs_array_iterator_contract COMMAND kxs_array_iterator_test)

add_executable(kxs_array_util_test ../kotlinc_native_ref/ArrayUtilContractTest.cpp
    ../kotlinc_native_ref/kotlin/collections/ArrayUtil.cpp
    ../kotlinc_native_ref/org/jetbrains/kotlin/name/Name.cpp)
target_compile_features(kxs_array_util_test PRIVATE cxx_std_20)
target_compile_options(kxs_array_util_test PRIVATE -UNDEBUG)
add_test(NAME kxs_array_util_contract COMMAND kxs_array_util_test)

# Actual Native IntArray algorithms and HashMap capacity dependencies.
add_executable(kxs_int_array_test ../kotlinc_native_ref/IntArrayContractTest.cpp
    ../kotlinc_native_ref/kotlin/IntArray.cpp
    ../kotlinc_native_ref/kotlin/collections/PrimitiveIterators.cpp
    ../kotlinc_native_ref/kotlin/collections/ArrayUtil.cpp
    ../kotlinc_native_ref/kotlin/collections/ArraysNative.cpp
    ../kotlinc_native_ref/kotlin/collections/AbstractListFunctions.cpp)
target_compile_features(kxs_int_array_test PRIVATE cxx_std_20)
target_compile_options(kxs_int_array_test PRIVATE -UNDEBUG)
add_test(NAME kxs_int_array_contract COMMAND kxs_int_array_test)

# Native primitive atomic algorithms required by the compiler's lazy lock.
add_executable(kxs_atomic_int_test ../kotlinc_native_ref/AtomicIntContractTest.cpp
    ../kotlinc_native_ref/kotlin/concurrent/atomics/Atomics.cpp)
target_compile_features(kxs_atomic_int_test PRIVATE cxx_std_20)
target_compile_options(kxs_atomic_int_test PRIVATE -UNDEBUG)
target_link_libraries(kxs_atomic_int_test PRIVATE Threads::Threads)
add_test(NAME kxs_atomic_int_contract COMMAND kxs_atomic_int_test)

get_filename_component(KXS_PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../.." ABSOLUTE)
find_package(Python3 3.8 QUIET COMPONENTS Interpreter)
if(Python3_FOUND)
    enable_testing()
    set(KXS_RUNTIME_TEST_ARGS "")
    if(TARGET kotlinx-coroutines-core)
        list(APPEND KXS_RUNTIME_TEST_ARGS --library "$<TARGET_FILE:kotlinx-coroutines-core>")
    endif()
    add_test(NAME test_kxs_inject
        COMMAND "${Python3_EXECUTABLE}" "${KXS_PROJECT_ROOT}/src/tests/ir/test_kxs_inject.py"
            --tool "$<TARGET_FILE:kxs-inject>"
            --codegen-tool "$<TARGET_FILE:kxs_codegen_test>"
            --root "${KXS_PROJECT_ROOT}"
            --headers "${KXS_PROJECT_ROOT}/src"
            --compiler "${CMAKE_CXX_COMPILER}"
            --work-dir "${CMAKE_CURRENT_BINARY_DIR}/ir-regression"
            ${KXS_RUNTIME_TEST_ARGS})
    set_tests_properties(test_kxs_inject PROPERTIES TIMEOUT 120)
    if(TARGET kotlinx-coroutines-core)
        add_test(NAME test_kxs_compiler_pass
            COMMAND "${Python3_EXECUTABLE}" "${KXS_PROJECT_ROOT}/src/tests/ir/test_llvm_pass.py"
                --compiler "${CMAKE_CXX_COMPILER}"
                --plugin "$<TARGET_FILE:KotlinxCoroutinePass>"
                --root "${KXS_PROJECT_ROOT}"
                --library "$<TARGET_FILE:kotlinx-coroutines-core>"
                --work-dir "${CMAKE_CURRENT_BINARY_DIR}/compiler-pass-regression")
        set_tests_properties(test_kxs_compiler_pass PROPERTIES TIMEOUT 120)
    endif()
endif()
