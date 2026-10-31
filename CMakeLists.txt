cmake_minimum_required(VERSION 3.16)

# Automatic vcpkg toolchain integration on Windows
if(WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "Vcpkg toolchain file")
    elseif(EXISTS "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "Vcpkg toolchain file")
    endif()
endif()

project(ShootEmUpEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for tooling (clangd, IWYU)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/VcpkgDependencies.cmake)
include(cmake/CompilerFlags.cmake)
include(cmake/GitHooks.cmake)

# Options
option(BUILD_TESTING "Build the automated test suite" ON)

find_package(SDL2 CONFIG REQUIRED)
set(GFLAGS_USE_TARGET_NAMESPACE ON)
find_package(gflags CONFIG REQUIRED)
find_package(glog CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

# Tier 1: Core Engine Library (Strictly zero external dependencies)
add_subdirectory(libs/ShooterCore)

# Tier 2: Platform Presentation Adapter (SDL2)
add_subdirectory(libs/SdlAdapter)

# Tier 3: Standalone Game Executable
add_subdirectory(apps/ThexderGame)

# Test Suite
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests/ShooterCoreTests)
    add_subdirectory(tests/SdlAdapterTests)
    add_subdirectory(tests/ThexderGameTests)
endif()

