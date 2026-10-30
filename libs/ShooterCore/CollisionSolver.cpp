#include "CollisionSolver.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <cmath>

#include "Hitbox.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore {

SweepHit CollisionSolver::sweepBox(const Hitbox& box, Velocity vel, float dt, const Hitbox& obstacle) noexcept
{
    const float dx { vel.dx * dt };
    const float dy { vel.dy * dt };

    if (dx == 0.0f && dy == 0.0f) {
        if (box.intersects(obstacle)) {
            return SweepHit { true, 0.0f, ContactNormal::None, TileCoord { 0U, 0U }, TileType::SolidWall };
        }
        return SweepHit { };
    }

    const float expMinX { obstacle.min().x - box.width() };
    const float expMinY { obstacle.min().y - box.height() };
    const float expMaxX { obstacle.max().x };
    const float expMaxY { obstacle.max().y };

    const float p0X { box.min().x };
    const float p0Y { box.min().y };

    float entryX { -1e30f };
    float exitX { 1e30f };

    if (dx == 0.0f) {
        if (p0X <= expMinX || p0X >= expMaxX) {
            return SweepHit { };
        }
    } else {
        const float invDx { 1.0f / dx };
        float t1 { (expMinX - p0X) * invDx };
        float t2 { (expMaxX - p0X) * invDx };
        if (t1 > t2) {
            std::swap(t1, t2);
        }
        entryX = t1;
        exitX = t2;
    }

    float entryY { -1e30f };
    float exitY { 1e30f };

    if (dy == 0.0f) {
        if (p0Y <= expMinY || p0Y >= expMaxY) {
            return SweepHit { };
        }
    } else {
        const float invDy { 1.0f / dy };
        float t1 { (expMinY - p0Y) * invDy };
        float t2 { (expMaxY - p0Y) * invDy };
        if (t1 > t2) {
            std::swap(t1, t2);
        }
        entryY = t1;
        exitY = t2;
    }

    const float entryTime { std::max(entryX, entryY) };
    const float exitTime { std::min(exitX, exitY) };

    if (entryTime > exitTime) {
        return SweepHit { };
    }

    if (entryTime >= 1.0f || exitTime <= 0.0f) {
        return SweepHit { };
    }

    if (entryTime < 0.0f) {
        if (box.intersects(obstacle)) {
            return SweepHit { true, 0.0f, ContactNormal::None, TileCoord { 0U, 0U }, TileType::SolidWall };
        }
        return SweepHit { };
    }

    ContactNormal normal { ContactNormal::None };
    if (entryX > entryY) {
        normal = (dx > 0.0f) ? ContactNormal::Left : ContactNormal::Right;
    } else {
        normal = (dy > 0.0f) ? ContactNormal::Up : ContactNormal::Down;
    }

    SweepHit hit { };
    hit.collided = true;
    hit.toi = entryTime;
    hit.normal = normal;
    return hit;
}

SweepHit CollisionSolver::sweepMap(const Hitbox& box, Velocity vel, float dt, const TileMap& map) noexcept
{
    const float dx { vel.dx * dt };
    const float dy { vel.dy * dt };

    if (dx == 0.0f && dy == 0.0f) {
        if (checkOverlap(box, map)) {
            return SweepHit { true, 0.0f, ContactNormal::None, TileCoord { 0U, 0U }, TileType::SolidWall };
        }
        return SweepHit { };
    }

    const float tileSize { map.tileSize() };
    if (tileSize <= 0.0f) {
        return SweepHit { };
    }

    const float broadMinX { std::min(box.min().x, box.min().x + dx) };
    const float broadMinY { std::min(box.min().y, box.min().y + dy) };
    const float broadMaxX { std::max(box.max().x, box.max().x + dx) };
    const float broadMaxY { std::max(box.max().y, box.max().y + dy) };

    const auto colMin { static_cast<std::uint32_t>(std::max(0.0f, broadMinX) / tileSize) };
    const auto rowMin { static_cast<std::uint32_t>(std::max(0.0f, broadMinY) / tileSize) };

    const float eps { 0.0001f };
    const float adjMaxX { std::max(broadMinX, broadMaxX - eps) };
    const float adjMaxY { std::max(broadMinY, broadMaxY - eps) };

    const auto rawColMax { static_cast<std::uint32_t>(std::max(0.0f, adjMaxX) / tileSize) };
    const auto rawRowMax { static_cast<std::uint32_t>(std::max(0.0f, adjMaxY) / tileSize) };

    const std::uint32_t maxMapCol { (map.width() > 0U) ? (map.width() - 1U) : 0U };
    const std::uint32_t maxMapRow { (map.height() > 0U) ? (map.height() - 1U) : 0U };

    const std::uint32_t colMax { std::min(rawColMax, maxMapCol) };
    const std::uint32_t rowMax { std::min(rawRowMax, maxMapRow) };

    SweepHit earliestHit { };
    earliestHit.toi = 1.0f;

    for (std::uint32_t r { rowMin }; r <= rowMax; ++r) {
        for (std::uint32_t c { colMin }; c <= colMax; ++c) {
            const TileCoord coord { c, r };
            if (map.isSolid(coord)) {
                const Hitbox tileBox { map.getTileBox(coord) };
                const SweepHit hit { sweepBox(box, vel, dt, tileBox) };
                if (hit.collided && hit.toi < earliestHit.toi) {
                    earliestHit = hit;
                    earliestHit.hitTile = coord;
                    earliestHit.tileType = map.getTile(coord);
                }
            }
        }
    }

    return earliestHit;
}

