
# debugging/CMakeLists.txt
project(kotlinx-coroutines-debug)

set(DEBUGGING_SOURCES
    src/kotlinx/coroutines/CoroutineInfo.cpp
)

add_library(kotlinx-coroutines-debug STATIC ${DEBUGGING_SOURCES})

target_include_directories(kotlinx-coroutines-debug PUBLIC
    ${CMAKE_SOURCE_DIR}/src/${KOTLINX_COROUTINES_SRC_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(kotlinx-coroutines-debug PUBLIC
    kotlinx-coroutines-core
)

if(KOTLINX_BUILD_CLANG_SUSPEND_PLUGIN)
    target_compile_options(kotlinx-coroutines-debug PRIVATE
        -fplugin=$<TARGET_FILE:KotlinxSuspendPlugin>
    )
    # Ensure plugin is built before we try to use it
    add_dependencies(kotlinx-coroutines-debug KotlinxSuspendPlugin)
endif()
