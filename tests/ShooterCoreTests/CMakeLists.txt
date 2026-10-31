add_executable(ShooterCoreTests
    Main.cpp
    TestAnimationController.cpp
    TestCollisionSolver.cpp
    TestCommandQueues.cpp
    TestDeterminismSoak.cpp
    TestEngineInput.cpp
    TestEntityPool.cpp
    TestGameEngine.cpp
    TestHitbox.cpp
    TestParticlePool.cpp
    TestShieldSystem.cpp
    TestTileMap.cpp
    TestTransformSystem.cpp
    TestTypes.cpp
    TestWeaponSystem.cpp
)

target_link_libraries(ShooterCoreTests PRIVATE
    ShooterCore
    GTest::gtest
    glog::glog
)

copy_executable_dependencies(ShooterCoreTests)

target_include_directories(ShooterCoreTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/libs/ShooterCore
)

apply_compiler_flags(ShooterCoreTests)

# Register Master Test Run
add_test(NAME ShooterCoreAllTests COMMAND ShooterCoreTests)
set_tests_properties(ShooterCoreAllTests PROPERTIES LABELS "master;all" TIMEOUT 30)

# Register Granular Modular Suites for CTest
macro(register_shooter_test TEST_NAME SUITE_NAME LABELS_STR TIMEOUT_SEC)
    add_test(NAME ${TEST_NAME} COMMAND ShooterCoreTests --suite=${SUITE_NAME})
    set_tests_properties(${TEST_NAME} PROPERTIES LABELS "${LABELS_STR}" TIMEOUT ${TIMEOUT_SEC})
endmacro()

register_shooter_test(ShooterCore_Types            Types            "unit;domain"        5)
register_shooter_test(ShooterCore_Hitbox           Hitbox           "unit;geometry"      5)
register_shooter_test(ShooterCore_TileMap          TileMap          "unit;spatial"       5)
register_shooter_test(ShooterCore_CollisionSolver  CollisionSolver  "physics;ccd"        10)
register_shooter_test(ShooterCore_TransformSystem  TransformSystem  "unit;fsm"           5)
register_shooter_test(ShooterCore_WeaponSystem     WeaponSystem     "combat;weapons"     10)
register_shooter_test(ShooterCore_ShieldSystem     ShieldSystem     "combat;shield"      5)
register_shooter_test(ShooterCore_EngineInput      EngineInput      "unit;input"         5)
register_shooter_test(ShooterCore_CommandQueues    CommandQueues    "queues;memory"      5)
register_shooter_test(ShooterCore_EntityPool       EntityPool       "memory;lifecycle"   10)
register_shooter_test(ShooterCore_ParticlePool     ParticlePool     "combat;particles"   5)
register_shooter_test(ShooterCore_GameEngine       GameEngine       "integration;engine" 15)
register_shooter_test(ShooterCore_Animation        Animation        "unit;animation"     5)
register_shooter_test(ShooterCore_DeterminismSoak  DeterminismSoak  "soak;determinism"   60)
