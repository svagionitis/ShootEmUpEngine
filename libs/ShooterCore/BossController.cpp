#include "BossController.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <cmath>

#include "AudioCommandQueue.h"
#include "EntityPool.h"
#include "Hitbox.h"
#include "Logger.h"
#include "ParticlePool.h"
#include "RenderCommandQueue.h"
#include "Types.h"

namespace ShooterCore {

BossController::BossController() noexcept
{
    initSegments();
}

void BossController::initSegments() noexcept
{
    // 0: Central Core
    segments_[0].type = BossSegmentType::CentralCore;
    segments_[0].relativeOffset = WorldPos { 0.0f, 0.0f };
    segments_[0].bounds = Hitbox { { -16.0f, -16.0f }, { 16.0f, 16.0f } };
    segments_[0].health = BossHealth { 200U, 200U };
    segments_[0].isArmored = false;

    // 1: Left Armor Plate
    segments_[1].type = BossSegmentType::LeftArmorPlate;
    segments_[1].relativeOffset = WorldPos { -24.0f, 0.0f };
    segments_[1].bounds = Hitbox { { -10.0f, -20.0f }, { 10.0f, 20.0f } };
    segments_[1].health = BossHealth { 150U, 150U };
    segments_[1].isArmored = true;

    // 2: Right Armor Plate
    segments_[2].type = BossSegmentType::RightArmorPlate;
    segments_[2].relativeOffset = WorldPos { 24.0f, 0.0f };
    segments_[2].bounds = Hitbox { { -10.0f, -20.0f }, { 10.0f, 20.0f } };
    segments_[2].health = BossHealth { 150U, 150U };
    segments_[2].isArmored = true;

    // 3: Top Turret
    segments_[3].type = BossSegmentType::TopTurret;
    segments_[3].relativeOffset = WorldPos { 0.0f, -24.0f };
    segments_[3].bounds = Hitbox { { -8.0f, -8.0f }, { 8.0f, 8.0f } };
    segments_[3].health = BossHealth { 80U, 80U };
    segments_[3].isArmored = false;

    // 4: Bottom Turret
    segments_[4].type = BossSegmentType::BottomTurret;
    segments_[4].relativeOffset = WorldPos { 0.0f, 24.0f };
    segments_[4].bounds = Hitbox { { -8.0f, -8.0f }, { 8.0f, 8.0f } };
    segments_[4].health = BossHealth { 80U, 80U };
    segments_[4].isArmored = false;
}

void BossController::spawn(WorldPos spawnPos) noexcept
{
    rootPos_ = spawnPos;
    initialY_ = spawnPos.y;
    velocity_ = Velocity { 0.0f, 0.0f };
    hoverTimer_ = 0.0f;
    attackTimer_ = 0.0f;
    phase_ = BossPhase::ShieldedBarrage;
    initSegments();
    LOG(INFO) << "BossController: Dreadnought Guardian spawned at (" << spawnPos.x << ", " << spawnPos.y << ").";
}

void BossController::update(DeltaTime dt, WorldPos playerPos, EntityPool& entities, ParticlePool& particles,
    AudioCommandQueue& audioQueue) noexcept
{
    static_cast<void>(entities);
    static_cast<void>(particles);

    if (phase_ == BossPhase::Dormant || phase_ == BossPhase::Defeated) {
        return;
    }

    hoverTimer_ += dt.seconds();
    attackTimer_ += dt.seconds();

    switch (phase_) {
    case BossPhase::ShieldedBarrage:
        updatePhase1(dt, audioQueue);
        break;
    case BossPhase::CoreExposure:
        updatePhase2(dt, audioQueue);
        break;
    case BossPhase::EnragedDesperation:
        updatePhase3(dt, playerPos, audioQueue);
        break;
    case BossPhase::Dormant:
    case BossPhase::Defeated:
    default:
        break;
    }
}

void BossController::updatePhase1(DeltaTime dt, AudioCommandQueue& audio) noexcept
{
    static_cast<void>(dt);
    // Smooth vertical sinusoidal hover
    rootPos_.y = initialY_ + std::sin(hoverTimer_ * 1.5f) * 32.0f;

    if (attackTimer_ >= 2.0f) {
        attackTimer_ = 0.0f;
        audio.pushSound(SoundEffectId::EnemyShoot);
    }
}

void BossController::updatePhase2(DeltaTime dt, AudioCommandQueue& audio) noexcept
{
    static_cast<void>(dt);
    // Faster sinusoidal hover
    rootPos_.y = initialY_ + std::sin(hoverTimer_ * 2.5f) * 44.0f;

    if (attackTimer_ >= 1.5f) {
        attackTimer_ = 0.0f;
        audio.pushSound(SoundEffectId::LaserFire);
    }
}

void BossController::updatePhase3(DeltaTime dt, WorldPos playerPos, AudioCommandQueue& audio) noexcept
{
    // Erratic horizontal tracking
    const float dx { playerPos.x - rootPos_.x };
    const float targetVx { (dx > 0.0f) ? 50.0f : -50.0f };
    velocity_.dx += (targetVx - velocity_.dx) * std::min(1.0f, dt.seconds() * 4.0f);
    rootPos_.x += velocity_.dx * dt.seconds();

    rootPos_.y = initialY_ + std::sin(hoverTimer_ * 3.5f) * 48.0f;

    if (attackTimer_ >= 1.0f) {
        attackTimer_ = 0.0f;
        audio.pushSound(SoundEffectId::EnemyShoot);
    }
}

bool BossController::applyDamage(const Hitbox& hitArea, std::uint16_t damage, AudioCommandQueue& audio) noexcept
{
    if (phase_ == BossPhase::Dormant || phase_ == BossPhase::Defeated) {
        return false;
    }

    bool hitAny { false };

    // Check armor plates and turrets first
    for (std::size_t i { 1U }; i < kSegmentCount; ++i) {
        BossSegment& seg { segments_[i] };
        if (seg.health.isDestroyed()) {
            continue;
        }

        const Hitbox segWorldBox { WorldPos { rootPos_.x + seg.relativeOffset.x + seg.bounds.min().x,
                                       rootPos_.y + seg.relativeOffset.y + seg.bounds.min().y },
            WorldPos { rootPos_.x + seg.relativeOffset.x + seg.bounds.max().x,
                rootPos_.y + seg.relativeOffset.y + seg.bounds.max().y } };

        if (hitArea.intersects(segWorldBox)) {
            seg.health.applyDamage(damage);
            hitAny = true;
            audio.pushSound(SoundEffectId::LaserHit);

            if (seg.health.isDestroyed()) {
                audio.pushSound(SoundEffectId::EnemyExplode);
                LOG(INFO) << "BossController: Segment " << i << " destroyed!";

                // Check if both armor plates (1 and 2) are destroyed
                if (segments_[1].health.isDestroyed() && segments_[2].health.isDestroyed()) {
                    if (phase_ == BossPhase::ShieldedBarrage) {
                        phase_ = BossPhase::CoreExposure;
                        audio.pushSound(SoundEffectId::BossRoar);
                        LOG(INFO) << "BossController: Armor shattered! Core exposed!";
                    }
                }
            }
            return true;
        }
    }

    // Check Central Core
    BossSegment& core { segments_[0] };
    const Hitbox coreWorldBox { WorldPos { rootPos_.x + core.relativeOffset.x + core.bounds.min().x,
                                    rootPos_.y + core.relativeOffset.y + core.bounds.min().y },
        WorldPos { rootPos_.x + core.relativeOffset.x + core.bounds.max().x,
            rootPos_.y + core.relativeOffset.y + core.bounds.max().y } };

    if (hitArea.intersects(coreWorldBox)) {
        const bool armorIntact { !segments_[1].health.isDestroyed() || !segments_[2].health.isDestroyed() };

        if (armorIntact) {
            // Deflected by intact ablative armor plates
            audio.pushSound(SoundEffectId::ShieldDeflect);
            return true;
        }

        // Core is exposed: takes direct damage
        core.health.applyDamage(damage);
        hitAny = true;
        audio.pushSound(SoundEffectId::PlayerDamage);

        if (core.health.isDestroyed()) {
            phase_ = BossPhase::Defeated;
            audio.pushSound(SoundEffectId::BossExplode);
            LOG(INFO) << "BossController: Dreadnought Guardian defeated!";
        } else if (core.health.current() <= 60U && phase_ != BossPhase::EnragedDesperation) {
            phase_ = BossPhase::EnragedDesperation;
            audio.pushSound(SoundEffectId::BossRoar);
            LOG(INFO) << "BossController: Core critical! Entered EnragedDesperation phase!";
        }
        return true;
    }

    return hitAny;
}

void BossController::emitRender(RenderCommandQueue& renderQueue) const noexcept
{
    if (phase_ == BossPhase::Dormant || phase_ == BossPhase::Defeated) {
        return;
    }

    for (std::size_t i { 0U }; i < kSegmentCount; ++i) {
        const BossSegment& seg { segments_[i] };
        if (seg.health.isDestroyed()) {
            continue;
        }

        const WorldPos origin { rootPos_.x + seg.relativeOffset.x + seg.bounds.min().x,
            rootPos_.y + seg.relativeOffset.y + seg.bounds.min().y };

        ColorRgba color { };
        if (seg.type == BossSegmentType::CentralCore) {
            if (phase_ == BossPhase::EnragedDesperation) {
                color = ColorRgba { 255U, 30U, 30U, 255U }; // Flashing critical red
            } else if (phase_ == BossPhase::CoreExposure) {
                color = ColorRgba { 255U, 180U, 40U, 255U }; // Exposed yellow-orange core
            } else {
                color = ColorRgba { 80U, 160U, 240U, 255U }; // Shielded blue core
            }
        } else if (seg.isArmored) {
            color = ColorRgba { 120U, 130U, 150U, 255U }; // Armored steel plates
        } else {
            color = ColorRgba { 180U, 60U, 60U, 255U }; // Red weapon turrets
        }

        renderQueue.pushBar(origin, seg.bounds.width(), seg.bounds.height(), 1.0f, color);
    }
}

} // namespace ShooterCore
