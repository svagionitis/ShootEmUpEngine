#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include <iostream>
#include <string>
#include <string_view>

#include <glog/logging.h>
#include <gtest/gtest.h>

TEST(ShooterCoreTests, Types)
{
    ShooterCore::Test::runTypeTests();
}

TEST(ShooterCoreTests, Hitbox)
{
    ShooterCore::Test::runHitboxTests();
}

TEST(ShooterCoreTests, TileMap)
{
    ShooterCore::Test::runTileMapTests();
}

TEST(ShooterCoreTests, CollisionSolver)
{
    ShooterCore::Test::runCollisionTests();
}

TEST(ShooterCoreTests, TransformSystem)
{
    ShooterCore::Test::runTransformTests();
}

TEST(ShooterCoreTests, WeaponSystem)
{
    ShooterCore::Test::runWeaponTests();
}

TEST(ShooterCoreTests, ShieldSystem)
{
    ShooterCore::Test::runShieldTests();
}

TEST(ShooterCoreTests, EngineInput)
{
    ShooterCore::Test::runEngineInputTests();
}

TEST(ShooterCoreTests, CommandQueues)
{
    ShooterCore::Test::runCommandQueueTests();
}

TEST(ShooterCoreTests, EntityPool)
{
    ShooterCore::Test::runEntityPoolTests();
}

TEST(ShooterCoreTests, GameEngine)
{
    ShooterCore::Test::runGameEngineTests();
}

TEST(ShooterCoreTests, Animation)
{
    ShooterCore::Test::runAnimationTests();
}

TEST(ShooterCoreTests, DeterminismSoak)
{
    ShooterCore::Test::runDeterminismSoakTests();
}

int main(int argc, char* argv[])
{
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = 1;

    LOG(INFO) << "Starting ShooterCore GoogleTest & GLog Suite...";
    testing::InitGoogleTest(&argc, argv);

    // Support --suite=<Name> mapping to GoogleTest filter
    for (int i { 1 }; i < argc; ++i) {
        const std::string_view arg { argv[i] };
        if (arg.rfind("--suite=", 0U) == 0U) {
            const std::string suiteFilter { std::string("ShooterCoreTests.") + std::string(arg.substr(8U)) };
            ::testing::GTEST_FLAG(filter) = suiteFilter;
            LOG(INFO) << "Applied suite filter: " << suiteFilter;
        }
    }

    return RUN_ALL_TESTS();
}
