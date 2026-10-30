#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include <chrono>
#include <iostream>
#include <string_view>

namespace {

struct SuiteEntry {
    std::string_view name;
    void (*runner)() noexcept;
    bool isSoak;
};

const SuiteEntry kSuites[] = { { "Types", &ShooterCore::Test::runTypeTests, false },
    { "Hitbox", &ShooterCore::Test::runHitboxTests, false }, { "TileMap", &ShooterCore::Test::runTileMapTests, false },
    { "CollisionSolver", &ShooterCore::Test::runCollisionTests, false },
    { "TransformSystem", &ShooterCore::Test::runTransformTests, false },
    { "WeaponSystem", &ShooterCore::Test::runWeaponTests, false },
    { "ShieldSystem", &ShooterCore::Test::runShieldTests, false },
    { "EngineInput", &ShooterCore::Test::runEngineInputTests, false },
    { "CommandQueues", &ShooterCore::Test::runCommandQueueTests, false },
    { "EntityPool", &ShooterCore::Test::runEntityPoolTests, false },
    { "GameEngine", &ShooterCore::Test::runGameEngineTests, false },
    { "DeterminismSoak", &ShooterCore::Test::runDeterminismSoakTests, true } };

} // namespace

int main(int argc, char* argv[])
{
    std::string_view targetSuite { };
    bool runSoak { false };

    for (int i { 1 }; i < argc; ++i) {
        const std::string_view arg { argv[i] };
        if (arg == "--list") {
            std::cout << "Available ShooterCore test suites:\n";
            for (const auto& entry : kSuites) {
                std::cout << "  - " << entry.name << (entry.isSoak ? " (soak)" : "") << "\n";
            }
            return 0;
        }
        if (arg == "--soak") {
            runSoak = true;
        } else if (arg.rfind("--suite=", 0U) == 0U) {
            targetSuite = arg.substr(8U);
        }
    }

    std::cout << "[INFO] Starting ShooterCore Automated Headless Tests...\n";
    const auto startTime = std::chrono::high_resolution_clock::now();

    for (const auto& entry : kSuites) {
        if (!targetSuite.empty() && entry.name != targetSuite) {
            continue;
        }
        if (entry.isSoak && !runSoak && targetSuite != entry.name) {
            continue;
        }

        std::cout << "[RUN ] Suite: " << entry.name << "\n";
        const std::uint32_t failBefore = ShooterCore::Test::gTestStats.failedAssertions;

        entry.runner();

        if (ShooterCore::Test::gTestStats.failedAssertions == failBefore) {
            std::cout << "[OK  ] Suite: " << entry.name << " passed.\n";
            ++ShooterCore::Test::gTestStats.passedSuites;
        } else {
            std::cerr << "[FAIL] Suite: " << entry.name << " had failures!\n";
            ++ShooterCore::Test::gTestStats.failedSuites;
        }
    }

    const auto endTime = std::chrono::high_resolution_clock::now();
    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    std::cout << "\n======================================================\n";
    std::cout << "Test Summary: " << ShooterCore::Test::gTestStats.passedSuites << " suites passed, "
              << ShooterCore::Test::gTestStats.failedSuites << " suites failed. (" << elapsedMs << " ms total)\n";
    std::cout << "Assertions:   "
              << (ShooterCore::Test::gTestStats.totalAssertions - ShooterCore::Test::gTestStats.failedAssertions)
              << " passed, " << ShooterCore::Test::gTestStats.failedAssertions << " failed.\n";
    std::cout << "======================================================\n";

    return (ShooterCore::Test::gTestStats.failedAssertions == 0U) ? 0 : 1;
}
