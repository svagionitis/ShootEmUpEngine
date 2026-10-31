#include "Level1Map.h"

#include <cstddef>
#include <cstdint>
#include <string>

#include "EgaMapLoader.h"
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

bool Level1Map::loadFromBinary(
    ShooterCore::TileMap& outMap, const std::string& ptrPath, const std::string& mapPath) noexcept
{
    const bool success { SdlAdapter::EgaMapLoader::loadMap(ptrPath, mapPath, outMap) };
    if (!success) {
        build(outMap);
        return false;
    }
    return true;
}

void Level1Map::generatePerimeter(ShooterCore::TileMap& map) noexcept
{
    const std::uint32_t mapW { map.width() };
    const std::uint32_t mapH { map.height() };

    for (std::uint32_t c { 0U }; c < mapW; ++c) {
        map.setTile(ShooterCore::TileCoord { c, 0U }, ShooterCore::TileType::SolidWall);
        map.setTile(ShooterCore::TileCoord { c, mapH - 1U }, ShooterCore::TileType::SolidWall);
    }

    for (std::uint32_t r { 0U }; r < mapH; ++r) {
        map.setTile(ShooterCore::TileCoord { 0U, r }, ShooterCore::TileType::SolidWall);
        map.setTile(ShooterCore::TileCoord { mapW - 1U, r }, ShooterCore::TileType::SolidWall);
    }
}

void Level1Map::generatePlatforms(ShooterCore::TileMap& map) noexcept
{
    for (std::uint32_t c { 1U }; c <= 18U && c < map.width(); ++c) {
        map.setTile(ShooterCore::TileCoord { c, 7U }, ShooterCore::TileType::SolidWall);
    }

    for (std::uint32_t r { 1U }; r <= 4U && r < map.height(); ++r) {
        map.setTile(ShooterCore::TileCoord { 18U, r }, ShooterCore::TileType::SolidWall);
    }

    for (std::uint32_t c { 1U }; c <= 35U && c < map.width(); ++c) {
        map.setTile(ShooterCore::TileCoord { c, 24U }, ShooterCore::TileType::SolidWall);
    }

    for (std::uint32_t c { 66U }; c <= 88U && c < map.width(); ++c) {
        map.setTile(ShooterCore::TileCoord { c, 18U }, ShooterCore::TileType::SolidWall);
        map.setTile(ShooterCore::TileCoord { c, 25U }, ShooterCore::TileType::SolidWall);
    }

    for (std::uint32_t c { 140U }; c <= 158U && c < map.width(); ++c) {
        map.setTile(ShooterCore::TileCoord { c, 29U }, ShooterCore::TileType::SolidWall);
    }
}

void Level1Map::generateTunnels(ShooterCore::TileMap& map) noexcept
{
    for (std::uint32_t c { 35U }; c <= 65U && c < map.width(); ++c) {
        map.setTile(ShooterCore::TileCoord { c, 15U }, ShooterCore::TileType::SolidWall);
        map.setTile(ShooterCore::TileCoord { c, 16U }, ShooterCore::TileType::Empty);
        map.setTile(ShooterCore::TileCoord { c, 17U }, ShooterCore::TileType::SolidWall);
    }

    for (std::uint32_t r { 8U }; r <= 14U && r < map.height(); ++r) {
        map.setTile(ShooterCore::TileCoord { 34U, r }, ShooterCore::TileType::SolidWall);
        map.setTile(ShooterCore::TileCoord { 66U, r }, ShooterCore::TileType::SolidWall);
    }
    for (std::uint32_t r { 18U }; r <= 23U && r < map.height(); ++r) {
        map.setTile(ShooterCore::TileCoord { 35U, r }, ShooterCore::TileType::SolidWall);
    }

    for (std::uint32_t c { 90U }; c <= 115U && c < map.width(); ++c) {
        map.setTile(ShooterCore::TileCoord { c, 9U }, ShooterCore::TileType::SolidWall);
        map.setTile(ShooterCore::TileCoord { c, 10U }, ShooterCore::TileType::Empty);
        map.setTile(ShooterCore::TileCoord { c, 11U }, ShooterCore::TileType::SolidWall);
    }
    for (std::uint32_t r { 1U }; r <= 8U && r < map.height(); ++r) {
        map.setTile(ShooterCore::TileCoord { 89U, r }, ShooterCore::TileType::SolidWall);
        map.setTile(ShooterCore::TileCoord { 116U, r }, ShooterCore::TileType::SolidWall);
    }
}

void Level1Map::generateBarricades(ShooterCore::TileMap& map) noexcept
{
    const std::uint32_t barricadeCols[] { 26U, 76U, 126U };
    const std::uint32_t barricadeRows[] { 20U, 20U, 18U };

    for (std::size_t i { 0U }; i < 3U; ++i) {
        const std::uint32_t c { barricadeCols[i] };
        const std::uint32_t baseR { barricadeRows[i] };
        for (std::uint32_t r { baseR }; r < baseR + 3U && r < map.height(); ++r) {
            if (c < map.width()) {
                map.setTile(ShooterCore::TileCoord { c, r }, ShooterCore::TileType::DestructibleWall);
            }
        }
    }
}

void Level1Map::generateHazards(ShooterCore::TileMap& map) noexcept
{
    for (std::uint32_t c { 45U }; c <= 55U && c < map.width(); ++c) {
        map.setTile(ShooterCore::TileCoord { c, 30U }, ShooterCore::TileType::Hazard);
    }

    for (std::uint32_t c { 100U }; c <= 110U && c < map.width(); ++c) {
        map.setTile(ShooterCore::TileCoord { c, 30U }, ShooterCore::TileType::Hazard);
    }
}

} // namespace ThexderGame
