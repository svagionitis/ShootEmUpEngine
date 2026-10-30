#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include "EntityPool.h"
#include "Hitbox.h"
#include "Types.h"

namespace ShooterCore::Test {

void runEntityPoolTests() noexcept
{
    // Test 1: Initial state
    {
        EntityPool pool { };
        TEST_ASSERT(pool.count() == 0U, "Pool count must be 0 initially");
        TEST_ASSERT(pool.capacity() == 128U, "Pool capacity must be 128");
        TEST_ASSERT(!pool.isFull(), "Pool must not be full initially");
    }

    // Test 2: Spawn and recycle
    {
        EntityPool pool { };
        const Hitbox box { WorldPos { 0.0f, 0.0f }, WorldPos { 16.0f, 16.0f } };
        const EntityId id1 { pool.spawn(EntityType::PatrolWalker, WorldPos { 10.0f, 20.0f }, box, 5U) };

        TEST_ASSERT(id1.isValid(), "Spawned EntityId must be valid");
        TEST_ASSERT(pool.count() == 1U, "Active count must be 1");
        TEST_ASSERT(pool.isValid(id1), "pool.isValid(id1) must be true");

        Entity* ent { pool.get(id1) };
        TEST_ASSERT(ent != nullptr, "get(id1) must not be nullptr");
        TEST_ASSERT(ent->type == EntityType::PatrolWalker, "Entity type must match");
        TEST_ASSERT(ent->pos.x == 10.0f && ent->pos.y == 20.0f, "Entity pos must match");
        TEST_ASSERT(ent->health == 5U, "Entity health must be 5");

        const bool despawned { pool.despawn(id1) };
        TEST_ASSERT(despawned, "despawn must succeed");
        TEST_ASSERT(pool.count() == 0U, "Active count must be 0 after despawn");
        TEST_ASSERT(!pool.isValid(id1), "Old id1 must no longer be valid");
        TEST_ASSERT(pool.get(id1) == nullptr, "get(id1) must return nullptr after despawn");
    }

    // Test 3: Stale handle rejection upon slot reuse
    {
        EntityPool pool { };
        const Hitbox box { WorldPos { 0.0f, 0.0f }, WorldPos { 16.0f, 16.0f } };
        const EntityId firstId { pool.spawn(EntityType::PatrolWalker, WorldPos { 0.0f, 0.0f }, box, 1U) };
        const std::uint16_t slotIndex { firstId.index() };
        const std::uint16_t firstGen { firstId.generation() };

        pool.despawn(firstId);

        // Spawn new entity which reuses the same slot
        const EntityId secondId { pool.spawn(EntityType::HomingFlyer, WorldPos { 50.0f, 50.0f }, box, 2U) };
        TEST_ASSERT(secondId.index() == slotIndex, "Reused slot should match previous slotIndex");
        TEST_ASSERT(secondId.generation() != firstGen, "New entity must have a different generation");
        TEST_ASSERT(pool.isValid(secondId), "secondId must be valid");
        TEST_ASSERT(!pool.isValid(firstId), "firstId must remain stale and invalid");
        TEST_ASSERT(pool.get(firstId) == nullptr, "get(firstId) must return nullptr");

        const Entity* secEnt { pool.get(secondId) };
        TEST_ASSERT(secEnt != nullptr, "get(secondId) must succeed");
        TEST_ASSERT(secEnt->type == EntityType::HomingFlyer, "Second entity type must be HomingFlyer");
    }

    // Test 4: Full capacity and safe overflow rejection
    {
        EntityPool pool { };
        const Hitbox box { WorldPos { 0.0f, 0.0f }, WorldPos { 8.0f, 8.0f } };

        for (std::size_t i { 0U }; i < EntityPool::kCapacity; ++i) {
            const EntityId id { pool.spawn(EntityType::Particle, WorldPos { 0.0f, 0.0f }, box, 1U) };
            TEST_ASSERT(id.isValid(), "Spawning up to capacity must succeed");
        }

        TEST_ASSERT(pool.isFull(), "Pool must be full after 128 spawns");
        TEST_ASSERT(pool.count() == EntityPool::kCapacity, "Count must equal capacity");

        const EntityId overflowId { pool.spawn(EntityType::Particle, WorldPos { 0.0f, 0.0f }, box, 1U) };
        TEST_ASSERT(!overflowId.isValid(), "Overflow spawn must return invalid EntityId");
        TEST_ASSERT(pool.count() == EntityPool::kCapacity, "Count must not exceed capacity");

        pool.clear();
        TEST_ASSERT(pool.count() == 0U, "Pool count must be 0 after clear");
        TEST_ASSERT(!pool.isFull(), "Pool must not be full after clear");
    }
}

} // namespace ShooterCore::Test