WorldPos CollisionSolver::resolveMove(const Hitbox& box, Velocity vel, float dt, const TileMap& map) noexcept
{
    if (dt <= 0.0f) {
        return box.min();
    }

    WorldPos currentPos { box.min() };
    Velocity currentVel { vel };
    float remainingDt { dt };
    const float kContactEpsilon { 0.001f };

    for (int iter { 0 }; iter < 2; ++iter) {
        if (currentVel.dx == 0.0f && currentVel.dy == 0.0f) {
            break;
        }

        const Hitbox currentBox { Hitbox::fromDims(currentPos, box.width(), box.height()) };
        const SweepHit hit { sweepMap(currentBox, currentVel, remainingDt, map) };

        if (!hit.collided) {
            currentPos.x += currentVel.dx * remainingDt;
            currentPos.y += currentVel.dy * remainingDt;
            break;
        }

        const float advanceToi { std::max(0.0f, hit.toi - kContactEpsilon) };
        currentPos.x += currentVel.dx * remainingDt * advanceToi;
        currentPos.y += currentVel.dy * remainingDt * advanceToi;

        remainingDt = remainingDt * std::max(0.0f, 1.0f - hit.toi);
        if (remainingDt <= 0.0f) {
            break;
        }

        if (hit.normal == ContactNormal::Left || hit.normal == ContactNormal::Right) {
            currentVel.dx = 0.0f;
        } else if (hit.normal == ContactNormal::Up || hit.normal == ContactNormal::Down) {
            currentVel.dy = 0.0f;
        } else {
            break;
        }
    }

    return currentPos;
}

bool CollisionSolver::checkOverlap(const Hitbox& box, const TileMap& map) noexcept
{
    return !map.hasClearance(box);
}

bool CollisionSolver::raycast(WorldPos start, WorldPos end, const TileMap& map, WorldPos& outHit) noexcept
{
    const float tileSize { map.tileSize() };
    if (tileSize <= 0.0f) {
        return false;
    }

    const float dx { end.x - start.x };
    const float dy { end.y - start.y };

    TileCoord current { start.toTileCoord(tileSize) };
    const TileCoord target { end.toTileCoord(tileSize) };

    if (map.isSolid(current)) {
        outHit = start;
        return true;
    }

    const int stepX { (dx >= 0.0f) ? 1 : -1 };
    const int stepY { (dy >= 0.0f) ? 1 : -1 };

    const float invDx { (dx != 0.0f) ? std::abs(1.0f / dx) : 1e30f };
    const float invDy { (dy != 0.0f) ? std::abs(1.0f / dy) : 1e30f };

    const float cellMinX { static_cast<float>(current.col) * tileSize };
    const float cellMinY { static_cast<float>(current.row) * tileSize };

    float tMaxX { 0.0f };
    if (dx > 0.0f) {
        tMaxX = (cellMinX + tileSize - start.x) * invDx;
    } else if (dx < 0.0f) {
        tMaxX = (start.x - cellMinX) * invDx;
    } else {
        tMaxX = 1e30f;
    }

    float tMaxY { 0.0f };
    if (dy > 0.0f) {
        tMaxY = (cellMinY + tileSize - start.y) * invDy;
    } else if (dy < 0.0f) {
        tMaxY = (start.y - cellMinY) * invDy;
    } else {
        tMaxY = 1e30f;
    }

    const float tDeltaX { tileSize * invDx };
    const float tDeltaY { tileSize * invDy };

    float tHit { 0.0f };

    while (current != target) {
        if (tMaxX < tMaxY) {
            tHit = tMaxX;
            tMaxX += tDeltaX;
            if (stepX > 0) {
                ++current.col;
            } else if (current.col > 0U) {
                --current.col;
            } else {
                break;
            }
        } else {
            tHit = tMaxY;
            tMaxY += tDeltaY;
            if (stepY > 0) {
                ++current.row;
            } else if (current.row > 0U) {
                --current.row;
            } else {
                break;
            }
        }

        if (tHit > 1.0f) {
            break;
        }

        if (map.isSolid(current)) {
            outHit.x = start.x + dx * std::min(1.0f, tHit);
            outHit.y = start.y + dy * std::min(1.0f, tHit);
            return true;
        }
    }

    return false;
}

} // namespace ShooterCore
