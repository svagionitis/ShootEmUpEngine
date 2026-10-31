#include "Level1Map.h"

#include <cstddef>
#include <cstdint>

#include "TileMap.h"
#include "Types.h"

namespace ThexderGame {

void Level1Map::build(ShooterCore::TileMap& outMap) noexcept
{
    for (std::uint32_t r { 0U }; r < outMap.height(); ++r) {
        for (std::uint32_t c { 0U }; c < outMap.width(); ++c) {
            outMap.setTile(ShooterCore::TileCoord { c, r }, ShooterCore::TileType::Empty);
        }
    }

    generatePerimeter(outMap);
    generatePlatforms(outMap);
    generateTunnels(outMap);
    generateBarricades(outMap);
    generateHazards(outMap);
}

void Level1Map::generatePerimeter(ShooterCore::TileMap& map) noexcept
{
    for (std::uint32_t c { 0U }; c < kMapWidth; ++c) {
        map.setTile(ShooterCore::TileCoord { c, 0U }, ShooterCore::TileType::SolidWall);
        map.setTile(ShooterCore::TileCoord { c, kMapHeight - 1U }, ShooterCore::TileType::SolidWall);
    }

    for (std::uint32_t r { 0U }; r < kMapHeight; ++r) {
        map.setTile(ShooterCore::TileCoord { 0U, r }, ShooterCore::TileType::SolidWall);
        map.setTile(ShooterCore::TileCoord { kMapWidth - 1U, r }, ShooterCore::TileType::SolidWall);
    }
}

void Level1Map::generatePlatforms(ShooterCore::TileMap& map) noexcept
{
    // Zone 1: Staging upper platform (Cols 1..18, Row 7)
    for (std::uint32_t c { 1U }; c <= 18U; ++c) {
        map.setTile(ShooterCore::TileCoord { c, 7U }, ShooterCore::TileType::SolidWall);
    }

    // Divider wall between spawn terrace and inner cavern (Cols 18, Rows 1..6)
    // with opening at Row 5..6 so player can drop into drop shaft at Col 19..22
    for (std::uint32_t r { 1U }; r <= 4U; ++r) {
        map.setTile(ShooterCore::TileCoord { 18U, r }, ShooterCore::TileType::SolidWall);
    }

    // Lower terrace under drop shaft (Cols 1..35, Row 24)
    for (std::uint32_t c { 1U }; c <= 35U; ++c) {
        map.setTile(ShooterCore::TileCoord { c, 24U }, ShooterCore::TileType::SolidWall);
    }

    // Central cavern terraces (Cols 66..88, Rows 18 and 24)
    for (std::uint32_t c { 66U }; c <= 88U; ++c) {
        map.setTile(ShooterCore::TileCoord { c, 18U }, ShooterCore::TileType::SolidWall);
        map.setTile(ShooterCore::TileCoord { c, 25U }, ShooterCore::TileType::SolidWall);
    }

    // Terminal gate platform (Cols 140..158, Row 29)
    for (std::uint32_t c { 140U }; c <= 158U; ++c) {
        map.setTile(ShooterCore::TileCoord { c, 29U }, ShooterCore::TileType::SolidWall);
    }
}

void Level1Map::generateTunnels(ShooterCore::TileMap& map) noexcept
{
    // Tunnel 1: Cols 35..65 at Row 16 (Height = 1 tile)
    // Solid ceiling at Row 15, floor at Row 17
    for (std::uint32_t c { 35U }; c <= 65U; ++c) {
        map.setTile(ShooterCore::TileCoord { c, 15U }, ShooterCore::TileType::SolidWall);
        map.setTile(ShooterCore::TileCoord { c, 16U }, ShooterCore::TileType::Empty);
        map.setTile(ShooterCore::TileCoord { c, 17U }, ShooterCore::TileType::SolidWall);
    }

    // Enclosing bulkheads at tunnel 1 entrance and exit
    for (std::uint32_t r { 8U }; r <= 14U; ++r) {
        map.setTile(ShooterCore::TileCoord { 34U, r }, ShooterCore::TileType::SolidWall);
        map.setTile(ShooterCore::TileCoord { 66U, r }, ShooterCore::TileType::SolidWall);
    }
    for (std::uint32_t r { 18U }; r <= 23U; ++r) {
        map.setTile(ShooterCore::TileCoord { 35U, r }, ShooterCore::TileType::SolidWall);
    }

    // Tunnel 2: Cols 90..115 at Row 10 (Height = 1 tile)
    // Solid ceiling at Row 9, floor at Row 11
    for (std::uint32_t c { 90U }; c <= 115U; ++c) {
        map.setTile(ShooterCore::TileCoord { c, 9U }, ShooterCore::TileType::SolidWall);
        map.setTile(ShooterCore::TileCoord { c, 10U }, ShooterCore::TileType::Empty);
        map.setTile(ShooterCore::TileCoord { c, 11U }, ShooterCore::TileType::SolidWall);
    }
    for (std::uint32_t r { 1U }; r <= 8U; ++r) {
        map.setTile(ShooterCore::TileCoord { 89U, r }, ShooterCore::TileType::SolidWall);
        map.setTile(ShooterCore::TileCoord { 116U, r }, ShooterCore::TileType::SolidWall);
    }
}

void Level1Map::generateBarricades(ShooterCore::TileMap& map) noexcept
{
    // Destructible block barrier 1: Cols 26..28, Rows 18..23
    for (std::uint32_t c { 26U }; c <= 28U; ++c) {
        for (std::uint32_t r { 18U }; r <= 23U; ++r) {
            map.setTile(ShooterCore::TileCoord { c, r }, ShooterCore::TileType::DestructibleWall);
        }
    }

    // Destructible block barrier 2: Cols 75..78, Rows 19..24
    for (std::uint32_t c { 75U }; c <= 78U; ++c) {
        for (std::uint32_t r { 19U }; r <= 24U; ++r) {
            map.setTile(ShooterCore::TileCoord { c, r }, ShooterCore::TileType::DestructibleWall);
        }
    }

    // Destructible block barrier 3: Cols 125..127, Rows 15..21
    for (std::uint32_t c { 125U }; c <= 127U; ++c) {
        for (std::uint32_t r { 15U }; r <= 21U; ++r) {
            map.setTile(ShooterCore::TileCoord { c, r }, ShooterCore::TileType::DestructibleWall);
        }
    }

    // Destructible block barrier 4: Cols 138..139, Rows 25..28 (Guards terminal gate)
    for (std::uint32_t c { 138U }; c <= 139U; ++c) {
        for (std::uint32_t r { 25U }; r <= 28U; ++r) {
            map.setTile(ShooterCore::TileCoord { c, r }, ShooterCore::TileType::DestructibleWall);
        }
    }
}

void Level1Map::generateHazards(ShooterCore::TileMap& map) noexcept
{
    // Geothermal lava pool 1: Cols 40..60 along bottom floor (Row 30)
    for (std::uint32_t c { 40U }; c <= 60U; ++c) {
        map.setTile(ShooterCore::TileCoord { c, 30U }, ShooterCore::TileType::Hazard);
    }

    // Geothermal lava pool 2: Cols 95..120 along bottom floor (Row 30)
    for (std::uint32_t c { 95U }; c <= 120U; ++c) {
        map.setTile(ShooterCore::TileCoord { c, 30U }, ShooterCore::TileType::Hazard);
    }

    // Thermal energy conduit 3: Cols 128..135 along bottom floor (Row 30)
    for (std::uint32_t c { 128U }; c <= 135U; ++c) {
        map.setTile(ShooterCore::TileCoord { c, 30U }, ShooterCore::TileType::Hazard);
    }
}

} // namespace ThexderGame
