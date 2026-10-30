cmake_minimum_required(VERSION 3.16)
project(ShootEmUpEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for tooling (clangd, IWYU)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/CompilerFlags.cmake)
include(cmake/GitHooks.cmake)

# Options
option(BUILD_TESTING "Build the automated test suite" ON)

# Discovery path for vcpkg packages
if(EXISTS "C:/vcpkg/installed/x64-windows")
    list(APPEND CMAKE_PREFIX_PATH "C:/vcpkg/installed/x64-windows")
endif()

find_package(SDL2 CONFIG REQUIRED)
set(GFLAGS_USE_TARGET_NAMESPACE ON)
find_package(gflags CONFIG REQUIRED)
find_package(glog CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

# Tier 1: Core Engine Library (Strictly zero external dependencies)
add_subdirectory(libs/ShooterCore)

# Tier 2: Platform Presentation Adapter (SDL2)
add_subdirectory(libs/SdlAdapter)

# Test Suite
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests/ShooterCoreTests)
    add_subdirectory(tests/SdlAdapterTests)
endif()

