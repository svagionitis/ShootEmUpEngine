# cmake/VcpkgDependencies.cmake
# Configures vcpkg package management on Windows and deploys runtime DLL dependencies.

if(WIN32)
    # Enable automatic applocal dependency copying if vcpkg toolchain is active
    set(VCPKG_APPLOCAL_DEPS ON CACHE BOOL "Automatically copy dependencies into output directory" FORCE)

    # Load vcpkg toolchain if not loaded prior to project()
    if(NOT DEFINED _VCPKG_TOOLCHAIN)
        if(DEFINED CMAKE_TOOLCHAIN_FILE AND EXISTS "${CMAKE_TOOLCHAIN_FILE}")
            include("${CMAKE_TOOLCHAIN_FILE}")
        elseif(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
            include("$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        elseif(EXISTS "C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
            include("C:/vcpkg/scripts/buildsystems/vcpkg.cmake")
        endif()
    endif()

    # Fallback discovery path for direct vcpkg installs
    if(EXISTS "C:/vcpkg/installed/x64-windows")
        list(APPEND CMAKE_PREFIX_PATH "C:/vcpkg/installed/x64-windows")
    endif()
    if(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/installed/x64-windows")
        list(APPEND CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/x64-windows")
    endif()
endif()

# Function: copy_executable_dependencies
# Copies runtime dependency DLLs (glog, gflags, GTest, SDL2, etc.) into the executable target's output directory.
function(copy_executable_dependencies TARGET_NAME)
    if(WIN32)
        set(RUNTIME_DEPS "")

        # Logging framework (glog and gflags)
        if(TARGET glog::glog)
            list(APPEND RUNTIME_DEPS $<TARGET_FILE:glog::glog>)
        endif()
        if(TARGET gflags::gflags)
            list(APPEND RUNTIME_DEPS $<TARGET_FILE:gflags::gflags>)
        endif()

        # GoogleTest framework
        if(TARGET GTest::gtest)
            list(APPEND RUNTIME_DEPS $<TARGET_FILE:GTest::gtest>)
        endif()

        # Presentation layer (SDL2 shared library)
        if(TARGET SDL2::SDL2)
            get_target_property(SDL2_TYPE SDL2::SDL2 TYPE)
            if(SDL2_TYPE STREQUAL "SHARED_LIBRARY")
                list(APPEND RUNTIME_DEPS $<TARGET_FILE:SDL2::SDL2>)
            endif()
        endif()

        # Additional targets specified by caller
        foreach(EXTRA_TGT IN LISTS ARGN)
            if(TARGET ${EXTRA_TGT})
                get_target_property(TGT_TYPE ${EXTRA_TGT} TYPE)
                if(TGT_TYPE STREQUAL "SHARED_LIBRARY")
                    list(APPEND RUNTIME_DEPS $<TARGET_FILE:${EXTRA_TGT}>)
                endif()
            endif()
        endforeach()

        if(RUNTIME_DEPS)
            add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${RUNTIME_DEPS}
                    $<TARGET_FILE_DIR:${TARGET_NAME}>
                COMMENT "Deploying runtime dependencies (glog, etc.) to $<TARGET_FILE_DIR:${TARGET_NAME}>"
            )
        endif()
    endif()
endfunction()

