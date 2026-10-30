#include "Hitbox.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>

#include "Types.h"

namespace ShooterCore {

Hitbox Hitbox::fromDims(WorldPos origin, float width, float height) noexcept
{
    const float safeW { std::max(0.0f, width) };
    const float safeH { std::max(0.0f, height) };

    const WorldPos maxPos { origin.x + safeW, origin.y + safeH };
    return Hitbox { origin, maxPos };
}

bool Hitbox::intersects(const Hitbox& other) const noexcept
{
    const bool overlapX { min_.x < other.max_.x && max_.x > other.min_.x };
    const bool overlapY { min_.y < other.max_.y && max_.y > other.min_.y };

    return overlapX && overlapY;
}

bool Hitbox::contains(WorldPos point) const noexcept
{
    const bool insideX { point.x >= min_.x && point.x <= max_.x };
    const bool insideY { point.y >= min_.y && point.y <= max_.y };

    return insideX && insideY;
}

} // namespace ShooterCore
