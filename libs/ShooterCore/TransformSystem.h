#pragma once

#include <cstddef>
#include <cstdint>

#include "Hitbox.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore {

/// @class TransformSystem
/// @brief Deterministic dual-mode transformation state machine.
/// @details Enforces spatial clearance before allowing state transitions.
/// Operates with zero dynamic heap allocations (CERT ERR55-CPP).
class TransformSystem final {
public:
    static constexpr float kRobotWidth { 16.0f };
    static constexpr float kRobotHeight { 32.0f };
    static constexpr float kJetWidth { 32.0f };
    static constexpr float kJetHeight { 16.0f };
    static constexpr float kDefaultTransitionSec { 0.100f };

    /// @brief Construct transformation system in default Robot state.
    /// @param[in] transitionSec Duration in seconds for transformation animation.
    explicit constexpr TransformSystem(float transitionSec = kDefaultTransitionSec) noexcept
        : transitionDuration_ { transitionSec > 0.0f ? transitionSec : kDefaultTransitionSec }
    {
    }

    /// @brief Get active craft mode.
    /// @return Active TransformMode.
    [[nodiscard]] constexpr TransformMode mode() const noexcept
    {
        return mode_;
    }

    /// @brief Check whether currently undergoing transformation.
    /// @return true if TransformingToJet or TransformingToRobot.
    [[nodiscard]] constexpr bool isTransforming() const noexcept
    {
        return (mode_ == TransformMode::TransformingToJet) || (mode_ == TransformMode::TransformingToRobot);
    }

    /// @brief Check whether weapons are locked due to active transformation.
    /// @return true if transforming; false if ready in stable Robot or Jet state.
    [[nodiscard]] constexpr bool isWeaponLocked() const noexcept
    {
        return isTransforming();
    }

    /// @brief Get transformation animation completion progress.
    /// @return Normalized progress in range [0.0, 1.0].
    [[nodiscard]] constexpr float progress() const noexcept
    {
        if (!isTransforming()) {
            return 1.0f;
        }
        if (transitionDuration_ <= 0.0f) {
            return 1.0f;
        }
        const float ratio { transitionTimer_ / transitionDuration_ };
        if (ratio < 0.0f) {
            return 0.0f;
        }
        if (ratio > 1.0f) {
            return 1.0f;
        }
        return ratio;
    }

    /// @brief Request transformation to the alternate craft mode.
    /// @param[in] currentPos Current origin of the craft.
    /// @param[in] map Terrain grid to verify clearance against.
    /// @param[in] anchor Alignment model for projected target hitbox.
    /// @return TransformResult indicating success or rejection cause.
    [[nodiscard]] TransformResult requestTransform(
        WorldPos currentPos, const TileMap& map, AnchorMode anchor = AnchorMode::BottomCenter) noexcept;

    /// @brief Advance simulation timer by fixed delta time.
    /// @param[in] dt Timestep duration.
    void update(DeltaTime dt) noexcept;

    /// @brief Force state for testing or level initialization.
    /// @param[in] targetMode Mode to apply immediately.
    void forceState(TransformMode targetMode) noexcept;

    /// @brief Compute projected bounding box for target mode at specified anchor.
    /// @param[in] currentPos Current position.
    /// @param[in] currentMode Origin mode.
    /// @param[in] targetMode Desired mode.
    /// @param[in] anchor Alignment model.
    /// @return Calculated target Hitbox.
    [[nodiscard]] static Hitbox projectBox(
        WorldPos currentPos, TransformMode currentMode, TransformMode targetMode, AnchorMode anchor) noexcept;

    /// @brief Get physical dimensions for specified mode.
    /// @param[in] mode Evaluated craft configuration.
    /// @param[out] width Horizontal extent in world units.
    /// @param[out] height Vertical extent in world units.
    static constexpr void getDims(TransformMode mode, float& width, float& height) noexcept
    {
        if (mode == TransformMode::Jet || mode == TransformMode::TransformingToRobot) {
            width = kJetWidth;
            height = kJetHeight;
        } else {
            width = kRobotWidth;
            height = kRobotHeight;
        }
    }

private:
    TransformMode mode_ { TransformMode::Robot };
    float transitionDuration_ { kDefaultTransitionSec };
    float transitionTimer_ { 0.0f };
};

} // namespace ShooterCore
