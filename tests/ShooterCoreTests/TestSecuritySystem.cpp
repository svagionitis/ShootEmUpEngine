#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include "AudioCommandQueue.h"
#include "Hitbox.h"
#include "RenderCommandQueue.h"
#include "SecuritySystem.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore::Test {

void runSecuritySystemTests() noexcept
{
    SecuritySystem security { };
    AudioCommandQueue audioQueue { };
    RenderCommandQueue renderQueue { };
    TileMap map { 64U, 64U, 16.0f };

    // 1. Registration
    const WorldPos genPos1 { 100.0f, 100.0f };
    const WorldPos genPos2 { 200.0f, 100.0f };
    TEST_ASSERT(security.registerGenerator(genPos1, 1U), "Generator 1 register must succeed");
    TEST_ASSERT(security.registerGenerator(genPos2, 1U), "Generator 2 register must succeed");
    TEST_ASSERT(security.activeCount() == 2U, "Active generator count must be 2");

    const Hitbox barrierBox { { 300.0f, 100.0f }, { 316.0f, 148.0f } };
    TEST_ASSERT(
        security.registerBarrier(WorldPos { 300.0f, 100.0f }, barrierBox, 1U, 2U), "Barrier register must succeed");
    TEST_ASSERT(!security.isBarrierOpen(1U), "Barrier must initially be locked");

    // 2. Tile Synchronization with Locked Barrier
    security.syncTiles(map);
    const TileCoord barrierCoord { WorldPos { 300.0f, 100.0f }.toTileCoord(16.0f) };
    TEST_ASSERT(map.isForcefield(barrierCoord), "Tile must be Forcefield when barrier locked");
    TEST_ASSERT(map.isSolid(barrierCoord), "Forcefield tile must be solid");

    // 3. Partial Damage to Generator 1
    const Hitbox attackHit1 { { 95.0f, 95.0f }, { 115.0f, 115.0f } };
    TEST_ASSERT(security.damageAt(attackHit1, 40U, audioQueue), "Attack overlapping Gen 1 must inflict damage");
    TEST_ASSERT(security.activeCount() == 2U, "Gen 1 is still alive after 40 damage");
    TEST_ASSERT(!security.isBarrierOpen(1U), "Barrier must still be locked");

    // 4. Destroy Generator 1
    TEST_ASSERT(security.damageAt(attackHit1, 70U, audioQueue), "Lethal damage to Gen 1 must succeed");
    TEST_ASSERT(security.activeCount() == 1U, "Active generators must decrease to 1");
    TEST_ASSERT(!security.isBarrierOpen(1U), "Barrier requires 2 generators, still locked");

    // 5. Destroy Generator 2
    const Hitbox attackHit2 { { 195.0f, 95.0f }, { 215.0f, 115.0f } };
    TEST_ASSERT(security.damageAt(attackHit2, 100U, audioQueue), "Lethal damage to Gen 2 must succeed");
    TEST_ASSERT(security.activeCount() == 0U, "Active generators must drop to 0");
    TEST_ASSERT(security.isBarrierOpen(1U), "Barrier 1 must now be open!");

    // 6. Tile Synchronization with Open Barrier
    security.syncTiles(map);
    TEST_ASSERT(!map.isForcefield(barrierCoord), "Tile must no longer be Forcefield after unlock");
    TEST_ASSERT(!map.isSolid(barrierCoord), "Disengaged barrier tile must be unobstructed");

    // 7. Render Commands
    security.emitRender(renderQueue);
    // Since all generators are inactive and barrier is open, no commands should be queued
    TEST_ASSERT(renderQueue.count() == 0U, "No render commands expected for destroyed generators and open gates");
}

} // namespace ShooterCore::Test
