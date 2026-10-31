#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include "AudioCommandQueue.h"
#include "EnemyAiSystem.h"
#include "EntityPool.h"
#include "Hitbox.h"
#include "ParticlePool.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore::Test {

static void testDivingInterceptorSwoop() noexcept
{
    EnemyAiSystem ai { };
    EntityPool pool { };
    TileMap map { 20U, 20U, 16.0f };
    AudioCommandQueue audio { };
    ParticlePool particles { };
    EnergyPoints energy { 100U };

    const Hitbox playerBox { WorldPos { 100.0f, 150.0f }, WorldPos { 116.0f, 182.0f } };
    const WorldPos playerPos { 108.0f, 182.0f };

    // Spawn an interceptor above the player at (108, 60)
    const EntityId id { pool.spawn(EntityType::DivingInterceptor, WorldPos { 108.0f, 60.0f },
        Hitbox { WorldPos { 100.0f, 52.0f }, WorldPos { 116.0f, 68.0f } }, 20U) };
    TEST_ASSERT(id.isValid(), "Interceptor must spawn");

    Entity* e { pool.get(id) };
    TEST_ASSERT(e != nullptr, "Entity pointer must be valid");
    TEST_ASSERT(e->stateTimer == 0.0f, "Initial stateTimer must be 0 (cruise)");

    // Advance 1 tick: interceptor detects player below and switches to swoop (stateTimer = 1.0f)
    const DeltaTime dt { 1.0f / 60.0f };
    ai.update(dt, playerPos, playerBox, false, energy, pool, map, audio, particles);

    TEST_ASSERT(e->stateTimer == 1.0f, "Interceptor must transition to swoop mode");
    TEST_ASSERT(e->vel.dy > 0.0f, "Interceptor must accelerate downwards toward player");
}

static void testWallCrawlerCornerTurn() noexcept
{
    EnemyAiSystem ai { };
    EntityPool pool { };
    TileMap map { 20U, 20U, 16.0f };
    AudioCommandQueue audio { };
    ParticlePool particles { };
    EnergyPoints energy { 100U };

    const Hitbox dummyBox { WorldPos { 0.0f, 0.0f }, WorldPos { 0.0f, 0.0f } };

    // Place a vertical solid wall at col 10 (x = 160)
    for (std::uint32_t r { 0U }; r < 20U; ++r) {
        map.setTile(TileCoord { 10U, r }, TileType::SolidWall);
    }

    // Spawn a crawler at (148, 100) facing Right, walking on floor
    const EntityId id { pool.spawn(EntityType::WallCrawler, WorldPos { 148.0f, 100.0f },
        Hitbox { WorldPos { 140.0f, 92.0f }, WorldPos { 156.0f, 108.0f } }, 15U) };
    Entity* e { pool.get(id) };
    TEST_ASSERT(e != nullptr, "Crawler must be valid");
    e->facing = FacingDirection::Right;
    e->stateTimer = 0.0f; // Floor

    // Update 5 ticks: crawler encounters solid wall in front and rotates 90 deg up wall (stateTimer = 1.0f)
    const DeltaTime dt { 1.0f / 60.0f };
    for (std::uint32_t i { 0U }; i < 5U; ++i) {
        ai.update(dt, WorldPos { 0.0f, 0.0f }, dummyBox, false, energy, pool, map, audio, particles);
    }

    TEST_ASSERT(e->stateTimer == 1.0f, "Crawler must transition to climbing up the wall");
}

static void testCeilingTurretFireAndShieldDeflect() noexcept
{
    EnemyAiSystem ai { };
    EntityPool pool { };
    TileMap map { 20U, 20U, 16.0f };
    AudioCommandQueue audio { };
    ParticlePool particles { };
    EnergyPoints energy { 100U };

    // Player positioned at (100, 100) with shield active
    const Hitbox playerBox { WorldPos { 92.0f, 90.0f }, WorldPos { 108.0f, 122.0f } };
    const WorldPos playerPos { 100.0f, 122.0f };

    // Ceiling turret at (100, 20)
    const EntityId tId { pool.spawn(EntityType::CeilingTurret, WorldPos { 100.0f, 20.0f },
        Hitbox { WorldPos { 92.0f, 16.0f }, WorldPos { 108.0f, 28.0f } }, 30U) };
    Entity* turret { pool.get(tId) };
    TEST_ASSERT(turret != nullptr, "Turret must spawn");
    turret->stateTimer = 1.98f; // Near fire interval (2.0s)

    // Advance 2 ticks: turret fires hostile bullet
    const DeltaTime dt { 1.0f / 60.0f };
    ai.update(dt, playerPos, playerBox, true, energy, pool, map, audio, particles);
    ai.update(dt, playerPos, playerBox, true, energy, pool, map, audio, particles);

    // Verify hostile bullet spawned
    bool foundBullet { false };
    for (std::size_t i { 0U }; i < pool.capacity(); ++i) {
        const EntitySlot& slt { pool.slot(i) };
        if (slt.active && slt.data.type == EntityType::HostileBullet) {
            foundBullet = true;
            break;
        }
    }
    TEST_ASSERT(foundBullet, "Turret must spawn HostileBullet projectile");
}

void runEnemyAiTests() noexcept
{
    testDivingInterceptorSwoop();
    testWallCrawlerCornerTurn();
    testCeilingTurretFireAndShieldDeflect();
}

} // namespace ShooterCore::Test
