#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include <array>

#include "AudioCommandQueue.h"
#include "EntityPool.h"
#include "Hitbox.h"
#include "ParticlePool.h"
#include "SecondaryArsenal.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore::Test {

static void testWeaponSelectorCycling() noexcept
{
    SecondaryArsenal arsenal { };
    TEST_ASSERT(arsenal.activeWeapon() == SecondaryWeaponType::None, "Initial secondary weapon must be None");

    arsenal.cycleWeapon();
    TEST_ASSERT(arsenal.activeWeapon() == SecondaryWeaponType::HomingMissile, "Cycle 1 must be HomingMissile");

    arsenal.cycleWeapon();
    TEST_ASSERT(arsenal.activeWeapon() == SecondaryWeaponType::FlashBomb, "Cycle 2 must be FlashBomb");

    arsenal.cycleWeapon();
    TEST_ASSERT(arsenal.activeWeapon() == SecondaryWeaponType::None, "Cycle 3 must wrap back to None");
}

static void testMissileAmmoAndFiring() noexcept
{
    SecondaryArsenal arsenal { };
    EntityPool entities { };
    AudioCommandQueue audio { };
    ParticlePool particles { };
    TileMap map { 20U, 20U, 16.0f };

    arsenal.setWeapon(SecondaryWeaponType::HomingMissile);
    TEST_ASSERT(arsenal.missileAmmo().count() == 12U, "Default missile ammo must be 12");

    // Spawn a target hostile entity at (100, 100)
    const EntityId targetId { entities.spawn(EntityType::PatrolWalker, WorldPos { 100.0f, 100.0f },
        Hitbox { WorldPos { 92.0f, 92.0f }, WorldPos { 108.0f, 108.0f } }, 40U) };
    TEST_ASSERT(targetId.isValid(), "Target entity must spawn successfully");

    // Fire missile toward right
    const bool fired { arsenal.triggerFire(
        WorldPos { 20.0f, 100.0f }, FacingDirection::Right, entities, audio, particles) };
    TEST_ASSERT(fired, "Missile firing must succeed");
    TEST_ASSERT(arsenal.missileAmmo().count() == 11U, "Missile ammo must decrement to 11");
    TEST_ASSERT(arsenal.activeMissileCount() == 1U, "Active missile count must be 1");
    TEST_ASSERT(audio.count() == 1U, "Audio command queue must receive MissileLaunch sound");

    // Advance simulation by 0.5s: missile should accelerate and steer toward target
    const DeltaTime dt { 1.0f / 60.0f };
    for (std::uint32_t i { 0U }; i < 30U; ++i) {
        arsenal.update(dt, entities, map, particles, audio);
    }

    const auto& missiles { arsenal.getMissiles() };
    TEST_ASSERT(missiles[0].pos.x > 20.0f, "Missile must have traveled forward along X");
}

static void testFlashBombDetonation() noexcept
{
    SecondaryArsenal arsenal { };
    EntityPool entities { };
    AudioCommandQueue audio { };
    ParticlePool particles { };
    TileMap map { 20U, 20U, 16.0f };

    // Place a destructible wall tile at (6, 5)
    map.setTile(TileCoord { 6U, 5U }, TileType::DestructibleWall);

    // Spawn 2 hostiles within 100 px of player
    const EntityId e1 { entities.spawn(EntityType::PatrolWalker, WorldPos { 120.0f, 80.0f },
        Hitbox { WorldPos { 112.0f, 72.0f }, WorldPos { 128.0f, 88.0f } }, 50U) };
    const EntityId e2 { entities.spawn(EntityType::DivingInterceptor, WorldPos { 90.0f, 90.0f },
        Hitbox { WorldPos { 82.0f, 82.0f }, WorldPos { 98.0f, 98.0f } }, 30U) };

    arsenal.setWeapon(SecondaryWeaponType::FlashBomb);
    TEST_ASSERT(arsenal.bombAmmo().count() == 1U, "Default bomb ammo must be 1");

    const bool fired { arsenal.triggerFire(
        WorldPos { 100.0f, 80.0f }, FacingDirection::Right, entities, audio, particles) };
    TEST_ASSERT(fired, "Flash bomb trigger must succeed");
    TEST_ASSERT(arsenal.bombAmmo().count() == 0U, "Bomb ammo must decrement to 0");
    TEST_ASSERT(arsenal.getFlashBomb().active, "Flash bomb state must be active");

    // Advance simulation: expanding shockwave sweeps over hostiles and destructible tile
    const DeltaTime dt { 1.0f / 60.0f };
    for (std::uint32_t i { 0U }; i < 20U; ++i) {
        arsenal.update(dt, entities, map, particles, audio);
    }

    const Entity* p1 { entities.get(e1) };
    const Entity* p2 { entities.get(e2) };
    TEST_ASSERT(p1 != nullptr && p1->health == 0U, "Hostile 1 must be disintegrated by flash bomb");
    TEST_ASSERT(p2 != nullptr && p2->health == 0U, "Hostile 2 must be disintegrated by flash bomb");
    TEST_ASSERT(map.getTile(TileCoord { 6U, 5U }) == TileType::Empty, "Destructible tile must be breached");
}

void runSecondaryArsenalTests() noexcept
{
    testWeaponSelectorCycling();
    testMissileAmmoAndFiring();
    testFlashBombDetonation();
}

} // namespace ShooterCore::Test
