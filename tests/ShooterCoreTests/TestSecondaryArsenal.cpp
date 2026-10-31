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
    TEST_ASSERT(arsenal.currentAmmo().count() == 0U, "Ammo for None must be 0");

    arsenal.cycleWeapon();
    TEST_ASSERT(arsenal.activeWeapon() == SecondaryWeaponType::StandardMissile, "Cycle 1 must be StandardMissile");
    TEST_ASSERT(arsenal.currentAmmo().count() == 15U, "Initial standard missile ammo must be 15");

    arsenal.cycleWeapon();
    TEST_ASSERT(arsenal.activeWeapon() == SecondaryWeaponType::HomingMissile, "Cycle 2 must be HomingMissile");
    TEST_ASSERT(arsenal.currentAmmo().count() == 12U, "Initial homing missile ammo must be 12");

    arsenal.cycleWeapon();
    TEST_ASSERT(arsenal.activeWeapon() == SecondaryWeaponType::NapalmBomb, "Cycle 3 must be NapalmBomb");
    TEST_ASSERT(arsenal.currentAmmo().count() == 6U, "Initial napalm ammo must be 6");

    arsenal.cycleWeapon();
    TEST_ASSERT(arsenal.activeWeapon() == SecondaryWeaponType::FlashBomb, "Cycle 4 must be FlashBomb");
    TEST_ASSERT(arsenal.currentAmmo().count() == 2U, "Initial flash bomb ammo must be 2");

    arsenal.cycleWeapon();
    TEST_ASSERT(arsenal.activeWeapon() == SecondaryWeaponType::DeflectorShield, "Cycle 5 must be DeflectorShield");
    TEST_ASSERT(arsenal.currentAmmo().count() == 3U, "Initial deflector ammo must be 3");

    arsenal.cycleWeapon();
    TEST_ASSERT(arsenal.activeWeapon() == SecondaryWeaponType::None, "Cycle 6 must wrap back to None");
}

