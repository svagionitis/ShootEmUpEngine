#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include <array>

#include "Hitbox.h"
#include "TileMap.h"
#include "Types.h"
#include "WeaponSystem.h"

namespace ShooterCore::Test {

static void testShieldBarrierBounds() noexcept
{
    const Hitbox craftBox { WorldPos { 10.0f, 20.0f }, WorldPos { 26.0f, 52.0f } };
    const Hitbox shieldBox { WeaponSystem::computeShield(craftBox) };

    TEST_ASSERT(shieldBox.min().x == 6.0f, "Shield min.x must be expanded by 4 px");
    TEST_ASSERT(shieldBox.min().y == 16.0f, "Shield min.y must be expanded by 4 px");
    TEST_ASSERT(shieldBox.max().x == 30.0f, "Shield max.x must be expanded by 4 px");
    TEST_ASSERT(shieldBox.max().y == 56.0f, "Shield max.y must be expanded by 4 px");
}

static void testShieldSustainedDrain() noexcept
{
    TileMap map { 10U, 10U, 16.0f };
    WeaponSystem weapons { };
    EnergyPoints energy { 100U };
    const Hitbox craftBox { WorldPos { 16.0f, 16.0f }, WorldPos { 32.0f, 48.0f } };
    const std::array<TargetCandidate, WeaponSystem::kMaxTargets> targets { };

    const DeltaTime dt { 1.0f / 60.0f };

    // Advance 60 ticks (1.0 second) with shield engaged (15 pts/sec)
    for (std::uint32_t i { 0U }; i < 60U; ++i) {
        weapons.updateWeapons(
            dt, false, true, false, TransformMode::Robot, FacingDirection::Right, craftBox, map, targets, 0U, energy);
    }

    TEST_ASSERT(weapons.isShieldActive(), "Shield must remain active during sustained deployment");
    TEST_ASSERT(energy.value() == 85U, "Energy must drop from 100 to 85 after 1.0s shield deployment");
}

static void testShieldImpactAbsorption() noexcept
{
    TileMap map { 10U, 10U, 16.0f };
    WeaponSystem weapons { };
    EnergyPoints energy { 100U };
    const Hitbox craftBox { WorldPos { 16.0f, 16.0f }, WorldPos { 32.0f, 48.0f } };
    const std::array<TargetCandidate, WeaponSystem::kMaxTargets> targets { };

    const DeltaTime dt { 1.0f / 60.0f };
    weapons.updateWeapons(
        dt, false, true, false, TransformMode::Robot, FacingDirection::Right, craftBox, map, targets, 0U, energy);

    TEST_ASSERT(weapons.isShieldActive(), "Shield must be active");

    // Impact with 20 damage
    const bool absorbed { weapons.absorbImpact(DamageValue { 20U }, energy) };
    TEST_ASSERT(absorbed, "Active shield must absorb incoming hostile impact");
    TEST_ASSERT(energy.value() == 80U, "Energy must decrement by impact damage from 100 to 80");
    TEST_ASSERT(weapons.isShieldActive(), "Shield remains active if energy > 0");
}

static void testShieldImpactCollapse() noexcept
{
    TileMap map { 10U, 10U, 16.0f };
    WeaponSystem weapons { };
    EnergyPoints energy { 15U };
    const Hitbox craftBox { WorldPos { 16.0f, 16.0f }, WorldPos { 32.0f, 48.0f } };
    const std::array<TargetCandidate, WeaponSystem::kMaxTargets> targets { };

    const DeltaTime dt { 1.0f / 60.0f };
    weapons.updateWeapons(
        dt, false, true, false, TransformMode::Robot, FacingDirection::Right, craftBox, map, targets, 0U, energy);

    TEST_ASSERT(weapons.isShieldActive(), "Shield initially active with 15 energy");

    // Overwhelming impact of 30 damage
    const bool absorbed { weapons.absorbImpact(DamageValue { 30U }, energy) };
    TEST_ASSERT(absorbed, "Impact was absorbed by the shield barrier");
    TEST_ASSERT(energy.isDepleted(), "Energy must be completely exhausted");
    TEST_ASSERT(!weapons.isShieldActive(), "Shield must immediately collapse upon energy exhaustion");
}

static void testShieldInactivePenetration() noexcept
{
    WeaponSystem weapons { };
    EnergyPoints energy { 100U };

    TEST_ASSERT(!weapons.isShieldActive(), "Shield is initially inactive");

    // Impact against inactive shield
    const bool absorbed { weapons.absorbImpact(DamageValue { 10U }, energy) };
    TEST_ASSERT(!absorbed, "Inactive shield must not absorb impact (hull takes hit)");
    TEST_ASSERT(energy.value() == 100U, "Energy is unconsumed by shield if shield is inactive");
}

static void testDualLaserAndShieldDrain() noexcept
{
    TileMap map { 10U, 10U, 16.0f };
    WeaponSystem weapons { };
    EnergyPoints energy { 100U };
    const Hitbox craftBox { WorldPos { 16.0f, 16.0f }, WorldPos { 32.0f, 48.0f } };
    const std::array<TargetCandidate, WeaponSystem::kMaxTargets> targets { };

    const DeltaTime dt { 1.0f / 60.0f };

    // Advance 60 ticks (1.0 second) with both laser (10 pts/s) and shield (15 pts/s)
    for (std::uint32_t i { 0U }; i < 60U; ++i) {
        weapons.updateWeapons(
            dt, true, true, false, TransformMode::Robot, FacingDirection::Right, craftBox, map, targets, 0U, energy);
    }

    TEST_ASSERT(weapons.isLaserActive(), "Laser must be active during dual deployment");
    TEST_ASSERT(weapons.isShieldActive(), "Shield must be active during dual deployment");
    TEST_ASSERT(energy.value() == 75U, "Combined drain must reduce energy from 100 to 75");
}

void runShieldTests() noexcept
{
    testShieldBarrierBounds();
    testShieldSustainedDrain();
    testShieldImpactAbsorption();
    testShieldImpactCollapse();
    testShieldInactivePenetration();
    testDualLaserAndShieldDrain();
}

} // namespace ShooterCore::Test
