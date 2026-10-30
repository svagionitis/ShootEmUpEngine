#pragma once

#include <cstddef>
#include <cstdint>

#include <vector>

#include "Hitbox.h"
#include "Types.h"

namespace ShooterCore {

/// @class TileMap
/// @brief 2D discrete spatial grid managing static terrain, destructible blocks, and hazards.
/// @details Operates with zero dynamic memory allocation during real-time queries and updates.
class TileMap final {
public:
    static constexpr float kDefaultTileSize { 16.0f };

    /// @brief Construct empty tilemap with dimensions and tile extent.
    /// @param[in] width Horizontal column count.
    /// @param[in] height Vertical row count.
    /// @param[in] tileSize Dimension in world units for each square cell.
    TileMap(std::uint32_t width, std::uint32_t height, float tileSize = kDefaultTileSize);

    /// @brief Get horizontal grid cell count.
    /// @return Number of columns.
    [[nodiscard]] std::uint32_t width() const noexcept
    {
        return width_;
    }

    /// @brief Get vertical grid cell count.
    /// @return Number of rows.
    [[nodiscard]] std::uint32_t height() const noexcept
    {
        return height_;
    }

    /// @brief Get dimension of square cell in world units.
    /// @return Side length in world units.
    [[nodiscard]] float tileSize() const noexcept
    {
        return tileSize_;
    }

    /// @brief Retrieve cell type at discrete coordinate.
    /// @param[in] coord Discrete grid column and row.
    /// @return Tile material; returns TileType::SolidWall if coordinate is out-of-bounds.
    [[nodiscard]] TileType getTile(TileCoord coord) const noexcept;

    /// @brief Query whether a coordinate represents an impassable solid obstacle.
    /// @param[in] coord Discrete grid coordinate.
    /// @return true if SolidWall or DestructibleWall; false otherwise.
    [[nodiscard]] bool isSolid(TileCoord coord) const noexcept;

    /// @brief Query whether a coordinate holds a destructible block.
    /// @param[in] coord Discrete grid coordinate.
    /// @return true if DestructibleWall; false otherwise.
    [[nodiscard]] bool isDestructible(TileCoord coord) const noexcept;

    /// @brief Query whether a coordinate holds a hazard cell.
    /// @param[in] coord Discrete grid coordinate.
    /// @return true if Hazard; false otherwise.
    [[nodiscard]] bool isHazard(TileCoord coord) const noexcept;

    /// @brief Query remaining durability of a destructible obstacle.
    /// @param[in] coord Discrete grid coordinate.
    /// @return Current TileHealth object; returns zero health if out-of-bounds.
    [[nodiscard]] TileHealth getHealth(TileCoord coord) const noexcept;

    /// @brief Assign tile material to designated coordinate.
    /// @param[in] coord Discrete grid coordinate.
    /// @param[in] type Material to write.
    void setTile(TileCoord coord, TileType type) noexcept;

    /// @brief Set explicit durability for a destructible tile.
    /// @param[in] coord Discrete grid coordinate.
    /// @param[in] health Durability level.
    void setHealth(TileCoord coord, TileHealth health) noexcept;

    /// @brief Apply damage to a destructible obstacle.
    /// @param[in] coord Discrete grid coordinate.
    /// @return true if damage resulted in complete block destruction; false otherwise.
    bool damageTile(TileCoord coord) noexcept;

    /// @brief Immediately destroy a destructible obstacle without iterative health depletion.
    /// @param[in] coord Discrete grid coordinate.
    /// @return true if a destructible block was removed; false if cell was not destructible.
    bool destroyTile(TileCoord coord) noexcept;

    /// @brief Evaluate whether an AABB hitbox has unobstructed clearance against all tiles.
    /// @param[in] box Bounding box in world coordinates.
    /// @return true if bounding box does not overlap any solid or destructible wall.
    [[nodiscard]] bool hasClearance(const Hitbox& box) const noexcept;

    /// @brief Compute bounding box in world coordinates for a tile cell.
    /// @param[in] coord Discrete grid coordinate.
    /// @return World-space Hitbox matching cell extents.
    [[nodiscard]] Hitbox getTileBox(TileCoord coord) const noexcept;

private:
    [[nodiscard]] bool inBounds(TileCoord coord) const noexcept;
    [[nodiscard]] std::size_t toIndex(TileCoord coord) const noexcept;

    std::uint32_t width_ { 0U };
    std::uint32_t height_ { 0U };
    float tileSize_ { kDefaultTileSize };
    std::vector<TileType> tiles_ { };
    std::vector<TileHealth> health_ { };
};

} // namespace ShooterCore
