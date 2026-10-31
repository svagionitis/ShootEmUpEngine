#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include "EntityPool.h"
#include "EntitySpawner.h"
#include "Hitbox.h"
#include "Types.h"

namespace ThexderGame::Test {

void runEntitySpawnerTests()
{
    LOG(INFO) << "Executing EntitySpawner Unit & Bounded Capacity Tests...";

    EntitySpawner spawner { };

    // 1. Nest count and initial state
    TEST_ASSERT_EQ(spawner.nestCount(), 8U, "Level 1 should configure exactly 8 generator nests");

    for (std::size_t i { 0U }; i < spawner.nestCount(); ++i) {
        const SpawnerNode& nest { spawner.getNest(i) };
        TEST_ASSERT(nest.pos.x > 0.0f && nest.pos.x < 160.0f * 16.0f, "Nest X coordinate must be in map bounds");
        TEST_ASSERT(nest.pos.y > 0.0f && nest.pos.y < 32.0f * 16.0f, "Nest Y coordinate must be in map bounds");
        TEST_ASSERT(nest.health > 0U, "Nest initial health must be greater than zero");
        TEST_ASSERT(!nest.destroyed, "Nest initial state must not be destroyed");
    }

    // 2. Spawning when player is near
    ShooterCore::EntityPool pool { };
    const SpawnerNode& nest0 { spawner.getNest(0U) };

    // Position player right beside nest 0
    const ShooterCore::WorldPos nearPos { nest0.pos.x + 10.0f, nest0.pos.y };
    spawner.updateSpawners(ShooterCore::DeltaTime { 4.0f }, nearPos, pool);

    TEST_ASSERT(pool.count() > 0U, "Nest should spawn drone after cooldown expiration");

    // 3. Pool Capacity Safety Invariant
    // Fill the pool so freeCount <= 4
    while ((pool.capacity() - pool.count()) > 4U) {
        const ShooterCore::WorldPos dummyPos { 10.0f, 10.0f };
        const ShooterCore::Hitbox dummyBox { dummyPos, ShooterCore::WorldPos { 20.0f, 20.0f } };
        (void)pool.spawn(ShooterCore::EntityType::PatrolWalker, dummyPos, dummyBox, 1U);
    }

    const std::size_t cappedCount { pool.count() };
    // Step spawner again near nest 0
    spawner.updateSpawners(ShooterCore::DeltaTime { 4.0f }, nearPos, pool);

    TEST_ASSERT_EQ(pool.count(), cappedCount,
        "Spawner must enforce pool headroom safety and refrain from spawning when freeCount <= 4");

    // 4. Nest Damage and Destruction Lifecycle
    TEST_ASSERT(!spawner.damageNest(0U, ShooterCore::DamageValue { 10U }), "Partial damage must not destroy nest");
    TEST_ASSERT_EQ(spawner.getNest(0U).health, 20U, "Nest health should decrement by 10");

    TEST_ASSERT(spawner.damageNest(0U, ShooterCore::DamageValue { 20U }), "Lethal damage must destroy nest");
    TEST_ASSERT(spawner.getNest(0U).destroyed, "Nest destroyed flag must be true");

    TEST_ASSERT(!spawner.damageNest(0U, ShooterCore::DamageValue { 10U }), "Destroyed nest returns false on hit");
}

} // namespace ThexderGame::Test
