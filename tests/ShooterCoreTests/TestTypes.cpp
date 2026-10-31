#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include "Types.h"

namespace ShooterCore::Test {

void runTypeTests() noexcept
{
    // 1. EnergyPoints default and depletion
    const EnergyPoints emptyEnergy { };
    TEST_ASSERT(emptyEnergy.value() == 0U, "Default energy must be 0");
    TEST_ASSERT(emptyEnergy.isDepleted(), "Empty energy must report isDepleted");

    // 2. Clamping upon construction
    const EnergyPoints clampedEnergy { 250U };
    TEST_ASSERT(clampedEnergy.value() == 100U, "Energy must clamp to default max of 100");
    TEST_ASSERT(!clampedEnergy.isDepleted(), "100 energy must not be depleted");

    // 3. Sufficient check
    TEST_ASSERT(clampedEnergy.hasSufficient(EnergyPoints { 50U }), "100 energy must be sufficient for 50");
    TEST_ASSERT(!emptyEnergy.hasSufficient(EnergyPoints { 1U }), "0 energy must not be sufficient for 1");

    // 4. Deduction and underflow safety
    EnergyPoints mutEnergy { 30U };
    mutEnergy.deduct(EnergyPoints { 10U });
    TEST_ASSERT(mutEnergy.value() == 20U, "30 - 10 must equal 20 energy");

    mutEnergy.deduct(EnergyPoints { 50U });
    TEST_ASSERT(mutEnergy.value() == 0U, "Deducting more than available must clamp to 0 without underflow");
    TEST_ASSERT(mutEnergy.isDepleted(), "Energy must be depleted after large deduction");

    // 5. Replenish and overflow safety
    mutEnergy.replenish(EnergyPoints { 40U });
    TEST_ASSERT(mutEnergy.value() == 40U, "0 + 40 replenish must equal 40");

    mutEnergy.replenish(EnergyPoints { 80U });
    TEST_ASSERT(mutEnergy.value() == 100U, "Replenish beyond 100 must clamp to 100");

    // 6. ShieldGauge invariants
    const ShieldGauge defShield { };
    TEST_ASSERT(defShield.percentage() == 0U, "Default shield must be 0%");
    TEST_ASSERT(!defShield.isActive(), "0% shield must be inactive");

    const ShieldGauge activeShield { 75U };
    TEST_ASSERT(activeShield.percentage() == 75U, "Shield percentage must match input");
    TEST_ASSERT(activeShield.isActive(), "75% shield must be active");

    const ShieldGauge clampedShield { 150U };
    TEST_ASSERT(clampedShield.percentage() == 100U, "Shield must clamp to 100%");

    // 7. WorldPos to TileCoord mapping
    const WorldPos posA { 32.0f, 48.0f };
    const TileCoord tileA { posA.toTileCoord(16.0f) };
    TEST_ASSERT(tileA.col == 2U, "32px at 16px/tile must map to col 2");
    TEST_ASSERT(tileA.row == 3U, "48px at 16px/tile must map to row 3");

    const WorldPos negPos { -10.0f, -5.0f };
    const TileCoord safeNegTile { negPos.toTileCoord(16.0f) };
    TEST_ASSERT(safeNegTile.col == 0U, "Negative coordinate must clamp to tile col 0");
    TEST_ASSERT(safeNegTile.row == 0U, "Negative coordinate must clamp to tile row 0");

    const TileCoord zeroTileSize { posA.toTileCoord(0.0f) };
    TEST_ASSERT(zeroTileSize.col == 0U && zeroTileSize.row == 0U, "Zero tile size must safely return (0, 0)");

    // SecondaryAmmo Tests
    SecondaryAmmo ammo { 10U, 24U };
    TEST_ASSERT(ammo.count() == 10U, "Initial ammo count must be 10");
    TEST_ASSERT(ammo.maxCapacity() == 24U, "Max ammo capacity must be 24");
    TEST_ASSERT(ammo.hasAmmo(), "Ammo must be available");
    TEST_ASSERT(!ammo.isFull(), "Ammo must not be full");

    ammo.consume();
    TEST_ASSERT(ammo.count() == 9U, "Ammo must decrement after consume");

    ammo.replenish(20U);
    TEST_ASSERT(ammo.count() == 24U, "Ammo must clamp to maxCapacity");
    TEST_ASSERT(ammo.isFull(), "Ammo must be full");

    SecondaryAmmo emptyAmmo { 0U, 10U };
    TEST_ASSERT(!emptyAmmo.hasAmmo(), "Empty ammo hasAmmo must be false");
    emptyAmmo.consume();
    TEST_ASSERT(emptyAmmo.count() == 0U, "Empty ammo consume must not underflow");

    // Phase 12: SectorBounds tests
    const SectorBounds sector { { 0.0f, 0.0f }, { 100.0f, 100.0f } };
    TEST_ASSERT(sector.contains(WorldPos { 50.0f, 50.0f }), "Sector must contain (50, 50)");
    TEST_ASSERT(sector.contains(WorldPos { 0.0f, 0.0f }), "Sector must contain min corner");
    TEST_ASSERT(sector.contains(WorldPos { 100.0f, 100.0f }), "Sector must contain max corner");
    TEST_ASSERT(!sector.contains(WorldPos { 101.0f, 50.0f }), "Sector must not contain point outside x");
    TEST_ASSERT(!sector.contains(WorldPos { 50.0f, -1.0f }), "Sector must not contain point outside y");

    // Phase 12: GeneratorHealth tests
    GeneratorHealth genHp { 100U };
    TEST_ASSERT(genHp.value() == 100U, "GeneratorHealth must initialize to 100");
    TEST_ASSERT(genHp.isAlive(), "Generator must be alive");
    genHp.applyDamage(40U);
    TEST_ASSERT(genHp.value() == 60U, "Generator health must be 60 after 40 damage");
    genHp.applyDamage(80U);
    TEST_ASSERT(genHp.value() == 0U, "Generator health must clamp to 0 on excess damage");
    TEST_ASSERT(!genHp.isAlive(), "Generator must not be alive at 0 HP");

    // Phase 12: BossHealth tests
    BossHealth bossHp { 150U, 150U };
    TEST_ASSERT(bossHp.current() == 150U, "Boss health current must be 150");
    TEST_ASSERT(bossHp.maximum() == 150U, "Boss health max must be 150");
    TEST_ASSERT(!bossHp.isDestroyed(), "Boss segment must not be destroyed");
    bossHp.applyDamage(50U);
    TEST_ASSERT(bossHp.current() == 100U, "Boss health must be 100 after 50 damage");
    bossHp.applyDamage(200U);
    TEST_ASSERT(bossHp.current() == 0U, "Boss health must clamp to 0");
    TEST_ASSERT(bossHp.isDestroyed(), "Boss segment must report isDestroyed");

    // Phase 12: EnergyTank tests
    const WorldPos tankPos { 256.0f, 128.0f };
    EnergyTank tank { tankPos };
    TEST_ASSERT(tank.position().x == 256.0f && tank.position().y == 128.0f, "Tank position must match");
    TEST_ASSERT(!tank.isCollected(), "Tank must not be collected initially");
    tank.collect();
    TEST_ASSERT(tank.isCollected(), "Tank must report collected after collect()");
}

} // namespace ShooterCore::Test
