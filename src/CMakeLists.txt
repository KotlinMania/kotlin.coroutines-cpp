cmake_minimum_required(VERSION 3.16)
project(kotlinx-coroutines-core VERSION 1.0.0 LANGUAGES CXX)

# Enable C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# Source layout (transliteration-first)
#
# The Kotlin ⇄ C++ refactor is moving from older multi-root layouts
# (e.g., kotlinx/coroutines/common/src, native/src, concurrent/src)
# to a direct package-mirroring tree:
#   src/kotlinx/coroutine/**  (or legacy src/kotlinx/coroutines/**)
#
# To keep the build stable during this migration, we discover sources under a
# configurable root and exclude test/tool/example trees.
# -----------------------------------------------------------------------------

# Allow parent/root CMake to override the folder name.
set(KOTLINX_COROUTINES_SRC_DIR "kotlinx/coroutines" CACHE STRING
    "Coroutine sources root relative to src/ (kotlinx/coroutine or kotlinx/coroutines)")

set(KOTLINX_COROUTINES_SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${KOTLINX_COROUTINES_SRC_DIR}")

if(NOT EXISTS "${KOTLINX_COROUTINES_SRC_ROOT}")
    message(FATAL_ERROR "Coroutine sources root not found: ${KOTLINX_COROUTINES_SRC_ROOT}")
endif()

# Collect all implementation sources under the root.
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
    "${KOTLINX_COROUTINES_SRC_ROOT}/*.cpp"
)

# Exclude non-library sources.
list(FILTER CORE_SOURCES EXCLUDE REGEX "/tools/")
list(FILTER CORE_SOURCES EXCLUDE REGEX "/examples/")
list(FILTER CORE_SOURCES EXCLUDE REGEX "/test/")
list(FILTER CORE_SOURCES EXCLUDE REGEX "/integration/")

# Create the core library
add_library(kotlinx-coroutines-core ${CORE_SOURCES})

if(KOTLINX_BUILD_CLANG_SUSPEND_PLUGIN)
    target_compile_options(kotlinx-coroutines-core PRIVATE -fplugin=$<TARGET_FILE:KotlinxSuspendPlugin>)
    add_dependencies(kotlinx-coroutines-core KotlinxSuspendPlugin)
endif()

# Set include directories for this target
# Headers are now co-located with sources in src/kotlinx/coroutines/
target_include_directories(kotlinx-coroutines-core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

# Link against threading libraries
find_package(Threads REQUIRED)
target_link_libraries(kotlinx-coroutines-core PUBLIC Threads::Threads)

# Set library properties
set_target_properties(kotlinx-coroutines-core PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Install the library
install(TARGETS kotlinx-coroutines-core
    EXPORT kotlinx-coroutines-core-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Install the export
install(EXPORT kotlinx-coroutines-core-targets
    FILE kotlinx-coroutines-core-targets.cmake
    NAMESPACE kotlinx::
    DESTINATION lib/cmake/kotlinx-coroutines-core
)

