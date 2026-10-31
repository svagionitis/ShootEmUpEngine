#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "TileMap.h"
#include "Types.h"

namespace ThexderGame {

/// @class Level1Map
/// @brief Builder and geometry provider for Thexder Level 1 cavernous labyrinth.
/// @details Populates TileMap with authentic Thexder caverns from MAP01.BIN/EGAPTR01.BIN
///          or procedural layout, 1-tile jet passages, and destructible barriers.
class Level1Map final {
public:
    static constexpr std::uint32_t kMapWidth { 160U };
    static constexpr std::uint32_t kAuthenticMapWidth { 320U };
    static constexpr std::uint32_t kMapHeight { 32U };
    static constexpr float kTileSize { 16.0f };

    Level1Map() noexcept = default;

    /// @brief Populate target TileMap with procedural Level 1 layout geometry.
    /// @param[out] outMap Pre-dimensioned tilemap to fill.
    static void build(ShooterCore::TileMap& outMap) noexcept;

    /// @brief Populate target TileMap from authentic DOS binary map files.
    /// @param[out] outMap Pre-dimensioned tilemap to fill.
    /// @param[in] ptrPath Path to EGAPTR01.BIN.
    /// @param[in] mapPath Path to MAP01.BIN.
    /// @return true if loaded and decoded cleanly; false if fallback was used.
    static bool loadFromBinary(ShooterCore::TileMap& outMap, const std::string& ptrPath = "assets/thexder/EGAPTR01.BIN",
        const std::string& mapPath = "assets/thexder/MAP01.BIN") noexcept;

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
