#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include <iostream>

int main()
{
    std::cout << "[INFO] Starting ShooterCore Automated Tests...\n";

    ShooterCore::Test::runTypeTests();
    ShooterCore::Test::runHitboxTests();
    ShooterCore::Test::runTileMapTests();
    ShooterCore::Test::runCollisionTests();
    ShooterCore::Test::runTransformTests();

    if (ShooterCore::Test::gFailureCount == 0U) {
        std::cout << "[PASS] All ShooterCore tests passed successfully.\n";
        return 0;
    }

    std::cerr << "[FAIL] Total failures encountered: " << ShooterCore::Test::gFailureCount << "\n";
    return 1;
}
