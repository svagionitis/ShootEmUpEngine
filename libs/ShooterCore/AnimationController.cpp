#include "AnimationController.h"

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
    exhaustTimer_ = 0.0f;
    enemyTimer_ = 0.0f;
    walkIndex_ = 0U;
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

} // namespace ShooterCore
