add_library(ShooterCore STATIC
    AnimationController.cpp
    AnimationController.h
    AudioCommandQueue.cpp
    CollisionSolver.cpp
    EntityPool.cpp
    GameEngine.cpp
    Hitbox.cpp
    Logger.cpp
    Logger.h
    RenderCommandQueue.cpp
    TileMap.cpp
    TransformSystem.cpp
    WeaponSystem.cpp
)

target_include_directories(ShooterCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ShooterCore PUBLIC
    glog::glog
)

apply_compiler_flags(ShooterCore)

