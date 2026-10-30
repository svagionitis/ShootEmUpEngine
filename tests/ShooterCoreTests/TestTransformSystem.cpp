#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include "Hitbox.h"
#include "TileMap.h"
#include "TransformSystem.h"
#include "Types.h"

namespace ShooterCore::Test {

static void testInitialState() noexcept
{
    const TransformSystem system { };
    TEST_ASSERT(system.mode() == TransformMode::Robot, "Initial mode must be Robot");
    TEST_ASSERT(!system.isTransforming(), "Initial system must not be transforming");
    TEST_ASSERT(!system.isWeaponLocked(), "Initial system weapons must not be locked");
    TEST_ASSERT(system.progress() >= 0.999f, "Initial system progress should be 1.0");
}

static void testDimensionsAndProjections() noexcept
{
    float rWidth { 0.0f };
    float rHeight { 0.0f };
    TransformSystem::getDims(TransformMode::Robot, rWidth, rHeight);
    TEST_ASSERT(rWidth == 16.0f, "Robot width must be 16 px");
    TEST_ASSERT(rHeight == 32.0f, "Robot height must be 32 px");

    float jWidth { 0.0f };
    float jHeight { 0.0f };
    TransformSystem::getDims(TransformMode::Jet, jWidth, jHeight);
    TEST_ASSERT(jWidth == 32.0f, "Jet width must be 32 px");
    TEST_ASSERT(jHeight == 16.0f, "Jet height must be 16 px");

    // Test projection from Jet (32x16) at (32, 16) to Robot (16x32) with BottomCenter anchor
    const WorldPos jetPos { 32.0f, 16.0f };
    const Hitbox robotBox { TransformSystem::projectBox(
        jetPos, TransformMode::Jet, TransformMode::Robot, AnchorMode::BottomCenter) };

    // Jet bottom is 16 + 16 = 32. Jet center X is 32 + 16 = 48.
    // Robot target: X in [48 - 8, 48 + 8] = [40, 56], Y in [32 - 32, 32] = [0, 32].
    TEST_ASSERT(robotBox.min().x == 40.0f, "BottomCenter Robot min.x must be 40");
    TEST_ASSERT(robotBox.min().y == 0.0f, "BottomCenter Robot min.y must be 0");
    TEST_ASSERT(robotBox.max().x == 56.0f, "BottomCenter Robot max.x must be 56");
    TEST_ASSERT(robotBox.max().y == 32.0f, "BottomCenter Robot max.y must be 32");

    // Test projection with TopLeft anchor
    const Hitbox topLeftBox { TransformSystem::projectBox(
        jetPos, TransformMode::Jet, TransformMode::Robot, AnchorMode::TopLeft) };
    TEST_ASSERT(topLeftBox.min().x == 32.0f, "TopLeft Robot min.x must be 32");
    TEST_ASSERT(topLeftBox.min().y == 16.0f, "TopLeft Robot min.y must be 16");
    TEST_ASSERT(topLeftBox.max().x == 48.0f, "TopLeft Robot max.x must be 48");
    TEST_ASSERT(topLeftBox.max().y == 48.0f, "TopLeft Robot max.y must be 48");
}

static void testOpenSpaceTransformation() noexcept
{
    TileMap map { 10U, 10U, 16.0f };
    TransformSystem system { 0.100f }; // 100 ms

    const WorldPos pos { 48.0f, 48.0f };
    const TransformResult res1 { system.requestTransform(pos, map, AnchorMode::BottomCenter) };
    TEST_ASSERT(res1 == TransformResult::Success, "Transform request in open space must succeed");
    TEST_ASSERT(system.mode() == TransformMode::TransformingToJet, "Mode must be TransformingToJet");
    TEST_ASSERT(system.isTransforming(), "System must report isTransforming");
    TEST_ASSERT(system.isWeaponLocked(), "Weapons must be locked during transformation");

    // Halfway update (50 ms)
    system.update(DeltaTime { 0.050f });
    TEST_ASSERT(system.mode() == TransformMode::TransformingToJet, "Mode must still be TransformingToJet at 50ms");
    TEST_ASSERT(system.progress() >= 0.49f && system.progress() <= 0.51f, "Progress should be 50%");
    TEST_ASSERT(system.isWeaponLocked(), "Weapons must still be locked at 50ms");

    // Trying to transform again while busy
    const TransformResult busyRes { system.requestTransform(pos, map, AnchorMode::BottomCenter) };
    TEST_ASSERT(busyRes == TransformResult::Busy, "Request during transition must return Busy");

    // Finish update (another 50 ms)
    system.update(DeltaTime { 0.050f });
    TEST_ASSERT(system.mode() == TransformMode::Jet, "Mode must transition to Jet upon timer completion");
    TEST_ASSERT(!system.isTransforming(), "System must not be transforming after completion");
    TEST_ASSERT(!system.isWeaponLocked(), "Weapons must unlock after transformation");
    TEST_ASSERT(system.progress() >= 0.999f, "Progress must be 1.0 after completion");
}

static void testHeadroomObstructionInTunnel() noexcept
{
    // 10x3 map:
    // Row 0: Solid (Y: 0..16)
    // Row 1: Empty (Y: 16..32, 16px high tunnel)
    // Row 2: Solid (Y: 32..48)
    TileMap map { 10U, 3U, 16.0f };
    for (std::uint32_t col { 0U }; col < 10U; ++col) {
        map.setTile(TileCoord { col, 0U }, TileType::SolidWall);
        map.setTile(TileCoord { col, 2U }, TileType::SolidWall);
    }

    TransformSystem system { 0.100f };
    system.forceState(TransformMode::Jet);
    TEST_ASSERT(system.mode() == TransformMode::Jet, "System must be forced to Jet");

    // Jet positioned in 1-tile tunnel: (x=32, y=16), height=16
    // Bottom edge is at y=32 (flush with floor row 2).
    const WorldPos jetPos { 32.0f, 16.0f };

    // Attempt transform to Robot with BottomCenter anchor
    // Projected Robot box reaches y: [0, 32], which overlaps Row 0 (solid ceiling)!
    const TransformResult result { system.requestTransform(jetPos, map, AnchorMode::BottomCenter) };
    TEST_ASSERT(result == TransformResult::RejectedObstruction,
        "Transformation to Robot in 1-tile tunnel must be rejected due to ceiling obstruction");
    TEST_ASSERT(system.mode() == TransformMode::Jet, "Mode must remain Jet when rejected");
    TEST_ASSERT(!system.isTransforming(), "System must not enter transition state when rejected");
}

static void testLateralObstructionForJet() noexcept
{
    // Narrow 1-tile vertical shaft:
    // Col 0: Solid
    // Col 1: Empty (16px wide)
    // Col 2: Solid
    TileMap map { 3U, 10U, 16.0f };
    for (std::uint32_t row { 0U }; row < 10U; ++row) {
        map.setTile(TileCoord { 0U, row }, TileType::SolidWall);
        map.setTile(TileCoord { 2U, row }, TileType::SolidWall);
    }

    TransformSystem system { 0.100f };
    // Robot in narrow shaft at (16, 32)
    const WorldPos robotPos { 16.0f, 32.0f };

    // Attempt transform to Jet (requires 32px width) with Center anchor
    const TransformResult result { system.requestTransform(robotPos, map, AnchorMode::Center) };
    TEST_ASSERT(result == TransformResult::RejectedObstruction,
        "Transformation to Jet in 1-tile shaft must be rejected due to side wall obstruction");
    TEST_ASSERT(system.mode() == TransformMode::Robot, "Mode must remain Robot when rejected");
}

static void testTransformSystemLifecycle() noexcept
{
    testInitialState();
    testDimensionsAndProjections();
    testOpenSpaceTransformation();
    testHeadroomObstructionInTunnel();
    testLateralObstructionForJet();
}

void runTransformTests() noexcept
{
    testTransformSystemLifecycle();
}

} // namespace ShooterCore::Test
