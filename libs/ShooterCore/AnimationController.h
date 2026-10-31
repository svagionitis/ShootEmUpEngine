#pragma once

#include <cstddef>
#include <cstdint>

#include "Types.h"

namespace ShooterCore {

/// @class AnimationController
/// @brief Deterministic zero-allocation animation state and frame timer engine.
/// @details Accrues delta time and calculates active sprite frames for the player
///          craft, enemies, afterburners, and explosion bursts (CERT ERR55-CPP).
class AnimationController final {
public:
    static constexpr float kWalkFrameDuration { 0.120f };
    static constexpr float kWalk8FrameDuration { 0.080f };
    static constexpr float kExhaustFrameDuration { 0.060f };
    static constexpr float kEnemyFrameDuration { 0.150f };
    static constexpr float kExplosionTotalTime { 0.320f };

    constexpr AnimationController() noexcept = default;

    /// @brief Advance animation timers by delta time.
    /// @param[in] dt Frame simulation timestep.
    void update(DeltaTime dt) noexcept;

    /// @brief Reset all internal timer accumulators.
    void reset() noexcept;

    /// @brief Select active player robot sprite based on movement flags.
    /// @param[in] moving true if robot is traversing horizontally.
    /// @param[in] inAir true if robot is jumping or falling.
    /// @param[in] crouching true if robot is tucked in crouch stance.
    /// @return Active SpriteId for player robot.
    [[nodiscard]] SpriteId currentRobotSprite(bool moving, bool inAir, bool crouching) const noexcept;

    /// @brief Select active jet chassis sprite.
    /// @return Active SpriteId for player jet.
    [[nodiscard]] constexpr SpriteId currentJetSprite() const noexcept
    {
        return SpriteId::PlayerJet;
    }

    /// @brief Select alternating jet afterburner exhaust flame frame.
    /// @return Active SpriteId for jet exhaust.
    [[nodiscard]] SpriteId currentJetExhaust() const noexcept;

    /// @brief Select intermediate transformation morphing frame.
    /// @param[in] progress Normalized transition progress [0.0f, 1.0f].
    /// @param[in] toJet true if transforming into jet; false if into robot.
    /// @return Active SpriteId for morph sequence.
    [[nodiscard]] SpriteId currentMorphSprite(float progress, bool toJet) const noexcept;

    /// @brief Select animated enemy sprite.
    /// @param[in] type Hostile entity category.
    /// @return Active SpriteId for the enemy type.
    [[nodiscard]] SpriteId currentEnemySprite(EntityType type) const noexcept;

    /// @brief Select multi-phase explosion frame based on lifetime progress.
    /// @param[in] progress Normalized explosion lifetime [0.0f, 1.0f].
    /// @return Active SpriteId for explosion phase.
    [[nodiscard]] SpriteId currentExplosionSprite(float progress) const noexcept;

    /// @brief Select authentic EGA mecha sprite frame based on full simulation state.
    /// @param[in] mode Transformation mode (Robot vs Jet vs Morph).
    /// @param[in] facing Direction mecha is facing (Left vs Right).
    /// @param[in] moving true if traversing horizontally.
    /// @param[in] inAir true if jumping or in flight.
    /// @param[in] crouching true if tucked in crouch stance.
    /// @param[in] laserActive true if auto-aiming laser is actively firing.
    /// @param[in] aimAngle Elevation angle in radians if laser is active.
    /// @param[in] vel Current velocity vector (dx, dy).
    /// @param[in] morphProgress Transformation progress [0.0f, 1.0f].
    /// @param[in] shieldActive true if shield barrier is active.
    /// @return Strongly-typed SpriteId for authentic EGA mecha frame.
    [[nodiscard]] SpriteId currentEgaMechaSprite(TransformMode mode, FacingDirection facing, bool moving, bool inAir,
        bool crouching, bool laserActive, float aimAngle, Velocity vel, float morphProgress,
        bool shieldActive) const noexcept;

    /// @brief Query active robot walk frame index (0, 1, or 2).
    /// @return Frame index.
    [[nodiscard]] constexpr std::uint8_t walkFrameIndex() const noexcept
    {
        return walkIndex_;
    }

    /// @brief Query active 8-frame walk index (0..7).
    /// @return Frame index [0..7].
    [[nodiscard]] constexpr std::uint8_t walk8Index() const noexcept
    {
        return walkIndex8_;
    }

    /// @brief Query active exhaust frame index (0 or 1).
    /// @return Frame index.
    [[nodiscard]] constexpr std::uint8_t exhaustFrameIndex() const noexcept
    {
        return exhaustIndex_;
    }

private:
    float walkTimer_ { 0.0f };
    float walkTimer8_ { 0.0f };
    float exhaustTimer_ { 0.0f };
    float enemyTimer_ { 0.0f };
    std::uint8_t walkIndex_ { 0U };
    std::uint8_t walkIndex8_ { 0U };
    std::uint8_t exhaustIndex_ { 0U };
    std::uint8_t enemyIndex_ { 0U };
};

} // namespace ShooterCore
