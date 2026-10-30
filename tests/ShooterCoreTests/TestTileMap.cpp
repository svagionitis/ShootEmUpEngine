#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include "Hitbox.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore::Test {

void runTileMapTests() noexcept
{
    // Test 1: Grid Dimensions and Default Empty State
    TileMap map { 10U, 10U, 16.0f };
    TEST_ASSERT(map.width() == 10U, "TileMap width must match initialization");
    TEST_ASSERT(map.height() == 10U, "TileMap height must match initialization");
    TEST_ASSERT(map.tileSize() == 16.0f, "TileMap tileSize must match initialization");
    TEST_ASSERT(map.getTile(TileCoord { 0U, 0U }) == TileType::Empty, "Default cells must be Empty");

    // Test 2: Out of Bounds Coordinates enforce Solid Wall perimeter
    TEST_ASSERT(map.getTile(TileCoord { 10U, 0U }) == TileType::SolidWall, "Out-of-bounds col must be SolidWall");
    TEST_ASSERT(map.getTile(TileCoord { 0U, 10U }) == TileType::SolidWall, "Out-of-bounds row must be SolidWall");
    TEST_ASSERT(map.isSolid(TileCoord { 99U, 99U }), "Out-of-bounds must be solid");

    // Test 3: Mutating Tiles and Classification Queries
    map.setTile(TileCoord { 2U, 3U }, TileType::SolidWall);
    TEST_ASSERT(map.getTile(TileCoord { 2U, 3U }) == TileType::SolidWall, "Cell (2,3) must be SolidWall");
    TEST_ASSERT(map.isSolid(TileCoord { 2U, 3U }), "SolidWall must be solid");
    TEST_ASSERT(!map.isDestructible(TileCoord { 2U, 3U }), "SolidWall must not be destructible");
    TEST_ASSERT(!map.isHazard(TileCoord { 2U, 3U }), "SolidWall must not be hazard");

    map.setTile(TileCoord { 4U, 4U }, TileType::Hazard);
    TEST_ASSERT(map.isHazard(TileCoord { 4U, 4U }), "Hazard tile must be recognized as hazard");
    TEST_ASSERT(!map.isSolid(TileCoord { 4U, 4U }), "Hazard cell is traversable/non-solid");

    // Test 4: Destructible Wall Durability and Crumbing
    map.setTile(TileCoord { 5U, 5U }, TileType::DestructibleWall);
    TEST_ASSERT(map.isDestructible(TileCoord { 5U, 5U }), "Tile (5,5) must be DestructibleWall");
    TEST_ASSERT(map.isSolid(TileCoord { 5U, 5U }), "DestructibleWall is solid obstacle");
    TEST_ASSERT(map.getHealth(TileCoord { 5U, 5U }).value() == TileHealth::kDefaultMax, "Initial health must be 3");

    // First impact
    bool destroyed = map.damageTile(TileCoord { 5U, 5U });
    TEST_ASSERT(!destroyed, "First hit must not destroy block");
    TEST_ASSERT(map.getHealth(TileCoord { 5U, 5U }).value() == 2U, "Health must decrease to 2");

    // Second impact
    destroyed = map.damageTile(TileCoord { 5U, 5U });
    TEST_ASSERT(!destroyed, "Second hit must not destroy block");
    TEST_ASSERT(map.getHealth(TileCoord { 5U, 5U }).value() == 1U, "Health must decrease to 1");

    // Third impact (final destruction)
    destroyed = map.damageTile(TileCoord { 5U, 5U });
    TEST_ASSERT(destroyed, "Third hit must destroy block");
    TEST_ASSERT(map.getTile(TileCoord { 5U, 5U }) == TileType::Empty, "Destroyed block must become Empty");
    TEST_ASSERT(!map.isSolid(TileCoord { 5U, 5U }), "Destroyed block must no longer be solid");

    // Test 5: Immediate Tile Destruction
    map.setTile(TileCoord { 6U, 6U }, TileType::DestructibleWall);
    TEST_ASSERT(map.destroyTile(TileCoord { 6U, 6U }), "destroyTile must return true for destructible");
    TEST_ASSERT(map.getTile(TileCoord { 6U, 6U }) == TileType::Empty, "destroyTile must clear cell immediately");
    TEST_ASSERT(!map.destroyTile(TileCoord { 6U, 6U }), "destroyTile on empty cell must return false");

    // Test 6: Tile Hitbox Calculation
    const Hitbox tileBox { map.getTileBox(TileCoord { 2U, 3U }) };
    TEST_ASSERT(tileBox.min().x == 32.0f && tileBox.min().y == 48.0f, "Tile (2,3) min box must be (32, 48)");
    TEST_ASSERT(tileBox.max().x == 48.0f && tileBox.max().y == 64.0f, "Tile (2,3) max box must be (48, 64)");

    // Test 7: Tunnel Clearance Checks (Robot 16x32 vs Jet 32x16)
    // Setup a 1-tile high tunnel: row 0 = Solid, row 1 = Empty, row 2 = Solid
    TileMap tunnelMap { 8U, 4U, 16.0f };
    for (std::uint32_t c { 0U }; c < 8U; ++c) {
        tunnelMap.setTile(TileCoord { c, 0U }, TileType::SolidWall); // Ceiling
        tunnelMap.setTile(TileCoord { c, 2U }, TileType::SolidWall); // Floor
    }

    // Jet craft: Width = 32px (2 tiles), Height = 16px (1 tile), at pos (16.0, 16.0) inside tunnel
    const Hitbox jetBox { Hitbox::fromDims(WorldPos { 16.0f, 16.0f }, 32.0f, 16.0f) };
    TEST_ASSERT(tunnelMap.hasClearance(jetBox), "Jet craft must have clearance inside 1-tile tunnel");

    // Robot craft: Width = 16px (1 tile), Height = 32px (2 tiles), at pos (16.0, 16.0)
    // Overlaps row 2 (floor)!
    const Hitbox robotBoxOverlapFloor { Hitbox::fromDims(WorldPos { 16.0f, 16.0f }, 16.0f, 32.0f) };
    TEST_ASSERT(!tunnelMap.hasClearance(robotBoxOverlapFloor), "Robot craft must fail clearance due to floor");

    // Robot craft attempting to fit in 1-tile space at pos (16.0, 0.0)
    // Overlaps row 0 (ceiling) and row 1
    const Hitbox robotBoxOverlapCeiling { Hitbox::fromDims(WorldPos { 16.0f, 0.0f }, 16.0f, 32.0f) };
    TEST_ASSERT(!tunnelMap.hasClearance(robotBoxOverlapCeiling), "Robot craft must fail clearance due to ceiling");
}

} // namespace ShooterCore::Test
