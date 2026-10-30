#pragma once

#include <cstddef>
#include <cstdint>

#include "Types.h"

namespace ShooterCore {

/// @class Hitbox
/// @brief Axis-Aligned Bounding Box (AABB) for spatial collision queries.
class Hitbox final {
public:
    /// @brief Default construct empty hitbox at origin.
    constexpr Hitbox() noexcept = default;

    /// @brief Construct hitbox from explicit min and max positions.
    /// @param[in] minPos Top-left coordinate.
    /// @param[in] maxPos Bottom-right coordinate.
    constexpr Hitbox(WorldPos minPos, WorldPos maxPos) noexcept
        : min_ { minPos }
        , max_ { maxPos }
    {
    }

    /// @brief Create hitbox from origin position and positive dimensions.
    /// @param[in] origin Top-left coordinate.
    /// @param[in] width Horizontal extent in world units.
    /// @param[in] height Vertical extent in world units.
    /// @return Configured Hitbox instance.
    [[nodiscard]] static Hitbox fromDims(WorldPos origin, float width, float height) noexcept;

    /// @brief Get minimum bounding coordinate.
    /// @return Top-left WorldPos.
    [[nodiscard]] constexpr WorldPos min() const noexcept
    {
        return min_;
    }

    /// @brief Get maximum bounding coordinate.
    /// @return Bottom-right WorldPos.
    [[nodiscard]] constexpr WorldPos max() const noexcept
    {
        return max_;
    }

    /// @brief Compute bounding box width.
    /// @return Width in world units.
    [[nodiscard]] constexpr float width() const noexcept
    {
        return max_.x - min_.x;
    }

    /// @brief Compute bounding box height.
    /// @return Height in world units.
    [[nodiscard]] constexpr float height() const noexcept
    {
        return max_.y - min_.y;
    }

    /// @brief Test intersection with another AABB.
    /// @param[in] other Bounding box to test against.
    /// @return true if bounding boxes overlap; false otherwise.
    [[nodiscard]] bool intersects(const Hitbox& other) const noexcept;

    /// @brief Test whether a point lies inside this AABB.
    /// @param[in] point World coordinate to evaluate.
    /// @return true if point is contained within bounds.
    [[nodiscard]] bool contains(WorldPos point) const noexcept;

private:
    WorldPos min_ { 0.0f, 0.0f };
    WorldPos max_ { 0.0f, 0.0f };
};

} // namespace ShooterCore
