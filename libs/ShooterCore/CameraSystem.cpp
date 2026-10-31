#include "CameraSystem.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "Hitbox.h"
#include "Types.h"

namespace ShooterCore {

void CameraSystem::lockArena(const SectorBounds& arenaBounds) noexcept
{
    isArenaLocked_ = true;
    lockedBounds_ = arenaBounds;

    const float maxX { std::max(arenaBounds.min.x, arenaBounds.max.x - viewport_.width) };
    const float maxY { std::max(arenaBounds.min.y, arenaBounds.max.y - viewport_.height) };
    viewport_.pos.x = std::clamp(viewport_.pos.x, arenaBounds.min.x, maxX);
    viewport_.pos.y = std::clamp(viewport_.pos.y, arenaBounds.min.y, maxY);
}

void CameraSystem::unlockArena() noexcept
{
    isArenaLocked_ = false;
}

void CameraSystem::update(DeltaTime dt, WorldPos playerPos, const SectorBounds& bounds) noexcept
{
    const SectorBounds& activeBounds { isArenaLocked_ ? lockedBounds_ : bounds };

    const float halfVw { viewport_.width * 0.5f };
    const float halfVh { viewport_.height * 0.5f };
    const float halfDw { kDefaultDeadzoneW * 0.5f };
    const float halfDh { kDefaultDeadzoneH * 0.5f };

    const float centerX { viewport_.pos.x + halfVw };
    const float centerY { viewport_.pos.y + halfVh };

    float targetX { viewport_.pos.x };
    float targetY { viewport_.pos.y };

    // Deadzone horizontal check
    if (playerPos.x > centerX + halfDw) {
        targetX = playerPos.x - halfVw - halfDw;
    } else if (playerPos.x < centerX - halfDw) {
        targetX = playerPos.x - halfVw + halfDw;
    }

    // Deadzone vertical check
    if (playerPos.y > centerY + halfDh) {
        targetY = playerPos.y - halfVh - halfDh;
    } else if (playerPos.y < centerY - halfDh) {
        targetY = playerPos.y - halfVh + halfDh;
    }

    // Smooth exponential dampening
    const float alpha { std::clamp(dt.seconds() * kSmoothingFactor, 0.0f, 1.0f) };
    viewport_.pos.x += (targetX - viewport_.pos.x) * alpha;
    viewport_.pos.y += (targetY - viewport_.pos.y) * alpha;

    // Viewport clamping to active sector / arena bounds
    const float maxX { std::max(activeBounds.min.x, activeBounds.max.x - viewport_.width) };
    const float maxY { std::max(activeBounds.min.y, activeBounds.max.y - viewport_.height) };

    viewport_.pos.x = std::clamp(viewport_.pos.x, activeBounds.min.x, maxX);
    viewport_.pos.y = std::clamp(viewport_.pos.y, activeBounds.min.y, maxY);
}

} // namespace ShooterCore