static void testStandardMissileDemolition() noexcept
{
    SecondaryArsenal arsenal { };
    EntityPool entities { };
    AudioCommandQueue audio { };
    ParticlePool particles { };
    TileMap map { 20U, 20U, 16.0f };

    // Place a destructible barrier spanning cols 7..8 and rows 4..6
    for (std::uint32_t r { 4U }; r <= 6U; ++r) {
        for (std::uint32_t c { 7U }; c <= 8U; ++c) {
            map.setTile(TileCoord { c, r }, TileType::DestructibleWall);
        }
    }

    arsenal.setWeapon(SecondaryWeaponType::StandardMissile);
    TEST_ASSERT(arsenal.standardMissileAmmo().count() == 15U, "Initial standard missile count must be 15");

    const bool fired { arsenal.triggerFire(
        WorldPos { 40.0f, 88.0f }, FacingDirection::Right, entities, audio, particles) };
    TEST_ASSERT(fired, "Standard missile fire must succeed");
    TEST_ASSERT(arsenal.standardMissileAmmo().count() == 14U, "Ammo must decrement to 14");
    TEST_ASSERT(arsenal.activeStandardMissileCount() == 1U, "Active standard missiles must be 1");

    // Advance simulation until missile strikes wall at X ~ 112 px
    const DeltaTime dt { 1.0f / 60.0f };
    for (std::uint32_t i { 0U }; i < 20U; ++i) {
        arsenal.update(dt, entities, map, particles, audio);
    }

    // Verify destructible wall shattered
    for (std::uint32_t r { 4U }; r <= 6U; ++r) {
        for (std::uint32_t c { 7U }; c <= 8U; ++c) {
            TEST_ASSERT(map.getTile(TileCoord { c, r }) == TileType::Empty, "Destructible tile must be cleared");
        }
    }
    TEST_ASSERT(arsenal.activeStandardMissileCount() == 0U, "Missile must explode on impact");
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

static void testNapalmBombAndGroundBurn() noexcept
{
    SecondaryArsenal arsenal { };
    EntityPool entities { };
    AudioCommandQueue audio { };
    ParticlePool particles { };
    TileMap map { 20U, 20U, 16.0f };

    // Place solid floor at row 10 (Y = 160 px)
    for (std::uint32_t c { 0U }; c < 20U; ++c) {
        map.setTile(TileCoord { c, 10U }, TileType::SolidWall);
    }

    // Spawn a crawler enemy on the floor at (100, 150)
    const EntityId crawler { entities.spawn(EntityType::WallCrawler, WorldPos { 100.0f, 150.0f },
        Hitbox { WorldPos { 92.0f, 142.0f }, WorldPos { 108.0f, 158.0f } }, 30U) };
    TEST_ASSERT(crawler.isValid(), "Crawler must spawn successfully");

    arsenal.setWeapon(SecondaryWeaponType::NapalmBomb);
    TEST_ASSERT(arsenal.napalmAmmo().count() == 6U, "Default napalm ammo must be 6");

    // Drop napalm from (90, 80)
    const bool dropped { arsenal.triggerFire(
        WorldPos { 90.0f, 80.0f }, FacingDirection::Right, entities, audio, particles) };
    TEST_ASSERT(dropped, "Napalm drop must succeed");
    TEST_ASSERT(arsenal.napalmAmmo().count() == 5U, "Napalm ammo must decrement to 5");

    // Advance simulation until napalm strikes floor and ignites pool
    const DeltaTime dt { 1.0f / 60.0f };
    for (std::uint32_t i { 0U }; i < 45U; ++i) {
        arsenal.update(dt, entities, map, particles, audio);
    }

    TEST_ASSERT(arsenal.activeNapalmPoolCount() > 0U, "Napalm pool must be active on ground");

    // Advance burn ticks for 1 second (60 frames)
    for (std::uint32_t i { 0U }; i < 60U; ++i) {
        arsenal.update(dt, entities, map, particles, audio);
    }

    const Entity* pCrawler { entities.get(crawler) };
    TEST_ASSERT(pCrawler != nullptr && pCrawler->health < 30U, "Crawler on floor must receive continuous burn damage");
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
    TEST_ASSERT(arsenal.bombAmmo().count() == 2U, "Default bomb ammo must be 2");

    const bool fired { arsenal.triggerFire(
        WorldPos { 100.0f, 80.0f }, FacingDirection::Right, entities, audio, particles) };
    TEST_ASSERT(fired, "Flash bomb trigger must succeed");
    TEST_ASSERT(arsenal.bombAmmo().count() == 1U, "Bomb ammo must decrement to 1");
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

static void testDeflectorShieldReflection() noexcept
{
    SecondaryArsenal arsenal { };
    EntityPool entities { };
    AudioCommandQueue audio { };
    ParticlePool particles { };
    TileMap map { 20U, 20U, 16.0f };

    arsenal.setWeapon(SecondaryWeaponType::DeflectorShield);
    TEST_ASSERT(arsenal.deflectorAmmo().count() == 3U, "Default deflector ammo must be 3");

    const bool activated { arsenal.triggerFire(
        WorldPos { 100.0f, 100.0f }, FacingDirection::Right, entities, audio, particles) };
    TEST_ASSERT(activated, "Deflector activation must succeed");
    TEST_ASSERT(arsenal.deflectorAmmo().count() == 2U, "Deflector ammo must decrement to 2");
    TEST_ASSERT(arsenal.isDeflectorActive(), "Deflector must be active");
    TEST_ASSERT(arsenal.getDeflector().hitpoints == 100U, "Initial deflector HP must be 100");

    // Spawn an incoming enemy projectile heading toward player from (116, 100) with vx = -100
    const EntityId projId { entities.spawn(EntityType::HostileBullet, WorldPos { 116.0f, 100.0f },
        Hitbox { WorldPos { 112.0f, 96.0f }, WorldPos { 120.0f, 104.0f } }, 10U) };
    Entity* proj { entities.get(projId) };
    TEST_ASSERT(proj != nullptr, "Enemy projectile must spawn");
    proj->vel = Velocity { -100.0f, 0.0f };

    // Advance 1 frame
    const DeltaTime dt { 1.0f / 60.0f };
    arsenal.update(dt, entities, map, particles, audio);

    // Projectile velocity must be inverted (vx > 0)
    TEST_ASSERT(proj->vel.dx > 0.0f, "Projectile velocity must be reflected by deflector");
    TEST_ASSERT(arsenal.getDeflector().hitpoints < 100U, "Deflector HP must absorb hit");
}

static void testReplenishMethods() noexcept
{
    SecondaryArsenal arsenal { };
    arsenal.replenishStandardMissiles(10U);
    TEST_ASSERT(arsenal.standardMissileAmmo().count() == 25U, "15 + 10 = 25 standard missiles");

    arsenal.replenishHomingMissiles(5U);
    TEST_ASSERT(arsenal.homingMissileAmmo().count() == 17U, "12 + 5 = 17 homing missiles");

    arsenal.replenishNapalm(5U);
    TEST_ASSERT(arsenal.napalmAmmo().count() == 11U, "6 + 5 = 11 napalm");

    arsenal.replenishBombs(2U);
    TEST_ASSERT(arsenal.bombAmmo().count() == 4U, "2 + 2 = 4 flash bombs");

    arsenal.replenishDeflector(3U);
    TEST_ASSERT(arsenal.deflectorAmmo().count() == 6U, "3 + 3 = 6 deflector cells");
}

void runSecondaryArsenalTests() noexcept
{
    testWeaponSelectorCycling();
    testStandardMissileDemolition();
    testMissileAmmoAndFiring();
    testNapalmBombAndGroundBurn();
    testFlashBombDetonation();
    testDeflectorShieldReflection();
    testReplenishMethods();
}

} // namespace ShooterCore::Test
