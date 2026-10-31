#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include "Hitbox.h"
#include "LabyrinthMap.h"
#include "TileMap.h"
#include "TransformSystem.h"
#include "Types.h"

namespace ShooterCore::Test {

void runLabyrinthArchitectureTests() noexcept
{
    TileMap map { LabyrinthMap::kMapWidth, LabyrinthMap::kMapHeight, LabyrinthMap::kTileSize };
    LabyrinthMap::build(map);

    // 1. Map Dimensions & Perimeter Verification
    TEST_ASSERT(map.width() == 256U, "Labyrinth width must be 256");
    TEST_ASSERT(map.height() == 128U, "Labyrinth height must be 128");
    TEST_ASSERT(map.isSolid(TileCoord { 0U, 0U }), "Perimeter top-left must be solid");
    TEST_ASSERT(map.isSolid(TileCoord { 255U, 127U }), "Perimeter bottom-right must be solid");

    // 2. Sector Partitioning & Identification
    const WorldPos alphaPos { 64.0f * 16.0f, 32.0f * 16.0f };
    const WorldPos betaPos { 180.0f * 16.0f, 32.0f * 16.0f };
    const WorldPos gammaPos { 64.0f * 16.0f, 96.0f * 16.0f };
    const WorldPos deltaPos { 200.0f * 16.0f, 96.0f * 16.0f };

    TEST_ASSERT(LabyrinthMap::getSectorAt(alphaPos) == SectorId::Alpha, "Pos in Alpha must map to SectorId::Alpha");
    TEST_ASSERT(LabyrinthMap::getSectorAt(betaPos) == SectorId::Beta, "Pos in Beta must map to SectorId::Beta");
    TEST_ASSERT(LabyrinthMap::getSectorAt(gammaPos) == SectorId::Gamma, "Pos in Gamma must map to SectorId::Gamma");
    TEST_ASSERT(LabyrinthMap::getSectorAt(deltaPos) == SectorId::Delta, "Pos in Delta must map to SectorId::Delta");

    const SectorBounds alphaBounds { LabyrinthMap::getSectorBounds(SectorId::Alpha) };
    TEST_ASSERT(alphaBounds.contains(alphaPos), "Alpha bounds must contain alphaPos");
    TEST_ASSERT(!alphaBounds.contains(betaPos), "Alpha bounds must not contain betaPos");

    // 3. 1-Tile Crawling Conduit Clearance in Sector Beta
    // Row 19 is an open 1-tile flue (16px high) between solid rows 18 and 20.
    TEST_ASSERT(map.isSolid(TileCoord { 150U, 18U }), "Row 18 must be solid ceiling");
    TEST_ASSERT(!map.isSolid(TileCoord { 150U, 19U }), "Row 19 must be open air gap");
    TEST_ASSERT(map.isSolid(TileCoord { 150U, 20U }), "Row 20 must be solid floor");

    // Jet Mode AABB: 24w x 12h placed in row 19 (y: 304 to 320)
    const Hitbox jetBox { { 150.0f * 16.0f, 19.0f * 16.0f + 2.0f }, { 150.0f * 16.0f + 24.0f, 19.0f * 16.0f + 14.0f } };
    TEST_ASSERT(map.hasClearance(jetBox), "Jet mode box must clear 1-tile conduit");

    // Robot Mode AABB: 16w x 28h placed at same floor
    const Hitbox robotBox { { 150.0f * 16.0f, 20.0f * 16.0f - 28.0f }, { 150.0f * 16.0f + 16.0f, 20.0f * 16.0f } };
    TEST_ASSERT(!map.hasClearance(robotBox), "Robot mode box must be obstructed in 1-tile conduit");

    // 4. Transform Clearance Invariant
    TransformSystem transformSys { };
    // Force into Jet mode
    transformSys.forceState(TransformMode::Jet);
    TEST_ASSERT(transformSys.mode() == TransformMode::Jet, "Must be in Jet mode");

    // Attempt transformation to Robot while inside 1-tile flue (bottom-center at floor y=320.0)
    const WorldPos inFluePos { 150.0f * 16.0f + 8.0f, 20.0f * 16.0f };
    const TransformResult res { transformSys.requestTransform(inFluePos, map) };
    TEST_ASSERT(
        res == TransformResult::RejectedObstruction, "Transform to Robot in 1-tile flue must be RejectedObstruction");
    TEST_ASSERT(transformSys.mode() == TransformMode::Jet, "Mode must remain Jet when blocked");

    // 5. Secret Wall Properties
    TEST_ASSERT(map.isSecret(TileCoord { 20U, 18U }), "Coord (20, 18) must be secret wall");
    TEST_ASSERT(map.isSolid(TileCoord { 20U, 18U }), "Secret wall must be solid before breach");
    TEST_ASSERT(map.isDestructible(TileCoord { 20U, 18U }), "Secret wall must be destructible");
    TEST_ASSERT(map.destroyTile(TileCoord { 20U, 18U }), "destroyTile on secret wall must succeed");
    TEST_ASSERT(!map.isSolid(TileCoord { 20U, 18U }), "Breached secret wall must become empty space");
}

} // namespace ShooterCore::Test
