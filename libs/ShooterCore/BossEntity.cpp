#include "BossEntity.h"

#include <cmath>
#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <array>

#include "AudioCommandQueue.h"
#include "EntityPool.h"
#include "Hitbox.h"
#include "ParticlePool.h"
#include "Types.h"

namespace ShooterCore {

const BossPart& BossEntity::getPart(std::size_t idx) const noexcept
{
    static constexpr BossPart kDummyPart { };
    if (idx >= partCount_) {
        return kDummyPart;
    }
    return parts_[idx];
}

std::uint8_t BossEntity::coreHealthPercent() const noexcept
{
    if (partCount_ == 0U || parts_[0].maxHp == 0U) {
        return 0U;
    }
    const auto pct { static_cast<std::uint32_t>(
        (static_cast<std::uint32_t>(parts_[0].currentHp) * 100U) / static_cast<std::uint32_t>(parts_[0].maxHp)) };
    return static_cast<std::uint8_t>(std::min(100U, pct));
}

void BossEntity::init(BossArchetype archetype, WorldPos spawnPos) noexcept
{
    archetype_ = archetype;
    rootPos_ = spawnPos;
    vel_ = Velocity { 0.0f, 0.0f };
    hoverTimer_ = 0.0f;
    attackTimer_ = 0.0f;
    deathTimer_ = 0.0f;
    phase_ = BossPhase::ShieldedBarrage;

    for (std::size_t i { 0U }; i < kMaxParts; ++i) {
        parts_[i] = BossPart { };
    }

    if (archetype == BossArchetype::OrbitalCruiser) {
        // Part 0: Central Armored Power Core
        parts_[0].type = BossPartType::WeakCore;
        parts_[0].offset = WorldPos { 0.0f, 0.0f };
        parts_[0].bounds = Hitbox { { -20.0f, -20.0f }, { 20.0f, 20.0f } };
        parts_[0].maxHp = 500U;
        parts_[0].currentHp = 500U;
        parts_[0].isArmored = true;

        // Part 1: Left Ion Thruster
        parts_[1].type = BossPartType::Thruster;
        parts_[1].offset = WorldPos { -48.0f, -24.0f };
        parts_[1].bounds = Hitbox { { -16.0f, -12.0f }, { 16.0f, 12.0f } };
        parts_[1].maxHp = 200U;
        parts_[1].currentHp = 200U;
        parts_[1].isArmored = false;

        // Part 2: Right Ion Thruster
        parts_[2].type = BossPartType::Thruster;
        parts_[2].offset = WorldPos { -48.0f, 24.0f };
        parts_[2].bounds = Hitbox { { -16.0f, -12.0f }, { 16.0f, 12.0f } };
        parts_[2].maxHp = 200U;
        parts_[2].currentHp = 200U;
        parts_[2].isArmored = false;

        // Part 3: Top Laser Turret
        parts_[3].type = BossPartType::Turret;
        parts_[3].offset = WorldPos { 0.0f, -32.0f };
        parts_[3].bounds = Hitbox { { -12.0f, -12.0f }, { 12.0f, 12.0f } };
        parts_[3].maxHp = 150U;
        parts_[3].currentHp = 150U;
        parts_[3].isArmored = false;

        // Part 4: Bottom Laser Turret
        parts_[4].type = BossPartType::Turret;
        parts_[4].offset = WorldPos { 0.0f, 32.0f };
        parts_[4].bounds = Hitbox { { -12.0f, -12.0f }, { 12.0f, 12.0f } };
        parts_[4].maxHp = 150U;
        parts_[4].currentHp = 150U;
        parts_[4].isArmored = false;

        partCount_ = 5U;
    } else {
        // Generic fallback profile
        parts_[0].type = BossPartType::WeakCore;
        parts_[0].offset = WorldPos { 0.0f, 0.0f };
        parts_[0].bounds = Hitbox { { -24.0f, -24.0f }, { 24.0f, 24.0f } };
        parts_[0].maxHp = 400U;
        parts_[0].currentHp = 400U;
        parts_[0].isArmored = false;
        partCount_ = 1U;
    }
}

bool BossEntity::applyDamage(const Hitbox& hitArea, std::uint16_t damage, AudioCommandQueue& audio) noexcept
{
    if (phase_ == BossPhase::Defeated || phase_ == BossPhase::Dormant) {
        return false;
    }

    bool hitAny { false };
    for (std::size_t i { 0U }; i < partCount_; ++i) {
        BossPart& part { parts_[i] };
        if (part.isDestroyed) {
            continue;
        }

        const WorldPos partCenter { rootPos_.x + part.offset.x, rootPos_.y + part.offset.y };
        const Hitbox worldBounds { WorldPos { partCenter.x + part.bounds.min().x, partCenter.y + part.bounds.min().y },
            WorldPos { partCenter.x + part.bounds.max().x, partCenter.y + part.bounds.max().y } };

        if (worldBounds.intersects(hitArea)) {
            hitAny = true;
            if (part.isArmored) {
                audio.pushSound(SoundEffectId::ShieldDeflect);
                continue;
            }

            if (part.currentHp <= damage) {
                part.currentHp = 0U;
                part.isDestroyed = true;
                audio.pushSound(SoundEffectId::EnemyExplode);

                // If core destroyed, trigger detonation sequence
                if (part.type == BossPartType::WeakCore) {
                    phase_ = BossPhase::Defeated; // Triggers end of boss
                    audio.pushSound(SoundEffectId::BossExplode);
                }
            } else {
                part.currentHp = static_cast<std::uint16_t>(part.currentHp - damage);
                audio.pushSound(SoundEffectId::LaserHit);
            }

            // Check if all secondary modules are destroyed -> unshield core
            bool modulesRemaining { false };
            for (std::size_t m { 1U }; m < partCount_; ++m) {
                if (!parts_[m].isDestroyed) {
                    modulesRemaining = true;
                    break;
                }
            }
            if (!modulesRemaining && partCount_ > 1U && phase_ != BossPhase::Defeated) {
                parts_[0].isArmored = false; // Cowl open!
                phase_ = BossPhase::CoreExposure;
            }
        }
    }

    return hitAny;
}

void BossEntity::update(
    DeltaTime dt, WorldPos playerPos, EntityPool& entities, ParticlePool& particles, AudioCommandQueue& audio) noexcept
{
    const float dtSec { dt.seconds() };
    if (dtSec <= 0.0f) {
        return;
    }

    if (phase_ == BossPhase::Defeated) {
        updateDetonation(dt, particles, audio);
        return;
    }

    if (phase_ == BossPhase::Dormant) {
        return;
    }

    if (archetype_ == BossArchetype::OrbitalCruiser) {
        updateOrbitalCruiser(dt, playerPos, entities, particles, audio);
    }
}

void BossEntity::updateOrbitalCruiser(
    DeltaTime dt, WorldPos playerPos, EntityPool& entities, ParticlePool& particles, AudioCommandQueue& audio) noexcept
{
    const float dtSec { dt.seconds() };
    hoverTimer_ += dtSec;
    attackTimer_ += dtSec;

    // Gentle vertical hovering oscillation
    rootPos_.y += std::sin(hoverTimer_ * 1.5f) * 20.0f * dtSec;

    // Left and right thruster particles
    if (!parts_[1].isDestroyed) {
        particles.emitSparks(WorldPos { rootPos_.x - 64.0f, rootPos_.y - 24.0f }, 180.0f, 4U);
    }
    if (!parts_[2].isDestroyed) {
        particles.emitSparks(WorldPos { rootPos_.x - 64.0f, rootPos_.y + 24.0f }, 180.0f, 4U);
    }

    // Periodic turret projectile fire toward player
    if (attackTimer_ >= 2.0f) {
        attackTimer_ = 0.0f;
        for (std::size_t i { 3U }; i <= 4U && i < partCount_; ++i) {
            if (!parts_[i].isDestroyed) {
                const WorldPos muzzle { rootPos_.x + parts_[i].offset.x, rootPos_.y + parts_[i].offset.y };
                const float dx { playerPos.x - muzzle.x };
                const float dy { playerPos.y - muzzle.y };
                const float dist { std::sqrt((dx * dx) + (dy * dy)) };
                if (dist > 1.0f) {
                    const float speed { 180.0f };
                    const EntityId bId { entities.spawn(
                        EntityType::HostileBullet, muzzle, Hitbox { { -3.0f, -3.0f }, { 3.0f, 3.0f } }, 10U) };
                    Entity* bullet { entities.get(bId) };
                    if (bullet != nullptr) {
                        bullet->vel = Velocity { (dx / dist) * speed, (dy / dist) * speed };
                    }
                }
            }
        }
        audio.pushSound(SoundEffectId::EnemyShoot);
    }
}

void BossEntity::updateDetonation(DeltaTime dt, ParticlePool& particles, AudioCommandQueue& audio) noexcept
{
    deathTimer_ += dt.seconds();
    if (deathTimer_ < 2.5f) {
        // Cascade of secondary detonations across parts
        for (std::size_t i { 0U }; i < partCount_; ++i) {
            const WorldPos burstPos { rootPos_.x + parts_[i].offset.x, rootPos_.y + parts_[i].offset.y };
            particles.emitSparks(burstPos, 0.0f, 8U);
        }
        audio.pushSound(SoundEffectId::EnemyExplode);
    }
}

} // namespace ShooterCore
