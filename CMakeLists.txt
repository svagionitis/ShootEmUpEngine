cmake_minimum_required(VERSION 3.16)
project(ShootEmUpEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for tooling (clangd, IWYU)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/CompilerFlags.cmake)

# Options
option(BUILD_TESTING "Build the automated test suite" ON)

# Tier 1: Core Engine Library (Strictly zero external dependencies)
add_subdirectory(libs/ShooterCore)

# Test Suite
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests/ShooterCoreTests)
endif()

