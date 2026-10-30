add_library(ShooterCore STATIC
    AudioCommandQueue.cpp
    CollisionSolver.cpp
    EntityPool.cpp
    GameEngine.cpp
    Hitbox.cpp
    RenderCommandQueue.cpp
    TileMap.cpp
    TransformSystem.cpp
    WeaponSystem.cpp
)

target_include_directories(ShooterCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

apply_compiler_flags(ShooterCore)

