#include "LabyrinthMap.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>

#include "Hitbox.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore {

SectorId LabyrinthMap::getSectorAt(WorldPos pos) noexcept
{
    const float midX { 128.0f * kTileSize };
    const float midY { 64.0f * kTileSize };

    if (pos.x < midX) {
        if (pos.y < midY) {
            return SectorId::Alpha;
        }
        return SectorId::Gamma;
    }

    if (pos.y < midY) {
        return SectorId::Beta;
    }
    return SectorId::Delta;
}

void LabyrinthMap::build(TileMap& outMap) noexcept
{
    generatePerimeter(outMap);
    generateSectorAlpha(outMap);
    generateSectorBeta(outMap);
    generateSectorGamma(outMap);
    generateSectorDelta(outMap);
}

void LabyrinthMap::generatePerimeter(TileMap& map) noexcept
{
    for (std::uint32_t c { 0U }; c < kMapWidth; ++c) {
        map.setTile(TileCoord { c, 0U }, TileType::SolidWall);
        map.setTile(TileCoord { c, 1U }, TileType::SolidWall);
        map.setTile(TileCoord { c, kMapHeight - 2U }, TileType::SolidWall);
        map.setTile(TileCoord { c, kMapHeight - 1U }, TileType::SolidWall);
    }

    for (std::uint32_t r { 0U }; r < kMapHeight; ++r) {
        map.setTile(TileCoord { 0U, r }, TileType::SolidWall);
        map.setTile(TileCoord { 1U, r }, TileType::SolidWall);
        map.setTile(TileCoord { kMapWidth - 2U, r }, TileType::SolidWall);
        map.setTile(TileCoord { kMapWidth - 1U, r }, TileType::SolidWall);
    }
}

void LabyrinthMap::generateSectorAlpha(TileMap& map) noexcept
{
    // Horizontal platforms & cavern terraces
    for (std::uint32_t c { 2U }; c < 60U; ++c) {
        map.setTile(TileCoord { c, 10U }, TileType::SolidWall);
        map.setTile(TileCoord { c, 22U }, TileType::SolidWall);
    }

    for (std::uint32_t c { 40U }; c < 120U; ++c) {
        map.setTile(TileCoord { c, 35U }, TileType::SolidWall);
        map.setTile(TileCoord { c, 50U }, TileType::SolidWall);
    }

    // Vertical descent shaft dividers
    for (std::uint32_t r { 10U }; r < 55U; ++r) {
        map.setTile(TileCoord { 30U, r }, TileType::SolidWall);
        map.setTile(TileCoord { 80U, r }, TileType::SolidWall);
    }

    // Openings in vertical shafts
    for (std::uint32_t r { 14U }; r < 18U; ++r) {
        map.setTile(TileCoord { 30U, r }, TileType::Empty);
    }
    for (std::uint32_t r { 40U }; r < 44U; ++r) {
        map.setTile(TileCoord { 80U, r }, TileType::Empty);
    }

    // Secret masonry wall concealing hidden chamber (Energy Tank 1)
    map.setTile(TileCoord { 20U, 18U }, TileType::SecretWall);
    map.setTile(TileCoord { 20U, 19U }, TileType::SecretWall);
    map.setTile(TileCoord { 20U, 20U }, TileType::SecretWall);
    map.setTile(TileCoord { 20U, 21U }, TileType::SecretWall);

    // Hazard spikes in pit
    for (std::uint32_t c { 60U }; c < 75U; ++c) {
        map.setTile(TileCoord { c, 62U }, TileType::Hazard);
    }
}

void LabyrinthMap::generateSectorBeta(TileMap& map) noexcept
{
    // Horizontal divider separating upper and lower flues
    for (std::uint32_t c { 128U }; c < 250U; ++c) {
        map.setTile(TileCoord { c, 18U }, TileType::SolidWall);
        map.setTile(TileCoord { c, 20U }, TileType::SolidWall); // row 19 is 1-tile crawling flue!

        map.setTile(TileCoord { c, 32U }, TileType::SolidWall);
        map.setTile(TileCoord { c, 34U }, TileType::SolidWall); // row 33 is 1-tile crawling flue!
    }

    // Row 19 and Row 33 are strictly empty 1-tile conduits (16px height)
    for (std::uint32_t c { 135U }; c < 240U; ++c) {
        map.setTile(TileCoord { c, 19U }, TileType::Empty);
        map.setTile(TileCoord { c, 33U }, TileType::Empty);
    }

    // Hazard ceiling spikes
    for (std::uint32_t c { 150U }; c < 170U; ++c) {
        map.setTile(TileCoord { c, 17U }, TileType::Hazard);
    }
}

void LabyrinthMap::generateSectorGamma(TileMap& map) noexcept
{
    // Security Complex bulkheads & generator vaults
    for (std::uint32_t c { 10U }; c < 120U; ++c) {
        map.setTile(TileCoord { c, 75U }, TileType::SolidWall);
        map.setTile(TileCoord { c, 95U }, TileType::SolidWall);
        map.setTile(TileCoord { c, 115U }, TileType::SolidWall);
    }

    // Generator vault columns
    for (std::uint32_t r { 75U }; r < 115U; ++r) {
        map.setTile(TileCoord { 35U, r }, TileType::SolidWall);
        map.setTile(TileCoord { 70U, r }, TileType::SolidWall);
        map.setTile(TileCoord { 105U, r }, TileType::SolidWall);
    }

    // Access corridors
    for (std::uint32_t r { 82U }; r < 86U; ++r) {
        map.setTile(TileCoord { 35U, r }, TileType::Empty);
        map.setTile(TileCoord { 70U, r }, TileType::Empty);
    }

    // Secret Room (Energy Tank 2) behind destructible masonry
    map.setTile(TileCoord { 15U, 100U }, TileType::SecretWall);
    map.setTile(TileCoord { 15U, 101U }, TileType::SecretWall);

    // Forcefield security barrier between Gamma and Delta
    for (std::uint32_t r { 88U }; r < 94U; ++r) {
        map.setTile(TileCoord { 127U, r }, TileType::Forcefield);
    }
}

void LabyrinthMap::generateSectorDelta(TileMap& map) noexcept
{
    // Sector Delta: Boss Arena enclosure walls (col 190 to 245, row 78 to 115)
    for (std::uint32_t c { 190U }; c <= 245U; ++c) {
        map.setTile(TileCoord { c, 78U }, TileType::SolidWall);
        map.setTile(TileCoord { c, 115U }, TileType::SolidWall);
    }

    for (std::uint32_t r { 78U }; r <= 115U; ++r) {
        map.setTile(TileCoord { 190U, r }, TileType::SolidWall);
        map.setTile(TileCoord { 245U, r }, TileType::SolidWall);
    }

    // Entry gateway into boss arena (col 190, rows 93-97)
    for (std::uint32_t r { 93U }; r <= 97U; ++r) {
        map.setTile(TileCoord { 190U, r }, TileType::Empty);
    }
}

} // namespace ShooterCore
