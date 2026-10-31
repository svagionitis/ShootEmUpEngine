add_library(ShooterCore STATIC
    AnimationController.cpp
    AnimationController.h
    AudioCommandQueue.cpp
    BossController.cpp
    BossController.h
    CameraSystem.cpp
    CameraSystem.h
    CollisionSolver.cpp
    EnemyAiSystem.cpp
    EnemyAiSystem.h
    EntityPool.cpp
    GameEngine.cpp
    Hitbox.cpp
    LabyrinthMap.cpp
    LabyrinthMap.h
    Logger.cpp
    Logger.h
    MinimapRadar.cpp
    MinimapRadar.h
    NestManager.cpp
    NestManager.h
    ParticlePool.cpp
    ParticlePool.h
    RenderCommandQueue.cpp
    SecondaryArsenal.cpp
    SecondaryArsenal.h
    SecretManager.cpp
    SecretManager.h
    SecuritySystem.cpp
    SecuritySystem.h
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

