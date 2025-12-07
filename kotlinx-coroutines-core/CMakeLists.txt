cmake_minimum_required(VERSION 3.16)

# Create the main kotlinx-coroutines-core library
set(CORE_SOURCES
    common/src/Job.cpp
    common/src/JobSupport.cpp
    common/src/JobImpl.cpp
    common/src/Await.cpp
    common/src/channels/Channel.cpp
    common/src/channels/BufferedChannel.cpp
    common/src/channels/ConflatedBufferedChannel.cpp
    common/src/channels/BufferOverflow.cpp
    common/src/sync/Mutex.cpp
    common/src/sync/Semaphore.cpp
)

# Create the core library
add_library(kotlinx-coroutines-core ${CORE_SOURCES})

# Set include directories for this target
target_include_directories(kotlinx-coroutines-core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/common/src
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

# Create a config file for find_package
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/kotlinx-coroutines-core-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/kotlinx-coroutines-core-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/kotlinx-coroutines-core-config.cmake"
    INSTALL_DESTINATION lib/cmake/kotlinx-coroutines-core
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/kotlinx-coroutines-core-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/kotlinx-coroutines-core-config-version.cmake"
    DESTINATION lib/cmake/kotlinx-coroutines-core
)