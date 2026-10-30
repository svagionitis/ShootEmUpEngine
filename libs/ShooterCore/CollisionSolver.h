#pragma once

#include <cstddef>
#include <cstdint>

#include "Hitbox.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore {

/// @class CollisionSolver
/// @brief Continuous Collision Detection (CCD) and spatial physics resolution.
/// @details Executes zero-heap Swept AABB calculations and corner sliding responses.
class CollisionSolver final {
public:
    /// @brief Continuous swept intersection between a moving hitbox and a static obstacle.
    /// @param[in] box Moving bounding box at origin.
    /// @param[in] vel Velocity vector of the moving box.
    /// @param[in] dt Timestep duration in seconds.
    /// @param[in] obstacle Static bounding box.
    /// @return SweepHit containing TOI in [0.0, 1.0], contact normal, and collision flag.
    [[nodiscard]] static SweepHit sweepBox(const Hitbox& box, Velocity vel, float dt, const Hitbox& obstacle) noexcept;

    /// @brief Continuous swept intersection of a moving hitbox against all solid tiles.
    /// @param[in] box Moving bounding box at start of tick.
    /// @param[in] vel Velocity vector.
    /// @param[in] dt Timestep duration in seconds.
    /// @param[in] map Discrete tile grid environment.
    /// @return Earliest SweepHit encountered along the trajectory.
    [[nodiscard]] static SweepHit sweepMap(const Hitbox& box, Velocity vel, float dt, const TileMap& map) noexcept;

    /// @brief Compute final world position with obstacle deflection and corner sliding.
    /// @param[in] box Moving bounding box at tick start.
    /// @param[in] vel Intended velocity vector.
    /// @param[in] dt Timestep duration in seconds.
    /// @param[in] map Discrete tile grid environment.
    /// @return Final resolved WorldPos of the bounding box origin.
    [[nodiscard]] static WorldPos resolveMove(const Hitbox& box, Velocity vel, float dt, const TileMap& map) noexcept;

    /// @brief Evaluate whether an AABB intersects any solid or destructible map obstacle.
    /// @param[in] box Bounding box to test.
    /// @param[in] map Discrete tile grid environment.
    /// @return true if overlapping any solid or destructible tile.
    [[nodiscard]] static bool checkOverlap(const Hitbox& box, const TileMap& map) noexcept;

    /// @brief Cast a discrete ray from start to end against the tile grid.
    /// @param[in] start Origin point in world units.
    /// @param[in] end Destination point in world units.
    /// @param[in] map Discrete tile grid environment.
    /// @param[out] outHit WorldPos where ray impacts first solid obstacle.
    /// @return true if ray impacted a solid obstacle; false if line of sight is clear.
    [[nodiscard]] static bool raycast(WorldPos start, WorldPos end, const TileMap& map, WorldPos& outHit) noexcept;
};

} // namespace ShooterCore
