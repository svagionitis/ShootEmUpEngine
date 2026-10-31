#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include "AudioCommandQueue.h"
#include "EntityPool.h"
#include "NestManager.h"
#include "ParticlePool.h"
#include "SupplyDropSystem.h"
#include "Types.h"

namespace ShooterCore::Test {

static void testNestRegistrationAndProximity() noexcept
{
    NestManager manager { };
    EntityPool entities { };
    SupplyDropSystem drops { };
    ParticlePool particles { };
    AudioCommandQueue audio { };

    TEST_ASSERT(manager.nestCount() == 0U, "Initial nest count must be 0");
    const bool added { manager.addNest(WorldPos { 200.0f, 150.0f }, 80U, 4.0f) };
    TEST_ASSERT(added, "addNest must succeed");
    TEST_ASSERT(manager.nestCount() == 1U, "Nest count must be 1");

    const auto& nests { manager.getNests() };
    TEST_ASSERT(nests[0].active, "Nest must be active");
    TEST_ASSERT(nests[0].health == 80U, "Nest health must be 80");

    // Player far away (> 160 px): interval is 4.0s
    const WorldPos farPlayer { 400.0f, 400.0f };
    const DeltaTime dt { 1.0f };
    manager.update(dt, farPlayer, entities, drops, particles, audio);
    TEST_ASSERT(entities.count() == 0U, "Far player should not trigger spawn after 1.0s");

    // Player moves close (< 160 px): interval is 1.2s
    const WorldPos nearPlayer { 220.0f, 150.0f };
    manager.update(DeltaTime { 0.3f }, nearPlayer, entities, drops, particles, audio);
    TEST_ASSERT(entities.count() == 1U, "Near player must trigger accelerated wave spawn");
}

static void testNestDamageAndLootDrop() noexcept
{
    NestManager manager { };
    SupplyDropSystem drops { };
    ParticlePool particles { };
    AudioCommandQueue audio { };

    manager.addNest(WorldPos { 100.0f, 100.0f }, 50U, 4.0f);
    TEST_ASSERT(drops.activeCount() == 0U, "Initial drops must be 0");

    // Non-lethal damage
    const bool destroyed1 { manager.damageNest(0U, DamageValue { 20U }, drops, particles, audio) };
    TEST_ASSERT(!destroyed1, "Nest should survive 20 dmg");
    TEST_ASSERT(manager.getNests()[0].health == 30U, "Nest health must drop to 30");
    TEST_ASSERT(drops.activeCount() == 0U, "No drop until destroyed");

    // Lethal damage
    const bool destroyed2 { manager.damageNest(0U, DamageValue { 30U }, drops, particles, audio) };
    TEST_ASSERT(destroyed2, "Nest must be destroyed by 30 dmg");
    TEST_ASSERT(manager.getNests()[0].destroyed, "Nest destroyed flag must be true");
    TEST_ASSERT(drops.activeCount() == 1U, "Destroyed nest must spawn 1 cargo canister drop");
}

void runNestManagerTests() noexcept
{
    testNestRegistrationAndProximity();
    testNestDamageAndLootDrop();
}

} // namespace ShooterCore::Test
