#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include "AudioCommandQueue.h"
#include "BossArenaSystem.h"
#include "BossEntity.h"
#include "Hitbox.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore::Test {

static void testBossHierarchicalHitboxes() noexcept
{
    BossEntity boss { };
    AudioCommandQueue audio { };

    boss.init(BossArchetype::OrbitalCruiser, WorldPos { 300.0f, 200.0f });
    TEST_ASSERT(boss.partCount() == 5U, "Orbital cruiser must have 5 parts");
    TEST_ASSERT(boss.coreHealthPercent() == 100U, "Initial core health must be 100%");
    TEST_ASSERT(!boss.isDefeated(), "Boss must not be defeated initially");

    // Part 0 (Core) is initially armored
    TEST_ASSERT(boss.getPart(0).isArmored, "Core must be armored initially");

    // Attack core directly while armored -> should deflect
    const Hitbox coreHit { { 290.0f, 190.0f }, { 310.0f, 210.0f } };
    const bool hitCore = boss.applyDamage(coreHit, 100U, audio);
    TEST_ASSERT(hitCore, "Damage should register on core");
    TEST_ASSERT(boss.getPart(0).currentHp == 500U, "Core HP should not drop while armored");

    // Destroy all 4 secondary modules (Thrusters 1, 2 and Turrets 3, 4)
    // Part 1: Left thruster at offset {-48, -24} -> WorldPos {252, 176}
    boss.applyDamage(Hitbox { { 240.0f, 160.0f }, { 260.0f, 190.0f } }, 200U, audio);
    TEST_ASSERT(boss.getPart(1).isDestroyed, "Left thruster must be destroyed");

    // Part 2: Right thruster at offset {-48, 24} -> WorldPos {252, 224}
    boss.applyDamage(Hitbox { { 240.0f, 210.0f }, { 260.0f, 240.0f } }, 200U, audio);
    TEST_ASSERT(boss.getPart(2).isDestroyed, "Right thruster must be destroyed");

    // Part 3: Top turret at offset {0, -32} -> WorldPos {300, 168}
    boss.applyDamage(Hitbox { { 290.0f, 155.0f }, { 310.0f, 180.0f } }, 150U, audio);
    TEST_ASSERT(boss.getPart(3).isDestroyed, "Top turret must be destroyed");

    // Part 4: Bottom turret at offset {0, 32} -> WorldPos {300, 232}
    boss.applyDamage(Hitbox { { 290.0f, 220.0f }, { 310.0f, 245.0f } }, 150U, audio);
    TEST_ASSERT(boss.getPart(4).isDestroyed, "Bottom turret must be destroyed");

    // Now all secondary modules are destroyed -> core cowl should open (isArmored = false)!
    TEST_ASSERT(!boss.getPart(0).isArmored, "Core must be unarmored after modules destroyed");

    // Attack unarmored core
    boss.applyDamage(coreHit, 500U, audio);
    TEST_ASSERT(boss.getPart(0).currentHp == 0U, "Core must reach 0 HP");
    TEST_ASSERT(boss.isDefeated(), "Boss must be defeated after core destroyed");
}

static void testBossArenaLockdownAndClear() noexcept
{
    BossArenaSystem arena { };
    BossEntity boss { };
    AudioCommandQueue audio { };
    TileMap map { 40U, 20U, 16.0f };

    arena.configureArena(Hitbox { { 200.0f, 0.0f }, { 600.0f, 320.0f } }, 200.0f, WorldPos { 550.0f, 160.0f });

    // Register blast doors at column 12 (X ~ 192 px), rows 5..8
    for (std::uint32_t r { 5U }; r <= 8U; ++r) {
        arena.addBlastDoor(TileCoord { 12U, r });
    }

    TEST_ASSERT(!arena.isLockedDown(), "Arena must be dormant initially");
    TEST_ASSERT(!arena.isCleared(), "Arena must not be cleared initially");

    // Player outside arena threshold (X = 150 px)
    const DeltaTime dt { 1.0f / 60.0f };
    arena.update(dt, WorldPos { 150.0f, 100.0f }, boss, map, audio);
    TEST_ASSERT(!arena.isLockedDown(), "Player at 150 px must not trigger lockdown");
    TEST_ASSERT(!map.isSolid(TileCoord { 12U, 5U }), "Blast door must still be open");

    // Player crosses threshold at X = 205 px -> triggers lockdown
    arena.update(dt, WorldPos { 205.0f, 100.0f }, boss, map, audio);
    TEST_ASSERT(arena.isLockedDown(), "Arena must enter locked down state");
    TEST_ASSERT(map.isSolid(TileCoord { 12U, 5U }), "Blast door must now be sealed solid");

    // Defeat boss
    boss.init(BossArchetype::OrbitalCruiser, WorldPos { 400.0f, 160.0f });
    // Destroy parts and core
    boss.applyDamage(Hitbox { { 300.0f, 100.0f }, { 500.0f, 220.0f } }, 1000U, audio);
    boss.applyDamage(Hitbox { { 300.0f, 100.0f }, { 500.0f, 220.0f } }, 1000U, audio);

    // Update arena with defeated boss -> arena cleared & blast doors open!
    arena.update(dt, WorldPos { 250.0f, 100.0f }, boss, map, audio);
    TEST_ASSERT(arena.isCleared(), "Arena must be cleared after boss defeat");
    TEST_ASSERT(!map.isSolid(TileCoord { 12U, 5U }), "Blast door must open on clear");
}

void runBossArenaTests() noexcept
{
    testBossHierarchicalHitboxes();
    testBossArenaLockdownAndClear();
}

} // namespace ShooterCore::Test
