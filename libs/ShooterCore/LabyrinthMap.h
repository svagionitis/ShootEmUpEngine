#pragma once

#include <cstddef>
#include <cstdint>

#include "Hitbox.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore {

/// @class LabyrinthMap
/// @brief Builder and spatial query provider for the 256x128 multi-sector labyrinth.
/// @details Constructs four tactical sectors with 1-tile crawling conduits, vertical descent
///          shafts, security generator vaults, secret masonry, and boss arena enclosure.
class LabyrinthMap final {
public:
    static constexpr std::uint32_t kMapWidth { 256U };
    static constexpr std::uint32_t kMapHeight { 128U };
    static constexpr float kTileSize { 16.0f };

    LabyrinthMap() noexcept = default;

    /// @brief Populate target TileMap with the sprawling 256x128 labyrinth layout.
    /// @param[out] outMap Pre-dimensioned tilemap to fill.
    static void build(TileMap& outMap) noexcept;

    /// @brief Query player spawn world coordinates in Sector Alpha.
    /// @return Continuous world coordinates for initial spawn.
    [[nodiscard]] static constexpr WorldPos getPlayerSpawn() noexcept
    {
        return WorldPos { 4.0f * kTileSize + 8.0f, 6.0f * kTileSize };
    }

    /// @brief Query stage completion exit warp portal world coordinates in Sector Delta.
    /// @return Continuous world coordinates of exit portal.
    [[nodiscard]] static constexpr WorldPos getExitPortal() noexcept
    {
        return WorldPos { 235.0f * kTileSize, 95.0f * kTileSize };
    }

    /// @brief Retrieve world bounding box for an individual sector.
    /// @param[in] id Sector enumeration identifier.
    /// @return WorldPos bounding rectangle for the sector.
    [[nodiscard]] static constexpr SectorBounds getSectorBounds(SectorId id) noexcept
    {
        switch (id) {
        case SectorId::Alpha:
            return SectorBounds { WorldPos { 0.0f, 0.0f }, WorldPos { 128.0f * kTileSize, 64.0f * kTileSize } };
        case SectorId::Beta:
            return SectorBounds { WorldPos { 128.0f * kTileSize, 0.0f },
                WorldPos { 256.0f * kTileSize, 64.0f * kTileSize } };
        case SectorId::Gamma:
            return SectorBounds { WorldPos { 0.0f, 64.0f * kTileSize },
                WorldPos { 128.0f * kTileSize, 128.0f * kTileSize } };
        case SectorId::Delta:
            return SectorBounds { WorldPos { 128.0f * kTileSize, 64.0f * kTileSize },
                WorldPos { 256.0f * kTileSize, 128.0f * kTileSize } };
        case SectorId::Count:
        default:
            return SectorBounds { WorldPos { 0.0f, 0.0f }, WorldPos { 256.0f * kTileSize, 128.0f * kTileSize } };
        }
    }

    /// @brief Retrieve world bounding box for the enclosed boss chamber inside Sector Delta.
    /// @return WorldPos bounding rectangle for the boss arena.
    [[nodiscard]] static constexpr SectorBounds getBossArenaBounds() noexcept
    {
        return SectorBounds { WorldPos { 190.0f * kTileSize, 78.0f * kTileSize },
            WorldPos { 245.0f * kTileSize, 115.0f * kTileSize } };
    }

    /// @brief Query which sector contains the given world coordinate.
    /// @param[in] pos Continuous world position.
    /// @return SectorId containing the position.
    [[nodiscard]] static SectorId getSectorAt(WorldPos pos) noexcept;

private:
    static void generatePerimeter(TileMap& map) noexcept;
    static void generateSectorAlpha(TileMap& map) noexcept;
    static void generateSectorBeta(TileMap& map) noexcept;
    static void generateSectorGamma(TileMap& map) noexcept;
    static void generateSectorDelta(TileMap& map) noexcept;
};

} // namespace ShooterCore
