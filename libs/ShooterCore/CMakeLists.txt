add_library(ShooterCore STATIC
    Hitbox.cpp
)

target_include_directories(ShooterCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

apply_compiler_flags(ShooterCore)

