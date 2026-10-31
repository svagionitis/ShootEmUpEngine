#pragma once

#include <cstddef>
#include <cstdint>

#include "TileMap.h"
#include "Types.h"

namespace ThexderGame {

/// @class Level1Map
/// @brief Builder and geometry provider for Thexder Level 1 cavernous labyrinth.
/// @details Populates a discrete 160x32 TileMap with authentic Thexder caverns,
///          1-tile jet passages, destructible barriers, and hazard lava beds.
class Level1Map final {
public:
    static constexpr std::uint32_t kMapWidth { 160U };
    static constexpr std::uint32_t kMapHeight { 32U };
    static constexpr float kTileSize { 16.0f };

    Level1Map() noexcept = default;

    /// @brief Populate target TileMap with Level 1 layout geometry.
    /// @param[out] outMap Pre-dimensioned tilemap to fill.
    static void build(ShooterCore::TileMap& outMap) noexcept;

    /// @brief Query designated initial player spawn world coordinates.
    /// @return Continuous world position (bottom-center aligned).
    [[nodiscard]] static constexpr ShooterCore::WorldPos getPlayerSpawn() noexcept
    {
        return ShooterCore::WorldPos { 4.0f * kTileSize + 8.0f, 6.0f * kTileSize };
    }

    /// @brief Query level completion exit portal world coordinates.
    /// @return Continuous world position of exit trigger zone.
    [[nodiscard]] static constexpr ShooterCore::WorldPos getExitPortal() noexcept
    {
        return ShooterCore::WorldPos { 154.0f * kTileSize, 28.0f * kTileSize };
    }

private:
    static void generatePerimeter(ShooterCore::TileMap& map) noexcept;
    static void generatePlatforms(ShooterCore::TileMap& map) noexcept;
    static void generateTunnels(ShooterCore::TileMap& map) noexcept;
    static void generateBarricades(ShooterCore::TileMap& map) noexcept;
    static void generateHazards(ShooterCore::TileMap& map) noexcept;
};

} // namespace ThexderGame
