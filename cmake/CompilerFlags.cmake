# cmake/CompilerFlags.cmake
# Strict compiler warning and security hardening configuration

function(apply_compiler_flags TARGET_NAME)
    if(MSVC)
        # MSVC (cl.exe or clang-cl)
        target_compile_options(${TARGET_NAME} PRIVATE
            /W4
            /WX
            /permissive-
            /EHsc
            /sdl
            /guard:cf
            /utf-8
            /GS
            # Strict conversion & type safety diagnostics
            /w14242 # conversion from 'type1' to 'type2', possible loss of data
            /w14254 # conversion from 'type1:field_bits' to 'type2:field_bits'
            /w14263 # member function does not override any base class virtual member function
            /w14265 # class has virtual functions, but destructor is not virtual
            /w14287 # unsigned/negative constant mismatch
            /we4289 # nonstandard extension: loop control variable used outside loop
            /w14296 # expression is always false/true
            /w14311 # pointer truncation from 'type' to 'type'
            /w14545 # expression before comma evaluates to a function missing argument list
            /w14546 # function call before comma missing argument list
            /w14547 # operator before comma has no effect
            /w14549 # operator before comma has no effect; did you intend operator?
            /w14555 # expression has no effect
            /w14826 # conversion is sign-extended
            /w14905 # wide string literal cast to LPSTR
            /w14906 # string literal cast to LPWSTR
            /w14928 # illegal copy-initialization; multiple user conversions
        )

        target_link_options(${TARGET_NAME} PRIVATE
            /WX
            /GUARD:CF
            /DYNAMICBASE
            /HIGHENTROPYVA
            /NXCOMPAT
        )

        target_compile_definitions(${TARGET_NAME} PRIVATE
            _CRT_SECURE_NO_WARNINGS=0
        )

    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # GCC or Clang
        target_compile_options(${TARGET_NAME} PRIVATE
            -Wall
            -Wextra
            -pedantic
            -pedantic-errors
            -Werror
            -Wconversion
            -Wsign-conversion
            -Wshadow
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Wcast-align
            -Wunused
            -Woverloaded-virtual
            -Wnull-dereference
            -Wdouble-promotion
            -Wformat=2
            -Wimplicit-fallthrough
        )

        if(CMAKE_CXX_SIMULATE_ID MATCHES "MSVC")
            # Clang with GNU driver targeting MSVC (lld-link)
            target_link_options(${TARGET_NAME} PRIVATE
                "LINKER:/WX"
                "LINKER:/GUARD:CF"
                "LINKER:/DYNAMICBASE"
                "LINKER:/HIGHENTROPYVA"
                "LINKER:/NXCOMPAT"
            )
        elseif(WIN32)
            # MinGW GCC / GNU ld
            target_link_options(${TARGET_NAME} PRIVATE
                "LINKER:--fatal-warnings"
                "LINKER:--dynamicbase"
                "LINKER:--nxcompat"
                "LINKER:--high-entropy-va"
            )
        else()
            # Linux / POSIX ELF hardening
            target_compile_options(${TARGET_NAME} PRIVATE
                -fstack-protector-strong
                -fPIE
            )

            target_compile_definitions(${TARGET_NAME} PRIVATE
                _GLIBCXX_ASSERTIONS
                _FORTIFY_SOURCE=2
            )

            if(NOT APPLE)
                target_link_options(${TARGET_NAME} PRIVATE
                    "LINKER:--fatal-warnings"
                    "LINKER:-z,relro"
                    "LINKER:-z,now"
                    "LINKER:-z,noexecstack"
                    -pie
                )
            endif()
        endif()
    endif()
endfunction()
