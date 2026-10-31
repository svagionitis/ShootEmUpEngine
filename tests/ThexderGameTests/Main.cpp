#include "TestHarness.h"

#include <cstdint>

#include <glog/logging.h>
#include <gtest/gtest.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>

TEST(ThexderGameTests, Level1Map)
{
    ThexderGame::Test::runLevel1MapTests();
}

TEST(ThexderGameTests, HudOverlay)
{
    ThexderGame::Test::runHudOverlayTests();
}

TEST(ThexderGameTests, EntitySpawner)
{
    ThexderGame::Test::runEntitySpawnerTests();
}

TEST(ThexderGameTests, GameState)
{
    ThexderGame::Test::runGameStateTests();
}

int main(int argc, char* argv[])
{
    SDL_SetMainReady();

    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = 1;

    (void)SDL_setenv("SDL_VIDEODRIVER", "dummy", 0);
    (void)SDL_setenv("SDL_AUDIODRIVER", "dummy", 0);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
