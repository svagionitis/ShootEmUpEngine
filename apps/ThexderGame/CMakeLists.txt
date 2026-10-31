# apps/ThexderGame/CMakeLists.txt

add_executable(ThexderGame
    Main.cpp
    GameState.h
    GameState.cpp
    Level1Map.h
    Level1Map.cpp
    HudOverlay.h
    HudOverlay.cpp
    EntitySpawner.h
    EntitySpawner.cpp
    ProceduralSprites.h
    ProceduralSprites.cpp
)

target_include_directories(ThexderGame
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/libs/ShooterCore
        ${CMAKE_SOURCE_DIR}/libs/SdlAdapter
)

target_link_libraries(ThexderGame
    PRIVATE
        ShooterCore
        SdlAdapter
        SDL2::SDL2
        gflags::gflags
        glog::glog
)

target_compile_options(ThexderGame PRIVATE ${PROJECT_WARNING_FLAGS})
target_compile_definitions(ThexderGame PRIVATE ${PROJECT_HARDENING_DEFINITIONS})

