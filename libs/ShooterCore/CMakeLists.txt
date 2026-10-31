add_library(ShooterCore STATIC
    AnimationController.cpp
    AnimationController.h
    AudioCommandQueue.cpp
    CollisionSolver.cpp
    EnemyAiSystem.cpp
    EnemyAiSystem.h
    EntityPool.cpp
    GameEngine.cpp
    Hitbox.cpp
    Logger.cpp
    Logger.h
    NestManager.cpp
    NestManager.h
    ParticlePool.cpp
    ParticlePool.h
    RenderCommandQueue.cpp
    SecondaryArsenal.cpp
    SecondaryArsenal.h
    SupplyDropSystem.cpp
    SupplyDropSystem.h
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

