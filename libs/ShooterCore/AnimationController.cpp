#include "AnimationController.h"

#include <cmath>
#include <cstddef>
#include <cstdint>

#include <algorithm>

#include "Types.h"

namespace ShooterCore {

void AnimationController::update(DeltaTime dt) noexcept
{
    const float deltaSeconds { dt.seconds() };
    if (deltaSeconds <= 0.0f) {
        return;
    }

    // Advance 3-frame walk cycle timer
    walkTimer_ += deltaSeconds;
    while (walkTimer_ >= kWalkFrameDuration) {
        walkTimer_ -= kWalkFrameDuration;
        walkIndex_ = static_cast<std::uint8_t>((walkIndex_ + 1U) % 3U);
    }

    // Advance 8-frame walk cycle timer
    walkTimer8_ += deltaSeconds;
    while (walkTimer8_ >= kWalk8FrameDuration) {
        walkTimer8_ -= kWalk8FrameDuration;
        walkIndex8_ = static_cast<std::uint8_t>((walkIndex8_ + 1U) % 8U);
    }

    // Advance 2-frame exhaust flicker timer
    exhaustTimer_ += deltaSeconds;
    while (exhaustTimer_ >= kExhaustFrameDuration) {
        exhaustTimer_ -= kExhaustFrameDuration;
        exhaustIndex_ = static_cast<std::uint8_t>((exhaustIndex_ + 1U) % 2U);
    }

    // Advance 2-frame enemy animation timer
    enemyTimer_ += deltaSeconds;
    while (enemyTimer_ >= kEnemyFrameDuration) {
        enemyTimer_ -= kEnemyFrameDuration;
        enemyIndex_ = static_cast<std::uint8_t>((enemyIndex_ + 1U) % 2U);
    }
}

void AnimationController::reset() noexcept
{
    walkTimer_ = 0.0f;
    walkTimer8_ = 0.0f;
    exhaustTimer_ = 0.0f;
    enemyTimer_ = 0.0f;
    walkIndex_ = 0U;
    walkIndex8_ = 0U;
    exhaustIndex_ = 0U;
    enemyIndex_ = 0U;
}

SpriteId AnimationController::currentRobotSprite(bool moving, bool inAir, bool crouching) const noexcept
{
    if (crouching) {
        return SpriteId::PlayerRobotCrouch;
    }

    if (inAir) {
        return SpriteId::PlayerRobotJump;
    }

    if (!moving) {
        return SpriteId::PlayerRobotIdle;
    }

    switch (walkIndex_) {
    case 0U:
        return SpriteId::PlayerRobotWalk1;
    case 1U:
        return SpriteId::PlayerRobotWalk2;
    case 2U:
    default:
        return SpriteId::PlayerRobotWalk3;
    }
}

SpriteId AnimationController::currentJetExhaust() const noexcept
{
    return (exhaustIndex_ == 0U) ? SpriteId::PlayerJetExhaust1 : SpriteId::PlayerJetExhaust2;
}

SpriteId AnimationController::currentMorphSprite(float progress, bool toJet) const noexcept
{
    const float clampedProgress { std::clamp(progress, 0.0f, 1.0f) };

    if (toJet) {
        if (clampedProgress < 0.33f) {
            return SpriteId::PlayerMorph1;
        }
        if (clampedProgress < 0.66f) {
            return SpriteId::PlayerMorph2;
        }
        return SpriteId::PlayerMorph3;
    }

    // Transforming back to Robot: Reverse order
    if (clampedProgress < 0.33f) {
        return SpriteId::PlayerMorph3;
    }
    if (clampedProgress < 0.66f) {
        return SpriteId::PlayerMorph2;
    }
    return SpriteId::PlayerMorph1;
}

SpriteId AnimationController::currentEnemySprite(EntityType type) const noexcept
{
    switch (type) {
    case EntityType::PatrolWalker:
        return (enemyIndex_ == 0U) ? SpriteId::EnemyCrawler1 : SpriteId::EnemyCrawler2;
    case EntityType::HomingFlyer:
        return (enemyIndex_ == 0U) ? SpriteId::EnemyBuzzer1 : SpriteId::EnemyBuzzer2;
    case EntityType::None:
    case EntityType::Player:
    case EntityType::HostileBullet:
    case EntityType::Particle:
    default:
        return SpriteId::None;
    }
}

SpriteId AnimationController::currentExplosionSprite(float progress) const noexcept
{
    const float clampedProgress { std::clamp(progress, 0.0f, 1.0f) };

    if (clampedProgress < 0.25f) {
        return SpriteId::ExplosionPhase1;
    }
    if (clampedProgress < 0.50f) {
        return SpriteId::ExplosionPhase2;
    }
    if (clampedProgress < 0.75f) {
        return SpriteId::ExplosionPhase3;
    }
    return SpriteId::ExplosionPhase4;
}

SpriteId AnimationController::currentEgaMechaSprite(TransformMode mode, FacingDirection facing, bool moving, bool inAir,
    bool crouching, bool laserActive, float aimAngle, Velocity vel, float morphProgress,
    bool shieldActive) const noexcept
{
    // 1. Shield flash precedence (blinking every alternate walk frame)
    if (shieldActive && (walkIndex_ % 2U == 0U)) {
        return toEgaMechaSpriteId(static_cast<std::uint8_t>(80U + (walkIndex_ % 8U)));
    }

    // 2. Morphing metamorphosis sequence (Frames 40..55 = 16 frames)
    if (mode == TransformMode::TransformingToJet || mode == TransformMode::TransformingToRobot) {
        const float clampedProgress { std::clamp(morphProgress, 0.0f, 0.999f) };
        auto step { static_cast<std::uint8_t>(clampedProgress * 16.0f) };
        if (mode == TransformMode::TransformingToRobot) {
            step = static_cast<std::uint8_t>(15U - step);
        }
        return toEgaMechaSpriteId(static_cast<std::uint8_t>(40U + step));
    }

    // 3. Jet flight kinematics (Frames 56..71 = 8 directional sectors x 2 frames)
    if (mode == TransformMode::Jet) {
        const float speedSq { vel.dx * vel.dx + vel.dy * vel.dy };
        int sector { (facing == FacingDirection::Left) ? 4 : 0 };
        if (speedSq > 100.0f) {
            float rad { std::atan2(vel.dy, vel.dx) };
            if (rad < 0.0f) {
                rad += 6.2831853f;
            }
            sector = static_cast<int>((rad + 0.392699f) / 0.785398f) % 8;
        }

        const auto baseFrame { static_cast<std::uint8_t>(56U + static_cast<std::uint8_t>(sector * 2)) };
        const auto exhaustToggle { static_cast<std::uint8_t>(exhaustIndex_ % 2U) };
        return toEgaMechaSpriteId(static_cast<std::uint8_t>(baseFrame + exhaustToggle));
    }

    // 4. Robot stance kinematics
    if (crouching) {
        return toEgaMechaSpriteId(static_cast<std::uint8_t>(32U + (walkIndex_ % 8U)));
    }

    if (laserActive) {
        const auto base { static_cast<std::uint8_t>((facing == FacingDirection::Left) ? 24U : 16U) };
        const float clampedAngle { std::clamp(aimAngle, -1.57079f, 1.57079f) };
        const auto elevation { static_cast<std::uint8_t>(
            std::clamp(static_cast<int>((clampedAngle + 1.57079f) / 3.14159f * 8.0f), 0, 7)) };
        return toEgaMechaSpriteId(static_cast<std::uint8_t>(base + elevation));
    }

    if (inAir) {
        return (facing == FacingDirection::Left) ? toEgaMechaSpriteId(8U) : toEgaMechaSpriteId(0U);
    }

    if (!moving) {
        return (facing == FacingDirection::Left) ? toEgaMechaSpriteId(8U) : toEgaMechaSpriteId(0U);
    }

    // Moving on ground: 8-frame walk cycle
    const auto walkBase { static_cast<std::uint8_t>((facing == FacingDirection::Left) ? 8U : 0U) };
    return toEgaMechaSpriteId(static_cast<std::uint8_t>(walkBase + (walkIndex8_ % 8U)));
}

} // namespace ShooterCore
