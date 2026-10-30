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
}

} // namespace ShooterCore::Test
