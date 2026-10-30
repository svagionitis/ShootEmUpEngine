# cmake/GitHooks.cmake
# Git hooks and clang-format automation targets

find_program(CLANG_FORMAT_BIN
    NAMES clang-format clang-format-23 clang-format-22 clang-format-21 clang-format-20 clang-format-19 clang-format-18 clang-format-17
    DOC "Path to clang-format executable"
)

# Collect all project C++ headers and source files for repository-wide format targets
file(GLOB_RECURSE PROJECT_FORMAT_FILES
    LIST_DIRECTORIES false
    "${CMAKE_SOURCE_DIR}/libs/*.h"
    "${CMAKE_SOURCE_DIR}/libs/*.cpp"
    "${CMAKE_SOURCE_DIR}/apps/*.h"
    "${CMAKE_SOURCE_DIR}/apps/*.cpp"
    "${CMAKE_SOURCE_DIR}/tests/*.h"
    "${CMAKE_SOURCE_DIR}/tests/*.cpp"
)

if(CLANG_FORMAT_BIN)
    message(STATUS "Found clang-format: ${CLANG_FORMAT_BIN}")
    set(CLANG_FORMAT_CMD "${CLANG_FORMAT_BIN}")

    # Custom target to format all code files in-place
    add_custom_target(format
        COMMAND "${CLANG_FORMAT_BIN}" -i ${PROJECT_FORMAT_FILES}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Formatting repository source files with clang-format"
        VERBATIM
    )

    # Custom target to verify formatting without modifying files
    add_custom_target(check-format
        COMMAND "${CLANG_FORMAT_BIN}" --dry-run --Werror ${PROJECT_FORMAT_FILES}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Verifying repository source formatting with clang-format"
        VERBATIM
    )
else()
    message(WARNING "clang-format not found; format and check-format targets will not be available.")
    set(CLANG_FORMAT_CMD "clang-format")
endif()

# Install pre-commit hook if this is a Git repository
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
    set(GIT_HOOKS_DIR "${CMAKE_SOURCE_DIR}/.git/hooks")
    set(PRE_COMMIT_HOOK_IN "${CMAKE_SOURCE_DIR}/cmake/git-hooks/pre-commit.in")
    set(PRE_COMMIT_HOOK_OUT "${GIT_HOOKS_DIR}/pre-commit")

    if(EXISTS "${PRE_COMMIT_HOOK_IN}")
        # Configure hook file ensuring strictly LF line endings for POSIX shell compatibility
        configure_file(
            "${PRE_COMMIT_HOOK_IN}"
            "${PRE_COMMIT_HOOK_OUT}"
            @ONLY
            NEWLINE_STYLE LF
        )

        # On non-Windows platforms, set executable bit
        if(NOT WIN32)
            execute_process(
                COMMAND chmod +x "${PRE_COMMIT_HOOK_OUT}"
                OUTPUT_QUIET
                ERROR_QUIET
            )
        endif()

        # Dedicated target to re-install/update git hooks
        add_custom_target(install-git-hooks
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${PRE_COMMIT_HOOK_IN}"
                "${PRE_COMMIT_HOOK_OUT}"
            COMMENT "Updating git pre-commit hook in ${GIT_HOOKS_DIR}..."
            VERBATIM
        )
        message(STATUS "Installed git pre-commit hook to ${PRE_COMMIT_HOOK_OUT}")
    endif()
endif()

