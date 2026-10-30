add_executable(ShooterCoreTests
    Main.cpp
    TestCollisionSolver.cpp
    TestHitbox.cpp
    TestShieldSystem.cpp
    TestTileMap.cpp
    TestTransformSystem.cpp
    TestTypes.cpp
    TestWeaponSystem.cpp
)

target_link_libraries(ShooterCoreTests PRIVATE
    ShooterCore
)

target_include_directories(ShooterCoreTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/libs/ShooterCore
)

apply_compiler_flags(ShooterCoreTests)

add_test(NAME ShooterCoreAllTests COMMAND ShooterCoreTests)

