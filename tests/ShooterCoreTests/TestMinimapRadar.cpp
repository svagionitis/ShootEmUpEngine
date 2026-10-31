#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include "MinimapRadar.h"
#include "RenderCommandQueue.h"
#include "SecuritySystem.h"
#include "Types.h"

namespace ShooterCore::Test {

void runMinimapRadarTests() noexcept
{
    MinimapRadar radar { };
    SecuritySystem security { };
    RenderCommandQueue renderQueue { };

    // 1. Initial State: All chunks undiscovered
    for (std::size_t r { 0U }; r < MinimapRadar::kChunkRows; ++r) {
        for (std::size_t c { 0U }; c < MinimapRadar::kChunkCols; ++c) {
            TEST_ASSERT(!radar.isDiscovered(c, r), "All chunks must start undiscovered");
        }
    }

    // 2. Direct Chunk Discovery
    radar.setDiscovered(5U, 8U);
    TEST_ASSERT(radar.isDiscovered(5U, 8U), "Chunk (5, 8) must report discovered");
    TEST_ASSERT(!radar.isDiscovered(4U, 8U), "Neighbor chunk (4, 8) must still be undiscovered");

    // Out of bounds safety
    radar.setDiscovered(100U, 100U);
    TEST_ASSERT(!radar.isDiscovered(100U, 100U), "Out of bounds chunk must be safe and false");

    // 3. Radius Discovery Around Player
    // Chunk extent is 128px. Position (640, 640) maps to chunk (5, 5).
    const WorldPos playerPos { 640.0f, 640.0f };
    radar.updateDiscovery(playerPos);

    TEST_ASSERT(radar.isDiscovered(5U, 5U), "Center chunk (5, 5) must be discovered");
    TEST_ASSERT(radar.isDiscovered(4U, 4U), "Diagonal neighbor (4, 4) must be discovered");
    TEST_ASSERT(radar.isDiscovered(6U, 6U), "Diagonal neighbor (6, 6) must be discovered");
    TEST_ASSERT(radar.isDiscovered(5U, 6U), "Adjacent neighbor (5, 6) must be discovered");

    // 4. Render Telemetry Output
    security.registerGenerator(WorldPos { 500.0f, 500.0f }, 1U);
    radar.emitCommands(renderQueue, playerPos, security, WorldPos { 3500.0f, 1500.0f }, true);

    // Should include: background + discovered chunks (1 + 9 = 10) + generator + boss + player
    TEST_ASSERT(renderQueue.count() > 0U, "Radar must emit draw commands to render queue");
}

} // namespace ShooterCore::Test
