#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include "AudioCommandQueue.h"
#include "Hitbox.h"
#include "RenderCommandQueue.h"
#include "SecretManager.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore::Test {

void runSecretManagerTests() noexcept
{
    SecretManager secrets { };
    AudioCommandQueue audioQueue { };
    RenderCommandQueue renderQueue { };
    TileMap map { 32U, 32U, 16.0f };

    // 1. Setup secret wall
    const TileCoord secretCoord { 10U, 10U };
    map.setTile(secretCoord, TileType::SecretWall);
    TEST_ASSERT(secrets.registerSecretWall(secretCoord), "Register secret wall must succeed");

    // 2. Damage and breach secret wall
    TEST_ASSERT(secrets.damageSecretWall(secretCoord, map, audioQueue), "damageSecretWall must return true");
    TEST_ASSERT(secrets.damageSecretWall(secretCoord, map, audioQueue), "Second damage tick");
    TEST_ASSERT(secrets.damageSecretWall(secretCoord, map, audioQueue), "Third damage tick destroys secret wall");
    TEST_ASSERT(!map.isSolid(secretCoord), "Breached wall must be open");

    // 3. Register Energy Tanks
    const WorldPos tankPos1 { 160.0f, 160.0f };
    const WorldPos tankPos2 { 240.0f, 160.0f };
    TEST_ASSERT(secrets.registerEnergyTank(tankPos1), "Register tank 1 must succeed");
    TEST_ASSERT(secrets.registerEnergyTank(tankPos2), "Register tank 2 must succeed");
    TEST_ASSERT(secrets.tankCount() == 2U, "Tank count must be 2");

    // 4. Initial Player State
    std::uint16_t maxEnergy { 100U };
    EnergyPoints playerEnergy { 30U };
    const Hitbox playerMissBox { { 50.0f, 50.0f }, { 66.0f, 78.0f } };

    // No overlap
    TEST_ASSERT(!secrets.checkPickup(playerMissBox, playerEnergy, maxEnergy, audioQueue),
        "No pickup when player is away from tanks");
    TEST_ASSERT(maxEnergy == 100U, "Max energy unchanged");
    TEST_ASSERT(playerEnergy.value() == 30U, "Player energy unchanged");

    // 5. Overlap Tank 1
    const Hitbox playerHitBox { { 158.0f, 158.0f }, { 174.0f, 186.0f } };
    TEST_ASSERT(secrets.checkPickup(playerHitBox, playerEnergy, maxEnergy, audioQueue),
        "Pickup must succeed when overlapping tank");
    TEST_ASSERT(maxEnergy == 120U, "Max energy must increase to 120");
    TEST_ASSERT(playerEnergy.value() == 120U, "Player energy must be fully refilled to 120");

    // 6. Cannot pick up Tank 1 twice
    TEST_ASSERT(!secrets.checkPickup(playerHitBox, playerEnergy, maxEnergy, audioQueue),
        "Cannot pick up already collected tank");

    // 7. Render Tank 2 (Tank 1 collected)
    secrets.emitRender(renderQueue);
    TEST_ASSERT(renderQueue.count() == 1U, "Only uncollected Tank 2 should emit a render command");
}

} // namespace ShooterCore::Test
