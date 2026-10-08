cmake_minimum_required(VERSION 3.16)
project(kotlinx_suspend_plugin LANGUAGES C CXX)

# Requested authoring requires the selected compiler's development packages.

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM CONFIG REQUIRED)
find_package(Clang CONFIG REQUIRED)
if(NOT TARGET clang-cpp OR NOT TARGET LLVM)
    message(FATAL_ERROR "KotlinxSuspendPlugin requires the compiler's shared Clang/LLVM development targets")
endif()

if(KOTLINX_BUILD_CLANG_SUSPEND_PLUGIN)
    # Bootstrap the translated compiler object ABI without linking those object
    # bodies into the metadata plugin that compiles them.
    add_library(KotlinxClassMetadataPlugin MODULE CompilerClassMetadataPlugin.cpp)
    target_include_directories(KotlinxClassMetadataPlugin PRIVATE
        ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
    target_compile_options(KotlinxClassMetadataPlugin PRIVATE -Wall -Wextra -Werror)
    target_link_libraries(KotlinxClassMetadataPlugin PRIVATE clang-cpp LLVM)
    set_target_properties(KotlinxClassMetadataPlugin PROPERTIES PREFIX ""
        KXS_LLVM_VERSION "${LLVM_PACKAGE_VERSION}")
    install(TARGETS KotlinxClassMetadataPlugin LIBRARY DESTINATION lib)

    # Actual translated compiler objects have their own C++ ABI. Native-only
    # rooted transport is linked exclusively by the explicit Native tests.
    function(kxs_enable_compiler_class_metadata target)
        add_dependencies(${target} KotlinxClassMetadataPlugin)
        target_compile_options(${target} PRIVATE
            "SHELL:-Xclang -load -Xclang $<TARGET_FILE:KotlinxClassMetadataPlugin>"
            "SHELL:-Xclang -add-plugin -Xclang kotlin-compiler-class-metadata")
    endfunction()
    set(_KXS_CLASS_RUNTIME "${CMAKE_CURRENT_SOURCE_DIR}/../kotlinc_native_ref")
    add_library(KotlinxCompilerObjects STATIC
        "${_KXS_CLASS_RUNTIME}/kotlin/ArrayIntrinsics.cpp"
        "${_KXS_CLASS_RUNTIME}/kotlin/collections/ArrayUtil.cpp"
        "${_KXS_CLASS_RUNTIME}/kotlin/collections/Arrays.cpp"
        "${_KXS_CLASS_RUNTIME}/kotlin/collections/CollectionToArray.cpp"
        "${_KXS_CLASS_RUNTIME}/kotlin/AnyIdentity.cpp"
        "${_KXS_CLASS_RUNTIME}/kotlin/AnyToString.cpp"
        "${_KXS_CLASS_RUNTIME}/kotlin/native/Runtime.cpp"
        "${_KXS_CLASS_RUNTIME}/kotlin/text/StringNumberConversions.cpp"
        "${_KXS_CLASS_RUNTIME}/kotlin/native/internal/CompilerObjectType.cpp"
        "${_KXS_CLASS_RUNTIME}/kotlin/native/internal/CompilerClassNames.cpp"
        "${_KXS_CLASS_RUNTIME}/kotlin/native/internal/TypeInfoNames.cpp"
        "${_KXS_CLASS_RUNTIME}/kotlin/native/internal/KClassIdentity.cpp"
        "${_KXS_CLASS_RUNTIME}/kotlin/native/internal/KClassNames.cpp"
        "${_KXS_CLASS_RUNTIME}/kotlin/native/internal/KClassPointerHash.cpp")
    set_target_properties(KotlinxCompilerObjects PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_compile_options(KotlinxCompilerObjects PRIVATE -Wall -Wextra -Werror)
    kxs_enable_compiler_class_metadata(KotlinxCompilerObjects)
    foreach(_KXS_COMPILER_TARGET kxs-inject KotlinxCoroutinePass kxs_codegen_test)
        if(TARGET ${_KXS_COMPILER_TARGET})
            target_link_libraries(${_KXS_COMPILER_TARGET} PRIVATE KotlinxCompilerObjects)
            kxs_enable_compiler_class_metadata(${_KXS_COMPILER_TARGET})
        endif()
    endforeach()

    add_library(KotlinxSuspendPlugin MODULE
            KotlinxSuspendPlugin.cpp
            FirSuspendCallChecker.cpp
            TailSuspendCallsCollector.cpp
            SuspendFunctionAnalyzer.cpp
            NativeSuspendLowering.cpp
            RestrictSuspensionUtils.cpp
            IrTypeUtils.cpp
            UpgradeCallableReferences.cpp
            NativeFunctionReferenceLowering.cpp
            AbstractFunctionReferenceLowering.cpp
            CompilerFrameLowering.cpp
    )

    target_include_directories(KotlinxSuspendPlugin PRIVATE
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
    )

    # Reliance on imported targets for include directories
    # target_link_libraries handles the dependencies.

    target_compile_definitions(KotlinxSuspendPlugin PUBLIC
        ${LLVM_DEFINITIONS}
        "-D__STDC_CONSTANT_MACROS"
        "-D__STDC_FORMAT_MACROS"
        "-D__STDC_LIMIT_MACROS"
    )

    if (LLVM_LINK_LLVM_DYLIB)
        target_compile_definitions(KotlinxSuspendPlugin PUBLIC "LLVM_LINK_LLVM_DYLIB")
    endif()

    # Share Clang/LLVM's runtime registries with the compiler loading the plugin.
    # A second static copy can register the same command-line options twice.
    target_link_libraries(KotlinxSuspendPlugin PRIVATE clang-cpp LLVM)
    target_link_libraries(KotlinxSuspendPlugin PRIVATE KotlinxCompilerObjects)
    kxs_enable_compiler_class_metadata(KotlinxSuspendPlugin)

    set_target_properties(KotlinxSuspendPlugin PROPERTIES
        PREFIX ""
        KXS_LLVM_VERSION "${LLVM_PACKAGE_VERSION}"
    )
    install(TARGETS KotlinxSuspendPlugin LIBRARY DESTINATION lib)

    include(CTest)
    if(BUILD_TESTING)
        add_executable(kxs_compiler_class_metadata_test
            "${_KXS_CLASS_RUNTIME}/CompilerClassMetadataContractTest.cpp"
            "${_KXS_CLASS_RUNTIME}/CompilerClassMetadataCrossUnit.cpp")
        target_link_libraries(kxs_compiler_class_metadata_test PRIVATE KotlinxCompilerObjects)
        kxs_enable_compiler_class_metadata(kxs_compiler_class_metadata_test)
        target_compile_options(kxs_compiler_class_metadata_test PRIVATE
            -UNDEBUG -Wall -Wextra -Werror)
        add_test(NAME kxs_compiler_class_metadata_contract
            COMMAND kxs_compiler_class_metadata_test)
    endif()
    if(BUILD_TESTING AND TARGET clang-cpp AND TARGET LLVM)
        add_executable(kxs_analyzer_test TailSuspendCallsCollector.cpp SuspendFunctionAnalyzerTest.cpp SuspendFunctionAnalyzer.cpp RestrictSuspensionUtils.cpp IrTypeUtils.cpp UpgradeCallableReferences.cpp NativeFunctionReferenceLowering.cpp AbstractFunctionReferenceLowering.cpp)
        target_include_directories(kxs_analyzer_test PRIVATE ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
        target_compile_options(kxs_analyzer_test PRIVATE -UNDEBUG -Wall -Wextra -Werror)
        target_link_libraries(kxs_analyzer_test PRIVATE clang-cpp LLVM)
        add_test(NAME kxs_analyzer_liveness COMMAND kxs_analyzer_test)
        add_executable(kxs_tail_collector_test TailSuspendCallsCollectorTest.cpp TailSuspendCallsCollector.cpp SuspendFunctionAnalyzer.cpp)
        target_include_directories(kxs_tail_collector_test PRIVATE ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
        target_compile_options(kxs_tail_collector_test PRIVATE -UNDEBUG -Wall -Wextra -Werror)
        target_link_libraries(kxs_tail_collector_test PRIVATE clang-cpp LLVM)
        add_test(NAME kxs_tail_collector COMMAND kxs_tail_collector_test)
    endif()
    if(TARGET kotlinx-coroutines-core)
        find_package(Python3 3.8 QUIET COMPONENTS Interpreter)
    endif()
    if(TARGET kotlinx-coroutines-core AND TARGET KotlinxCoroutinePass AND Python3_FOUND)
        get_filename_component(KXS_PLUGIN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../.." ABSOLUTE)
        find_program(KXS_PLUGIN_CLANG NAMES clang++ HINTS "${LLVM_TOOLS_BINARY_DIR}" NO_DEFAULT_PATH)
        if(KXS_PLUGIN_CLANG)
            add_test(NAME kxs_plugin_handoff
                COMMAND "${Python3_EXECUTABLE}" "${KXS_PLUGIN_ROOT}/src/tests/ir/test_suspend_plugin.py"
                    --root "${KXS_PLUGIN_ROOT}" --plugin "$<TARGET_FILE:KotlinxSuspendPlugin>"
                    --ir-plugin "$<TARGET_FILE:KotlinxCoroutinePass>"
                    --plugin-compiler "${KXS_PLUGIN_CLANG}" --compiler "${CMAKE_CXX_COMPILER}"
                    --library "$<TARGET_FILE:kotlinx-coroutines-core>"
                    "--library-link-options=${CMAKE_CXX_FLAGS} ${CMAKE_EXE_LINKER_FLAGS}"
                    --work-dir "${CMAKE_CURRENT_BINARY_DIR}/handoff-regression")
            file(GLOB _KXS_KONANC_DIRS "$ENV{HOME}/.konan/kotlin-native-prebuilt-*/bin")
            list(SORT _KXS_KONANC_DIRS ORDER DESCENDING)
            find_program(KXS_KONANC_EXECUTABLE NAMES konanc HINTS ${_KXS_KONANC_DIRS})
            if(KXS_KONANC_EXECUTABLE)
                add_test(NAME kxs_kotlin_native_handoff
                    COMMAND "${Python3_EXECUTABLE}" "${KXS_PLUGIN_ROOT}/src/tests/ir/test_kotlin_native_handoff.py"
                        --root "${KXS_PLUGIN_ROOT}" --plugin "$<TARGET_FILE:KotlinxSuspendPlugin>"
                        --ir-plugin "$<TARGET_FILE:KotlinxCoroutinePass>"
                        --compiler "${KXS_PLUGIN_CLANG}" --konanc "${KXS_KONANC_EXECUTABLE}"
                        --library "$<TARGET_FILE:kotlinx-coroutines-core>"
                        "--library-link-options=${CMAKE_CXX_FLAGS} ${CMAKE_EXE_LINKER_FLAGS}"
                        --work-dir "${CMAKE_CURRENT_BINARY_DIR}/kotlin-native-handoff")
                set_tests_properties(kxs_kotlin_native_handoff PROPERTIES TIMEOUT 180)
            else()
                message(STATUS "Actual Kotlin-generated frame regression needs KXS_KONANC_EXECUTABLE")
            endif()
        endif()
    endif()
endif()
