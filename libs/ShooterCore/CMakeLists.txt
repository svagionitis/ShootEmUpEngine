add_library(ShooterCore STATIC
    CollisionSolver.cpp
    Hitbox.cpp
    TileMap.cpp
)

target_include_directories(ShooterCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

apply_compiler_flags(ShooterCore)

