#include "TileMap.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <vector>

#include <glog/logging.h>

#include "Hitbox.h"
#include "Types.h"

namespace ShooterCore {

TileMap::TileMap(std::uint32_t width, std::uint32_t height, float tileSize)
    : width_ { width }
    , height_ { height }
    , tileSize_ { std::max(1.0f, tileSize) }
    , tiles_ { static_cast<std::size_t>(width) * static_cast<std::size_t>(height), TileType::Empty }
    , health_ { static_cast<std::size_t>(width) * static_cast<std::size_t>(height), TileHealth { 0U } }
{
}

bool TileMap::inBounds(TileCoord coord) const noexcept
{
    return coord.col < width_ && coord.row < height_;
}

std::size_t TileMap::toIndex(TileCoord coord) const noexcept
{
    return (static_cast<std::size_t>(coord.row) * static_cast<std::size_t>(width_))
        + static_cast<std::size_t>(coord.col);
}

TileType TileMap::getTile(TileCoord coord) const noexcept
{
    if (!inBounds(coord)) {
        return TileType::SolidWall;
    }
    return tiles_[toIndex(coord)];
}

bool TileMap::isSolid(TileCoord coord) const noexcept
{
    const TileType type { getTile(coord) };
    return type == TileType::SolidWall || type == TileType::DestructibleWall || type == TileType::SecretWall
        || type == TileType::Forcefield;
}

bool TileMap::isDestructible(TileCoord coord) const noexcept
{
    const TileType type { getTile(coord) };
    return type == TileType::DestructibleWall || type == TileType::SecretWall;
}

bool TileMap::isHazard(TileCoord coord) const noexcept
{
    return getTile(coord) == TileType::Hazard;
}

bool TileMap::isSecret(TileCoord coord) const noexcept
{
    return getTile(coord) == TileType::SecretWall;
}

bool TileMap::isForcefield(TileCoord coord) const noexcept
{
    return getTile(coord) == TileType::Forcefield;
}

TileHealth TileMap::getHealth(TileCoord coord) const noexcept
{
    if (!inBounds(coord)) {
        return TileHealth { 0U };
    }
    return health_[toIndex(coord)];
}

void TileMap::setTile(TileCoord coord, TileType type) noexcept
{
    if (!inBounds(coord)) {
        return;
    }

    const std::size_t idx { toIndex(coord) };
    tiles_[idx] = type;
    if (type == TileType::DestructibleWall || type == TileType::SecretWall) {
        health_[idx].reset();
    } else {
        health_[idx] = TileHealth { 0U };
    }
}

void TileMap::setHealth(TileCoord coord, TileHealth health) noexcept
{
    if (!inBounds(coord)) {
        return;
    }

    const std::size_t idx { toIndex(coord) };
    health_[idx] = health;
}

bool TileMap::damageTile(TileCoord coord) noexcept
{
    if (!inBounds(coord)) {
        return false;
    }

    const std::size_t idx { toIndex(coord) };
    if (tiles_[idx] != TileType::DestructibleWall && tiles_[idx] != TileType::SecretWall) {
        return false;
    }

    health_[idx].damage();
    if (health_[idx].isDestroyed()) {
        tiles_[idx] = TileType::Empty;
        LOG(INFO) << "TileMap: Block eroded and destroyed at (" << coord.col << ", " << coord.row << ").";
        return true;
    }

    return false;
}

bool TileMap::destroyTile(TileCoord coord) noexcept
{
    if (!inBounds(coord)) {
        return false;
    }

    const std::size_t idx { toIndex(coord) };
    if (tiles_[idx] != TileType::DestructibleWall && tiles_[idx] != TileType::SecretWall) {
        return false;
    }

    health_[idx] = TileHealth { 0U };
    tiles_[idx] = TileType::Empty;
    LOG(INFO) << "TileMap: Block destroyed at (" << coord.col << ", " << coord.row << ").";
    return true;
}

Hitbox TileMap::getTileBox(TileCoord coord) const noexcept
{
    const float originX { static_cast<float>(coord.col) * tileSize_ };
    const float originY { static_cast<float>(coord.row) * tileSize_ };
    return Hitbox::fromDims(WorldPos { originX, originY }, tileSize_, tileSize_);
}

bool TileMap::hasClearance(const Hitbox& box) const noexcept
{
    if (box.width() <= 0.0f || box.height() <= 0.0f) {
        return false;
    }

    const float mapWorldW { static_cast<float>(width_) * tileSize_ };
    const float mapWorldH { static_cast<float>(height_) * tileSize_ };

    if (box.min().x < 0.0f || box.min().y < 0.0f || box.max().x > mapWorldW || box.max().y > mapWorldH) {
        return false;
    }

    const float safeMinX { std::max(0.0f, box.min().x) };
    const float safeMinY { std::max(0.0f, box.min().y) };
    const float safeMaxX { std::max(0.0f, box.max().x) };
    const float safeMaxY { std::max(0.0f, box.max().y) };

    const auto colMin { static_cast<std::uint32_t>(safeMinX / tileSize_) };
    const auto rowMin { static_cast<std::uint32_t>(safeMinY / tileSize_) };

    const float eps { 0.0001f };
    const float adjMaxX { std::max(safeMinX, safeMaxX - eps) };
    const float adjMaxY { std::max(safeMinY, safeMaxY - eps) };

    const auto colMax { static_cast<std::uint32_t>(adjMaxX / tileSize_) };
    const auto rowMax { static_cast<std::uint32_t>(adjMaxY / tileSize_) };

    for (std::uint32_t r { rowMin }; r <= rowMax; ++r) {
        for (std::uint32_t c { colMin }; c <= colMax; ++c) {
            const TileCoord coord { c, r };
            if (isSolid(coord)) {
                if (box.intersects(getTileBox(coord))) {
                    return false;
                }
            }
        }
    }

    return true;
}

} // namespace ShooterCore
