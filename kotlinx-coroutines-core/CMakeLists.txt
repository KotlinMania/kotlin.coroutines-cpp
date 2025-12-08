cmake_minimum_required(VERSION 3.16)
project(kotlinx-coroutines-core VERSION 1.0.0 LANGUAGES CXX)

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
        ${CMAKE_CURRENT_SOURCE_DIR}/common/src/channels
        ${CMAKE_CURRENT_SOURCE_DIR}/common/src/sync
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