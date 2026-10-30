#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include <array>

#include "Hitbox.h"
#include "TileMap.h"
#include "Types.h"
#include "WeaponSystem.h"

namespace ShooterCore::Test {

static void testDrainAccumulator() noexcept
{
    DrainAccumulator accum { };
    TEST_ASSERT(accum.accrued() == 0.0f, "DrainAccumulator accrued must start at 0");
    TEST_ASSERT(accum.consumeWhole() == 0U, "Initial consumeWhole must return 0");

    // Accumulate 10 points/sec for 0.05 sec = 0.5 points
    accum.accumulate(10.0f, 0.05f);
    TEST_ASSERT(accum.consumeWhole() == 0U, "Consuming < 1.0 must return 0");
    TEST_ASSERT(accum.accrued() >= 0.499f && accum.accrued() <= 0.501f, "Accrued must be ~0.5");

    // Accumulate another 0.06 sec = 0.6 points (total 1.1 points)
    accum.accumulate(10.0f, 0.06f);
    TEST_ASSERT(accum.consumeWhole() == 1U, "Consuming >= 1.0 must extract 1");
    TEST_ASSERT(accum.accrued() >= 0.099f && accum.accrued() <= 0.101f, "Remainder must be ~0.1");

    accum.reset();
    TEST_ASSERT(accum.accrued() == 0.0f, "Reset must zero the accumulator");
}

static void testLaserEnergyDrain() noexcept
{
    TileMap map { 20U, 20U, 16.0f };
    WeaponSystem weapons { };
    EnergyPoints energy { 100U };
    const Hitbox craftBox { WorldPos { 32.0f, 32.0f }, WorldPos { 48.0f, 64.0f } };
    const std::array<TargetCandidate, WeaponSystem::kMaxTargets> targets { };

    const DeltaTime dt { 1.0f / 60.0f };
    for (std::uint32_t i { 0U }; i < 60U; ++i) {
        weapons.updateWeapons(
            dt, true, false, false, TransformMode::Robot, FacingDirection::Right, craftBox, map, targets, 0U, energy);
    }

    TEST_ASSERT(weapons.isLaserActive(), "Laser must be active during sustained fire");
    TEST_ASSERT(energy.value() == 90U, "Energy must drop from 100 to 90 after 1s laser fire");

    // Advance until energy is completely depleted
    for (std::uint32_t i { 0U }; i < 600U; ++i) {
        weapons.updateWeapons(
            dt, true, false, false, TransformMode::Robot, FacingDirection::Right, craftBox, map, targets, 0U, energy);
    }

    TEST_ASSERT(energy.isDepleted(), "Energy must eventually reach 0");
    TEST_ASSERT(!weapons.isLaserActive(), "Laser must cease firing when energy is depleted");
    TEST_ASSERT(!weapons.getBeamSegment().active, "Beam must be inactive when energy is depleted");
}

static void testLaserLineOfSight() noexcept
{
    TileMap map { 20U, 20U, 16.0f };
    WeaponSystem weapons { };
    EnergyPoints energy { 100U };
    const Hitbox craftBox { WorldPos { 32.0f, 32.0f }, WorldPos { 48.0f, 64.0f } };

    // Place an active enemy at (100, 40)
    std::array<TargetCandidate, WeaponSystem::kMaxTargets> targets { };
    targets[0].id = 101U;
    targets[0].bounds = Hitbox { WorldPos { 96.0f, 40.0f }, WorldPos { 112.0f, 56.0f } };
    targets[0].type = TargetType::Hostile;
    targets[0].active = true;

    const DeltaTime dt { 1.0f / 60.0f };
    weapons.updateWeapons(
        dt, true, false, false, TransformMode::Robot, FacingDirection::Right, craftBox, map, targets, 1U, energy);

    TEST_ASSERT(weapons.isLaserActive(), "Laser must fire with clear line-of-sight");
    const BeamSegment beamClear { weapons.getBeamSegment() };
    TEST_ASSERT(beamClear.active, "Beam must be active");
    TEST_ASSERT(beamClear.lockedToTarget, "Beam must lock onto target in open space");
    TEST_ASSERT(beamClear.targetId == 101U, "Locked target ID must match candidate");
    TEST_ASSERT(beamClear.endpoint.x == targets[0].center().x, "Beam endpoint X must match target center");
    TEST_ASSERT(beamClear.endpoint.y == targets[0].center().y, "Beam endpoint Y must match target center");

    // Insert a solid wall at column 4, row 2-3 (between craft and enemy)
    map.setTile(TileCoord { 4U, 2U }, TileType::SolidWall);
    map.setTile(TileCoord { 4U, 3U }, TileType::SolidWall);

    weapons.updateWeapons(
        dt, true, false, false, TransformMode::Robot, FacingDirection::Right, craftBox, map, targets, 1U, energy);

    const BeamSegment beamBlocked { weapons.getBeamSegment() };
    TEST_ASSERT(beamBlocked.active, "Beam is still emitted");
    TEST_ASSERT(!beamBlocked.lockedToTarget, "Beam must not lock onto obscured enemy");
    TEST_ASSERT(beamBlocked.targetId == 0U, "Locked target ID must be 0 when blocked");
    TEST_ASSERT(beamBlocked.endpoint.x < targets[0].center().x, "Beam must terminate at solid wall");
}

static void testTargetPrioritization() noexcept
{
    TileMap map { 25U, 25U, 16.0f };
    WeaponSystem weapons { };
    EnergyPoints energy { 100U };
    const Hitbox craftBox { WorldPos { 32.0f, 32.0f }, WorldPos { 48.0f, 64.0f } };

    // Candidate 0: Distant hostile at X=200
    // Candidate 1: Close hostile at X=90
    std::array<TargetCandidate, WeaponSystem::kMaxTargets> targets { };
    targets[0].id = 1U;
    targets[0].bounds = Hitbox { WorldPos { 200.0f, 40.0f }, WorldPos { 216.0f, 56.0f } };
    targets[0].type = TargetType::Hostile;
    targets[0].active = true;

    targets[1].id = 2U;
    targets[1].bounds = Hitbox { WorldPos { 90.0f, 40.0f }, WorldPos { 106.0f, 56.0f } };
    targets[1].type = TargetType::Hostile;
    targets[1].active = true;

    const DeltaTime dt { 1.0f / 60.0f };
    weapons.updateWeapons(
        dt, true, false, false, TransformMode::Robot, FacingDirection::Right, craftBox, map, targets, 2U, energy);

    const BeamSegment beam { weapons.getBeamSegment() };
    TEST_ASSERT(beam.lockedToTarget, "Beam must lock onto a target");
    TEST_ASSERT(beam.targetId == 2U, "Closest enemy must be prioritized over distant enemy");

    // Now test hostile threat priority over closer destructible block
    // Candidate 0: Hostile at X=110
    // Candidate 1: Destructible at X=90
    targets[0].id = 10U;
    targets[0].bounds = Hitbox { WorldPos { 110.0f, 40.0f }, WorldPos { 126.0f, 56.0f } };
    targets[0].type = TargetType::Hostile;

    targets[1].id = 20U;
    targets[1].bounds = Hitbox { WorldPos { 90.0f, 40.0f }, WorldPos { 106.0f, 56.0f } };
    targets[1].type = TargetType::Destructible;

    weapons.updateWeapons(
        dt, true, false, false, TransformMode::Robot, FacingDirection::Right, craftBox, map, targets, 2U, energy);

    const BeamSegment beamThreat { weapons.getBeamSegment() };
    TEST_ASSERT(beamThreat.targetId == 10U, "Hostile threat bonus must prioritize enemy over destructible block");
}

static void testModeTargetingConstraints() noexcept
{
    TileMap map { 20U, 20U, 16.0f };
    WeaponSystem weapons { };
    EnergyPoints energy { 100U };
    const Hitbox craftBox { WorldPos { 32.0f, 80.0f }, WorldPos { 48.0f, 112.0f } };

    // Target at steep forward-up angle (~60 degrees from horizontal)
    // Craft muzzle is at ~X=48, Y=96. Target at X=70, Y=58 (dx=22, dy=-38, dist=44, cos~0.50)
    std::array<TargetCandidate, WeaponSystem::kMaxTargets> targets { };
    targets[0].id = 7U;
    targets[0].bounds = Hitbox { WorldPos { 64.0f, 50.0f }, WorldPos { 76.0f, 66.0f } };
    targets[0].type = TargetType::Hostile;
    targets[0].active = true;

    const DeltaTime dt { 1.0f / 60.0f };

    // In Robot mode (cone threshold cos = 0.2588 / 75 deg), target is inside cone
    weapons.updateWeapons(
        dt, true, false, false, TransformMode::Robot, FacingDirection::Right, craftBox, map, targets, 1U, energy);

    TEST_ASSERT(weapons.getBeamSegment().lockedToTarget, "Robot wide cone must acquire steep-angled target");
    TEST_ASSERT(weapons.getBeamSegment().targetId == 7U, "Robot must lock candidate 7");

    // In Jet mode (cone threshold cos = 0.7071 / 45 deg), target is outside cone
    weapons.updateWeapons(
        dt, true, false, false, TransformMode::Jet, FacingDirection::Right, craftBox, map, targets, 1U, energy);

    TEST_ASSERT(!weapons.getBeamSegment().lockedToTarget, "Jet narrow cone must reject steep-angled target");

    // Transformation lockout: if isTransforming == true, weapons are locked
    weapons.updateWeapons(
        dt, true, false, true, TransformMode::Robot, FacingDirection::Right, craftBox, map, targets, 1U, energy);

    TEST_ASSERT(!weapons.isLaserActive(), "Weapon must not fire during transformation");
    TEST_ASSERT(!weapons.getBeamSegment().active, "Beam must not emit during transformation");
}

static void testDestructibleTileErosion() noexcept
{
    TileMap map { 15U, 15U, 16.0f };
    WeaponSystem weapons { };
    EnergyPoints energy { 100U };
    const Hitbox craftBox { WorldPos { 32.0f, 32.0f }, WorldPos { 48.0f, 64.0f } };

    // Place destructible tile directly in line of fire at column 5, row 3 (X=80, Y=48)
    const TileCoord blockCoord { 5U, 3U };
    map.setTile(blockCoord, TileType::DestructibleWall);
    map.setHealth(blockCoord, TileHealth { 3U });

    const std::array<TargetCandidate, WeaponSystem::kMaxTargets> targets { };
    const DeltaTime dt { 1.0f / 60.0f };

    // Fire for 0.25 seconds (15 ticks at 5 hits/sec = 1 hit delivered)
    for (std::uint32_t i { 0U }; i < 15U; ++i) {
        weapons.updateWeapons(
            dt, true, false, false, TransformMode::Robot, FacingDirection::Right, craftBox, map, targets, 0U, energy);
    }

    TEST_ASSERT(map.getHealth(blockCoord).value() == 2U, "Destructible block durability must reduce to 2");

    // Fire for another 0.45 seconds (total > 0.6s at 5 hits/sec = 3 hits total)
    for (std::uint32_t i { 0U }; i < 30U; ++i) {
        weapons.updateWeapons(
            dt, true, false, false, TransformMode::Robot, FacingDirection::Right, craftBox, map, targets, 0U, energy);
    }

    TEST_ASSERT(map.getHealth(blockCoord).isDestroyed(), "Destructible block must be destroyed by continuous fire");
}

void runWeaponTests() noexcept
{
    testDrainAccumulator();
    testLaserEnergyDrain();
    testLaserLineOfSight();
    testTargetPrioritization();
    testModeTargetingConstraints();
    testDestructibleTileErosion();
}

} // namespace ShooterCore::Test
