#include "TestHarness.h"

#include <cstdint>

#include "CameraSystem.h"
#include "Types.h"

namespace ShooterCore::Test {

void runCameraSystemTests() noexcept
{
    CameraSystem camera { };
    const SectorBounds sector { WorldPos { 0.0f, 0.0f }, WorldPos { 1000.0f, 1000.0f } };

    // 1. Initial State
    camera.setPosition(WorldPos { 100.0f, 100.0f });
    TEST_ASSERT_NEAR(camera.viewport().pos.x, 100.0f, 0.01f, "Initial camera X");
    TEST_ASSERT_NEAR(camera.viewport().pos.y, 100.0f, 0.01f, "Initial camera Y");
    TEST_ASSERT(!camera.isLocked(), "Camera must not be locked initially");

    // 2. Deadzone Idle: Viewport center is at (260, 220). Deadzone is [260-32, 260+32] = [228, 292] x [220-24, 220+24]
    // = [196, 244].
    const WorldPos playerInsideDeadzone { 260.0f, 220.0f };
    camera.update(DeltaTime { 0.1f }, playerInsideDeadzone, sector);
    TEST_ASSERT_NEAR(camera.viewport().pos.x, 100.0f, 0.01f, "Camera must not move when player is centered");
    TEST_ASSERT_NEAR(camera.viewport().pos.y, 100.0f, 0.01f, "Camera must not move when player is centered");

    // 3. Move player outside deadzone right margin (> 292)
    const WorldPos playerRight { 350.0f, 220.0f };
    camera.update(DeltaTime { 0.1f }, playerRight, sector);
    TEST_ASSERT(camera.viewport().pos.x > 100.0f, "Camera must scroll right when player exceeds right deadzone");

    // 4. Sector Clamping to Min Bounds (0, 0)
    const WorldPos playerFarLeft { -50.0f, -50.0f };
    for (int i { 0 }; i < 30; ++i) {
        camera.update(DeltaTime { 0.1f }, playerFarLeft, sector);
    }
    TEST_ASSERT_NEAR(camera.viewport().pos.x, 0.0f, 0.01f, "Camera X must clamp to sector min (0)");
    TEST_ASSERT_NEAR(camera.viewport().pos.y, 0.0f, 0.01f, "Camera Y must clamp to sector min (0)");

    // 5. Sector Clamping to Max Bounds (1000 - 320 = 680, 1000 - 240 = 760)
    const WorldPos playerFarRight { 2000.0f, 2000.0f };
    for (int i { 0 }; i < 30; ++i) {
        camera.update(DeltaTime { 0.1f }, playerFarRight, sector);
    }
    TEST_ASSERT_NEAR(camera.viewport().pos.x, 680.0f, 0.01f, "Camera X must clamp to sector max (680)");
    TEST_ASSERT_NEAR(camera.viewport().pos.y, 760.0f, 0.01f, "Camera Y must clamp to sector max (760)");

    // 6. Arena Lock
    const SectorBounds arenaBounds { WorldPos { 3000.0f, 1000.0f }, WorldPos { 3400.0f, 1300.0f } };
    camera.lockArena(arenaBounds);
    TEST_ASSERT(camera.isLocked(), "Camera must report isLocked");

    // Clamps immediately to arena
    camera.update(DeltaTime { 0.1f }, WorldPos { 0.0f, 0.0f }, sector);
    TEST_ASSERT(camera.viewport().pos.x >= 3000.0f, "Locked camera must stay in arena bounds");
    TEST_ASSERT(camera.viewport().pos.y >= 1000.0f, "Locked camera must stay in arena bounds");

    camera.unlockArena();
    TEST_ASSERT(!camera.isLocked(), "Camera must be unlocked");

    // 7. Trauma & Screen Shake System
    TEST_ASSERT(!camera.trauma().isActive(), "Initial trauma must be inactive");
    TEST_ASSERT_NEAR(camera.trauma().value(), 0.0f, 0.001f, "Initial trauma value must be 0");
    TEST_ASSERT_NEAR(camera.trauma().magnitude(), 0.0f, 0.001f, "Initial trauma magnitude must be 0");

    ShakeOffset zeroShake { camera.computeShakeOffset() };
    TEST_ASSERT_NEAR(zeroShake.x, 0.0f, 0.001f, "Zero trauma yields zero shake X");
    TEST_ASSERT_NEAR(zeroShake.y, 0.0f, 0.001f, "Zero trauma yields zero shake Y");

    // Add trauma
    camera.addTrauma(0.5f);
    TEST_ASSERT(camera.trauma().isActive(), "Trauma must be active after adding 0.5");
    TEST_ASSERT_NEAR(camera.trauma().value(), 0.5f, 0.001f, "Trauma must be 0.5");
    TEST_ASSERT_NEAR(camera.trauma().magnitude(), 0.25f, 0.001f, "Trauma magnitude must be 0.25 (0.5^2)");

    // Clamping to 1.0
    camera.addTrauma(0.8f);
    TEST_ASSERT_NEAR(camera.trauma().value(), 1.0f, 0.001f, "Trauma must clamp at 1.0");
    TEST_ASSERT_NEAR(camera.trauma().magnitude(), 1.0f, 0.001f, "Max trauma magnitude must be 1.0");

    // Advance time and check shake offset displacement
    camera.updateTrauma(DeltaTime { 0.1f });
    ShakeOffset activeShake { camera.computeShakeOffset() };
    TEST_ASSERT(std::abs(activeShake.x) <= CameraSystem::kMaxShakeX, "Shake X must be bounded by kMaxShakeX");
    TEST_ASSERT(std::abs(activeShake.y) <= CameraSystem::kMaxShakeY, "Shake Y must be bounded by kMaxShakeY");

    // Decay test: decaying with 1.5/sec for 1.0 second should reduce trauma to 0
    camera.updateTrauma(DeltaTime { 1.0f });
    TEST_ASSERT(!camera.trauma().isActive(), "Trauma must decay to zero after 1s");
    TEST_ASSERT_NEAR(camera.trauma().value(), 0.0f, 0.001f, "Decayed trauma value must be 0");
}

} // namespace ShooterCore::Test
