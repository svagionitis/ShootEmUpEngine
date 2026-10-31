#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

#include "Hitbox.h"
#include "Types.h"

namespace ShooterCore {

/// @class CameraSystem
/// @brief High-precision smoothed deadzone camera tracking with sector clamping and arena lockdown.
class CameraSystem final {
public:
    static constexpr float kDefaultDeadzoneW { 64.0f };
    static constexpr float kDefaultDeadzoneH { 48.0f };
    static constexpr float kSmoothingFactor { 12.0f };
    static constexpr float kMaxShakeX { 12.0f };
    static constexpr float kMaxShakeY { 8.0f };
    static constexpr float kMaxShakeAngle { 2.5f };

    constexpr CameraSystem() noexcept = default;

    /// @brief Update camera tracking relative to player position.
    void update(DeltaTime dt, WorldPos playerPos, const SectorBounds& bounds) noexcept;

    /// @brief Lock viewport strictly to boss enclosure arena.
    void lockArena(const SectorBounds& arenaBounds) noexcept;

    /// @brief Release arena lock and return to dynamic sector tracking.
    void unlockArena() noexcept;

    /// @brief Query whether camera is locked to boss arena.
    [[nodiscard]] constexpr bool isLocked() const noexcept
    {
        return isArenaLocked_;
    }

    /// @brief Query current viewport world bounding box.
    [[nodiscard]] constexpr CameraViewport viewport() const noexcept
    {
        return viewport_;
    }

    /// @brief Direct setter for camera position (used during respawn / level start).
    void setPosition(WorldPos pos) noexcept
    {
        viewport_.pos = pos;
    }

    /// @brief Inject camera trauma from high-impact explosion or hit.
    /// @param[in] delta Trauma scalar to add (clamped to [0.0, 1.0]).
    void addTrauma(float delta) noexcept
    {
        trauma_.add(delta);
    }

    /// @brief Advance trauma decay accumulator by timestep.
    /// @param[in] dt Timestep duration.
    void updateTrauma(DeltaTime dt) noexcept;

    /// @brief Query active trauma value object.
    /// @return Current trauma level.
    [[nodiscard]] constexpr Trauma trauma() const noexcept
    {
        return trauma_;
    }

    /// @brief Calculate current kinetic displacement shake offset.
    /// @return Multi-axis shake offset.
    [[nodiscard]] ShakeOffset computeShakeOffset() const noexcept;

    /// @brief Compute effective camera world position including shake displacement.
    /// @return WorldPos offset by current shake.
    [[nodiscard]] WorldPos effectivePosition() const noexcept;

    /// @brief Reset trauma and shake accumulator.
    void resetTrauma() noexcept
    {
        trauma_.reset();
        shakeTime_ = 0.0f;
    }

private:
    CameraViewport viewport_ { { 0.0f, 0.0f }, 320.0f, 240.0f };
    bool isArenaLocked_ { false };
    SectorBounds lockedBounds_ { };
    Trauma trauma_ { };
    float shakeTime_ { 0.0f };
};

} // namespace ShooterCore
