#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include "AudioCommandQueue.h"
#include "Hitbox.h"
#include "ParticlePool.h"
#include "SecondaryArsenal.h"
#include "SupplyDropSystem.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore::Test {

static void testCanisterSpawningAndMagnetism() noexcept
{
    SupplyDropSystem drops { };
    SecondaryArsenal arsenal { };
    TileMap map { 20U, 20U, 16.0f };
    ParticlePool particles { };
    AudioCommandQueue audio { };
    EnergyPoints energy { 50U };
    ShieldGauge shield { 0U };

    TEST_ASSERT(drops.activeCount() == 0U, "Initial active canisters must be 0");
    const bool spawned { drops.spawnCanister(WorldPos { 100.0f, 100.0f }, CanisterType::MissilePack) };
    TEST_ASSERT(spawned, "Canister must spawn successfully");
    TEST_ASSERT(drops.activeCount() == 1U, "Active canisters must be 1");

    const auto& canisters { drops.getCanisters() };
    TEST_ASSERT(!canisters[0].magnetized, "Initial canister must not be magnetized");

    // Player far away (x = 300)
    const Hitbox farPlayerBox { WorldPos { 300.0f, 100.0f }, WorldPos { 316.0f, 132.0f } };
    const DeltaTime dt { 1.0f / 60.0f };
    drops.update(dt, farPlayerBox, map, arsenal, energy, shield, audio, particles);
    TEST_ASSERT(!canisters[0].magnetized, "Canister should not magnetize when player is far");

    // Player moves within 32 px (x = 120, y = 100)
    const Hitbox nearPlayerBox { WorldPos { 110.0f, 90.0f }, WorldPos { 126.0f, 122.0f } };
    drops.update(dt, nearPlayerBox, map, arsenal, energy, shield, audio, particles);
    TEST_ASSERT(canisters[0].magnetized, "Canister must magnetize when player is within 32px");
}

static void testCanisterCollectionPayloads() noexcept
{
    SupplyDropSystem drops { };
    SecondaryArsenal arsenal { };
    TileMap map { 20U, 20U, 16.0f };
    ParticlePool particles { };
    AudioCommandQueue audio { };
    EnergyPoints energy { 20U };
    ShieldGauge shield { 0U };

    // Consume some missile ammo
    arsenal.replenishMissiles(0U);
    const std::uint8_t initialMissiles { arsenal.missileAmmo().count() };

    // Spawn MissilePack directly on player
    drops.spawnCanister(WorldPos { 50.0f, 50.0f }, CanisterType::MissilePack);
    const Hitbox playerBox { WorldPos { 40.0f, 40.0f }, WorldPos { 60.0f, 60.0f } };

    const DeltaTime dt { 1.0f / 60.0f };
    drops.update(dt, playerBox, map, arsenal, energy, shield, audio, particles);

    TEST_ASSERT(drops.activeCount() == 0U, "Canister must be collected upon overlap");
    TEST_ASSERT(arsenal.missileAmmo().count() == initialMissiles + 6U, "MissilePack must add 6 missiles");
    TEST_ASSERT(audio.count() == 1U, "Pickup chime must be queued");

    // Test EnergyBoost payload
    drops.spawnCanister(WorldPos { 50.0f, 50.0f }, CanisterType::EnergyBoost);
    drops.update(dt, playerBox, map, arsenal, energy, shield, audio, particles);
    TEST_ASSERT(energy.value() == 50U, "EnergyBoost must add 30 energy points (20 -> 50)");

    // Test ShieldRecharge payload
    drops.spawnCanister(WorldPos { 50.0f, 50.0f }, CanisterType::ShieldRecharge);
    drops.update(dt, playerBox, map, arsenal, energy, shield, audio, particles);
    TEST_ASSERT(shield.percentage() == 100U, "ShieldRecharge must restore shield to 100%");
}

void runSupplyDropTests() noexcept
{
    testCanisterSpawningAndMagnetism();
    testCanisterCollectionPayloads();
}

} // namespace ShooterCore::Test
