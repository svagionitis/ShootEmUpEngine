#include "TestHarness.h"

#include <cstdint>

#include "Hitbox.h"
#include "Level1Map.h"
#include "TileMap.h"
#include "Types.h"

namespace ThexderGame::Test {

void runLevel1MapTests()
{
    LOG(INFO) << "Executing Level1Map Unit & Clearance Tests...";

    ShooterCore::TileMap map { Level1Map::kMapWidth, Level1Map::kMapHeight };
    Level1Map::build(map);

    const auto assertTile = [](ShooterCore::TileType actual, ShooterCore::TileType expected, const char* msg) {
        TEST_ASSERT_EQ(static_cast<std::uint8_t>(actual), static_cast<std::uint8_t>(expected), msg);
    };

    // 1. Dimensions
    TEST_ASSERT_EQ(map.width(), Level1Map::kMapWidth, "Map width must match 160 tiles");
    TEST_ASSERT_EQ(map.height(), Level1Map::kMapHeight, "Map height must match 32 tiles");

    // 2. Perimeter boundaries
    assertTile(map.getTile(ShooterCore::TileCoord { 0U, 0U }), ShooterCore::TileType::SolidWall,
        "Top-left corner must be solid");
    assertTile(map.getTile(ShooterCore::TileCoord { 159U, 0U }), ShooterCore::TileType::SolidWall,
        "Top-right corner must be solid");
    assertTile(map.getTile(ShooterCore::TileCoord { 0U, 31U }), ShooterCore::TileType::SolidWall,
        "Bottom-left corner must be solid");
    assertTile(map.getTile(ShooterCore::TileCoord { 159U, 31U }), ShooterCore::TileType::SolidWall,
        "Bottom-right corner must be solid");

    // 3. 1-Tile Tunnel Clearance Invariant
    // In Tunnel 1 (Cols 35..65, Row 16):
    for (std::uint32_t c { 36U }; c <= 64U; ++c) {
        assertTile(map.getTile(ShooterCore::TileCoord { c, 15U }), ShooterCore::TileType::SolidWall,
            "Tunnel ceiling must be solid");
        assertTile(map.getTile(ShooterCore::TileCoord { c, 16U }), ShooterCore::TileType::Empty,
            "Tunnel crawlspace must be empty (1 tile high)");
        assertTile(map.getTile(ShooterCore::TileCoord { c, 17U }), ShooterCore::TileType::SolidWall,
            "Tunnel floor must be solid");
    }

    // 4. Hitbox Clearance Verification: Jet fits, Robot does not
    const float testX { 40.0f * Level1Map::kTileSize };
    const float floorY { 17.0f * Level1Map::kTileSize };

    // Jet is 16 px high: bounds = [floorY - 16, floorY] = [row 16 top, row 16 bottom]
    const ShooterCore::Hitbox jetBox { ShooterCore::WorldPos { testX, floorY - 16.0f },
        ShooterCore::WorldPos { testX + 32.0f, floorY } };

    // Robot is 32 px high: bounds = [floorY - 32, floorY] = [row 15 top, row 16 bottom]
    const ShooterCore::Hitbox robotBox { ShooterCore::WorldPos { testX, floorY - 32.0f },
        ShooterCore::WorldPos { testX + 16.0f, floorY } };

    // Verify ceiling tile intersection at row 15
    const ShooterCore::Hitbox ceilingTile { ShooterCore::WorldPos { testX, 15.0f * Level1Map::kTileSize },
        ShooterCore::WorldPos { testX + 16.0f, 16.0f * Level1Map::kTileSize } };

    TEST_ASSERT(!jetBox.intersects(ceilingTile), "Jet mode must not intersect 1-tile tunnel ceiling");
    TEST_ASSERT(robotBox.intersects(ceilingTile), "Robot mode must intersect 1-tile tunnel ceiling (blocked)");

    // 5. Destructible barricades
    assertTile(map.getTile(ShooterCore::TileCoord { 26U, 20U }), ShooterCore::TileType::DestructibleWall,
        "Barricade 1 must have destructible walls");
    assertTile(map.getTile(ShooterCore::TileCoord { 76U, 20U }), ShooterCore::TileType::DestructibleWall,
        "Barricade 2 must have destructible walls");
    assertTile(map.getTile(ShooterCore::TileCoord { 126U, 18U }), ShooterCore::TileType::DestructibleWall,
        "Barricade 3 must have destructible walls");

    // 6. Hazard pools
    assertTile(
        map.getTile(ShooterCore::TileCoord { 45U, 30U }), ShooterCore::TileType::Hazard, "Lava pit 1 must be hazard");
    assertTile(
        map.getTile(ShooterCore::TileCoord { 100U, 30U }), ShooterCore::TileType::Hazard, "Lava pit 2 must be hazard");

    // 7. Spawn position validity
    const ShooterCore::WorldPos spawn { Level1Map::getPlayerSpawn() };
    const ShooterCore::TileCoord spawnTile { spawn.toTileCoord(Level1Map::kTileSize) };
    TEST_ASSERT_EQ(spawnTile.col, 4U, "Spawn column should be 4");
    TEST_ASSERT_EQ(spawnTile.row, 6U, "Spawn row should be 6");

    // Floor under spawn (Row 7) must be solid
    assertTile(map.getTile(ShooterCore::TileCoord { spawnTile.col, 7U }), ShooterCore::TileType::SolidWall,
        "Staging terrace under player spawn must be solid");

    // Headroom at spawn (Rows 5 and 6) must be clear for 2-tile Robot
    assertTile(map.getTile(ShooterCore::TileCoord { spawnTile.col, 5U }), ShooterCore::TileType::Empty,
        "Headroom row 5 at spawn must be empty");
    assertTile(map.getTile(ShooterCore::TileCoord { spawnTile.col, 6U }), ShooterCore::TileType::Empty,
        "Headroom row 6 at spawn must be empty");

    // 8. Exit portal bounds
    const ShooterCore::WorldPos exitPortal { Level1Map::getExitPortal() };
    TEST_ASSERT(exitPortal.x > 0.0f && exitPortal.x < 160.0f * 16.0f, "Exit portal X must be within map bounds");
    TEST_ASSERT(exitPortal.y > 0.0f && exitPortal.y < 32.0f * 16.0f, "Exit portal Y must be within map bounds");
}

} // namespace ThexderGame::Test
