cmake_minimum_required(VERSION 3.16)
project(kotlinx-coroutines-cpp VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable testing
enable_testing()

# Add compile options
add_compile_options(-Wall -Wextra -Wpedantic)

# Include directories
include_directories(include)

# Add subdirectories
add_subdirectory(kotlinx-coroutines-core)

# Add test directory if it exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test")
    add_subdirectory(test)
endif()

# Install headers
install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp"
)