# tests/ThexderGameTests/CMakeLists.txt

add_executable(ThexderGameTests
    Main.cpp
    TestLevel1Map.cpp
    TestHudOverlay.cpp
    TestEntitySpawner.cpp
    TestGameState.cpp
    ${CMAKE_SOURCE_DIR}/apps/ThexderGame/Level1Map.h
    ${CMAKE_SOURCE_DIR}/apps/ThexderGame/Level1Map.cpp
    ${CMAKE_SOURCE_DIR}/apps/ThexderGame/HudOverlay.h
    ${CMAKE_SOURCE_DIR}/apps/ThexderGame/HudOverlay.cpp
    ${CMAKE_SOURCE_DIR}/apps/ThexderGame/EntitySpawner.h
    ${CMAKE_SOURCE_DIR}/apps/ThexderGame/EntitySpawner.cpp
    ${CMAKE_SOURCE_DIR}/apps/ThexderGame/ProceduralSprites.h
    ${CMAKE_SOURCE_DIR}/apps/ThexderGame/ProceduralSprites.cpp
    ${CMAKE_SOURCE_DIR}/apps/ThexderGame/GameState.h
    ${CMAKE_SOURCE_DIR}/apps/ThexderGame/GameState.cpp
)

target_include_directories(ThexderGameTests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/apps/ThexderGame
        ${CMAKE_SOURCE_DIR}/libs/ShooterCore
        ${CMAKE_SOURCE_DIR}/libs/SdlAdapter
)

target_link_libraries(ThexderGameTests
    PRIVATE
        ShooterCore
        SdlAdapter
        SDL2::SDL2
        gflags::gflags
        glog::glog
        GTest::gtest
        GTest::gtest_main
)

target_compile_options(ThexderGameTests PRIVATE ${PROJECT_WARNING_FLAGS})
target_compile_definitions(ThexderGameTests PRIVATE ${PROJECT_HARDENING_DEFINITIONS})

include(GoogleTest)
gtest_discover_tests(ThexderGameTests)

add_test(NAME ThexderGameAllTests COMMAND ThexderGameTests)
set_tests_properties(ThexderGameAllTests PROPERTIES LABELS "master;app")

