#include "SecondaryArsenal.h"

#include <cmath>
#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <array>

#include "AudioCommandQueue.h"
#include "EntityPool.h"
#include "Hitbox.h"
#include "ParticlePool.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore {

namespace {
    constexpr float kDegToRad { 0.0174532925f };
    constexpr float kRadToDeg { 57.2957795f };

    [[nodiscard]] float normalizeAngleDeg(float deg) noexcept
    {
        while (deg > 180.0f) {
            deg -= 360.0f;
        }
        while (deg < -180.0f) {
            deg += 360.0f;
        }
        return deg;
    }
}

void SecondaryArsenal::cycleWeapon() noexcept
{
    if (activeWeapon_ == SecondaryWeaponType::None) {
        activeWeapon_ = SecondaryWeaponType::HomingMissile;
    } else if (activeWeapon_ == SecondaryWeaponType::HomingMissile) {
        activeWeapon_ = SecondaryWeaponType::FlashBomb;
    } else {
        activeWeapon_ = SecondaryWeaponType::None;
    }
}

void SecondaryArsenal::setWeapon(SecondaryWeaponType type) noexcept
{
    activeWeapon_ = type;
}

void SecondaryArsenal::replenishMissiles(std::uint8_t count) noexcept
{
    missileAmmo_.replenish(count);
}

void SecondaryArsenal::replenishBombs(std::uint8_t count) noexcept
{
    bombAmmo_.replenish(count);
}

bool SecondaryArsenal::triggerFire(WorldPos craftPos, FacingDirection facing, EntityPool& entities,
    AudioCommandQueue& audio, ParticlePool& particles) noexcept
{
    if (fireCooldown_ > 0.0f) {
        return false;
    }

    if (activeWeapon_ == SecondaryWeaponType::HomingMissile) {
        if (!missileAmmo_.hasAmmo()) {
            return false;
        }

        // Find free missile slot
        std::size_t freeIdx { kMaxMissiles };
        for (std::size_t i { 0U }; i < kMaxMissiles; ++i) {
            if (!missiles_[i].active) {
                freeIdx = i;
                break;
            }
        }

        if (freeIdx >= kMaxMissiles) {
            return false;
        }

        missileAmmo_.consume();
        HomingMissile& m { missiles_[freeIdx] };
        m.pos = WorldPos { craftPos.x, craftPos.y - 12.0f };
        const float dirSign { facing == FacingDirection::Left ? -1.0f : 1.0f };
        m.vel = Velocity { dirSign * kInitialMissileSpeed, -30.0f };
        m.angleDeg = facing == FacingDirection::Left ? 180.0f : 0.0f;
        m.fuelSec = kMissileFuelTime;
        m.smokeTimer = 0.0f;
        m.targetId = findMissileTarget(m.pos, entities);
        m.active = true;

        ++activeMissileCount_;
        fireCooldown_ = kMissileFireCooldown;
        audio.pushSound(SoundEffectId::MissileLaunch);
        particles.emitSmoke(m.pos);
        return true;
    }

    if (activeWeapon_ == SecondaryWeaponType::FlashBomb) {
        if (!bombAmmo_.hasAmmo() || flashBomb_.active) {
            return false;
        }

        bombAmmo_.consume();
        flashBomb_.active = true;
        flashBomb_.center = craftPos;
        flashBomb_.currentRadius = 16.0f;
        flashBomb_.maxRadius = 320.0f;
        flashBomb_.timer = 0.0f;
        flashBomb_.duration = kFlashBombDuration;

        fireCooldown_ = 0.5f;
        audio.pushSound(SoundEffectId::FlashBombExplode);
        particles.emitShockwave(craftPos, 24U);
        return true;
    }

    return false;
}

EntityId SecondaryArsenal::findMissileTarget(WorldPos missilePos, const EntityPool& entities) const noexcept
{
    EntityId bestId { };
    float bestDistSq { 320.0f * 320.0f };

    for (std::size_t i { 0U }; i < entities.capacity(); ++i) {
        const EntitySlot& slot { entities.slot(i) };
        if (!slot.active) {
            continue;
        }
        const Entity& e { slot.data };
        if (e.type == EntityType::Player || e.type == EntityType::Particle || e.type == EntityType::CargoCanister) {
            continue;
        }
        if (e.health == 0U) {
            continue;
        }

        const float dx { e.pos.x - missilePos.x };
        const float dy { e.pos.y - missilePos.y };
        const float distSq { (dx * dx) + (dy * dy) };

        if (distSq < bestDistSq) {
            bestDistSq = distSq;
            bestId = EntityId { static_cast<std::uint16_t>(i), slot.generation };
        }
    }

    return bestId;
}

void SecondaryArsenal::explodeMissile(
    std::size_t idx, EntityPool& entities, TileMap& map, ParticlePool& particles, AudioCommandQueue& audio) noexcept
{
    HomingMissile& m { missiles_[idx] };
    m.active = false;
    if (activeMissileCount_ > 0U) {
        --activeMissileCount_;
    }

    // Emit impact sparks and debris
    particles.emitSparks(m.pos, 0.0f, 16U);
    audio.pushSound(SoundEffectId::EnemyExplode);

    // Apply splash damage to hostiles within splash radius
    for (std::size_t i { 0U }; i < entities.capacity(); ++i) {
        const EntitySlot& slot { entities.slot(i) };
        if (!slot.active) {
            continue;
        }
        Entity& e { *entities.get(EntityId { static_cast<std::uint16_t>(i), slot.generation }) };
        if (e.type == EntityType::Player || e.type == EntityType::Particle || e.type == EntityType::CargoCanister) {
            continue;
        }
        if (e.health == 0U) {
            continue;
        }

        const float dx { e.pos.x - m.pos.x };
        const float dy { e.pos.y - m.pos.y };
        const float distSq { (dx * dx) + (dy * dy) };
        if (distSq <= (kMissileSplashRadius * kMissileSplashRadius)) {
            if (e.health <= kMissileSplashDamage) {
                e.health = 0U;
            } else {
                e.health = static_cast<std::uint16_t>(e.health - kMissileSplashDamage);
            }
        }
    }

    // Damage destructible walls in adjacent tiles
    const TileCoord centerTile { m.pos.toTileCoord(16.0f) };
    for (std::int32_t r { -1 }; r <= 1; ++r) {
        for (std::int32_t c { -1 }; c <= 1; ++c) {
            const auto checkRow { static_cast<std::int32_t>(centerTile.row) + r };
            const auto checkCol { static_cast<std::int32_t>(centerTile.col) + c };
            if (checkRow >= 0 && checkCol >= 0) {
                const TileCoord tc { static_cast<std::uint32_t>(checkCol), static_cast<std::uint32_t>(checkRow) };
                if (map.getTile(tc) == TileType::DestructibleWall) {
                    map.damageTile(tc);
                }
            }
        }
    }
}

void SecondaryArsenal::update(
    DeltaTime dt, EntityPool& entities, TileMap& map, ParticlePool& particles, AudioCommandQueue& audio) noexcept
{
    const float dtSec { dt.seconds() };
    if (dtSec <= 0.0f) {
        return;
    }

    if (fireCooldown_ > 0.0f) {
        fireCooldown_ = std::max(0.0f, fireCooldown_ - dtSec);
    }

    // 1. Advance Homing Missiles
    for (std::size_t i { 0U }; i < kMaxMissiles; ++i) {
        HomingMissile& m { missiles_[i] };
        if (!m.active) {
            continue;
        }

        m.fuelSec -= dtSec;
        if (m.fuelSec <= 0.0f) {
            explodeMissile(i, entities, map, particles, audio);
            continue;
        }

        m.smokeTimer += dtSec;
        if (m.smokeTimer >= 0.05f) {
            m.smokeTimer = 0.0f;
            particles.emitSmoke(m.pos);
        }

        // Target tracking & proportional navigation
        if (!entities.isValid(m.targetId)) {
            m.targetId = findMissileTarget(m.pos, entities);
        } else {
            const Entity* target { entities.get(m.targetId) };
            if (target == nullptr || target->health == 0U) {
                m.targetId = findMissileTarget(m.pos, entities);
            }
        }

        if (entities.isValid(m.targetId)) {
            const Entity* target { entities.get(m.targetId) };
            if (target != nullptr) {
                const float dx { target->pos.x - m.pos.x };
                const float dy { target->pos.y - m.pos.y };
                const float desiredAngleDeg { std::atan2(dy, dx) * kRadToDeg };
                const float angleDiff { normalizeAngleDeg(desiredAngleDeg - m.angleDeg) };
                const float maxTurn { kMissileTurnRateDeg * dtSec };
                m.angleDeg += std::clamp(angleDiff, -maxTurn, maxTurn);
                m.angleDeg = normalizeAngleDeg(m.angleDeg);
            }
        }

        // Accelerate along angle
        const float angleRad { m.angleDeg * kDegToRad };
        const float currentSpeed { std::sqrt((m.vel.dx * m.vel.dx) + (m.vel.dy * m.vel.dy)) };
        const float newSpeed { std::min(kMissileTopSpeed, currentSpeed + (kMissileThrust * dtSec)) };
        m.vel.dx = std::cos(angleRad) * newSpeed;
        m.vel.dy = std::sin(angleRad) * newSpeed;

        m.pos.x += m.vel.dx * dtSec;
        m.pos.y += m.vel.dy * dtSec;

        // Bounding box collision against hostile entities
        const Hitbox missileBox { WorldPos { m.pos.x - 3.0f, m.pos.y - 3.0f },
            WorldPos { m.pos.x + 3.0f, m.pos.y + 3.0f } };

        bool collided { false };
        for (std::size_t eIdx { 0U }; eIdx < entities.capacity(); ++eIdx) {
            const EntitySlot& slot { entities.slot(eIdx) };
            if (!slot.active) {
                continue;
            }
            Entity& e { *entities.get(EntityId { static_cast<std::uint16_t>(eIdx), slot.generation }) };
            if (e.type == EntityType::Player || e.type == EntityType::Particle || e.type == EntityType::CargoCanister) {
                continue;
            }
            if (e.health == 0U) {
                continue;
            }

            if (missileBox.intersects(e.bounds)) {
                if (e.health <= kMissileDirectDamage) {
                    e.health = 0U;
                } else {
                    e.health = static_cast<std::uint16_t>(e.health - kMissileDirectDamage);
                }
                collided = true;
                break;
            }
        }

        if (collided) {
            explodeMissile(i, entities, map, particles, audio);
            continue;
        }

        // Terrain collision
        const TileCoord tile { m.pos.toTileCoord(16.0f) };
        if (map.isSolid(tile)) {
            explodeMissile(i, entities, map, particles, audio);
            continue;
        }
    }

    // 2. Advance Screen-Clearing Flash Bomb
    if (flashBomb_.active) {
        flashBomb_.timer += dtSec;
        flashBomb_.currentRadius += kFlashBombSpeed * dtSec;

        // Damage hostiles within radius
        const float rSq { flashBomb_.currentRadius * flashBomb_.currentRadius };
        for (std::size_t i { 0U }; i < entities.capacity(); ++i) {
            const EntitySlot& slot { entities.slot(i) };
            if (!slot.active) {
                continue;
            }
            Entity& e { *entities.get(EntityId { static_cast<std::uint16_t>(i), slot.generation }) };
            if (e.type == EntityType::Player || e.type == EntityType::Particle || e.type == EntityType::CargoCanister) {
                continue;
            }
            if (e.health == 0U) {
                continue;
            }

            const float dx { e.pos.x - flashBomb_.center.x };
            const float dy { e.pos.y - flashBomb_.center.y };
            const float distSq { (dx * dx) + (dy * dy) };

            if (distSq <= rSq) {
                if (e.health <= kFlashBombDamage) {
                    e.health = 0U;
                } else {
                    e.health = static_cast<std::uint16_t>(e.health - kFlashBombDamage);
                }
            }
        }

        // Disintegrate destructible walls within radius
        const std::int32_t tileRadius { static_cast<std::int32_t>(flashBomb_.currentRadius / 16.0f) };
        const TileCoord centerTile { flashBomb_.center.toTileCoord(16.0f) };

        for (std::int32_t r { -tileRadius }; r <= tileRadius; ++r) {
            for (std::int32_t c { -tileRadius }; c <= tileRadius; ++c) {
                const auto checkRow { static_cast<std::int32_t>(centerTile.row) + r };
                const auto checkCol { static_cast<std::int32_t>(centerTile.col) + c };
                if (checkRow >= 0 && checkCol >= 0) {
                    const TileCoord tc { static_cast<std::uint32_t>(checkCol), static_cast<std::uint32_t>(checkRow) };
                    if (map.getTile(tc) == TileType::DestructibleWall) {
                        const float dx { (static_cast<float>(tc.col) * 16.0f + 8.0f) - flashBomb_.center.x };
                        const float dy { (static_cast<float>(tc.row) * 16.0f + 8.0f) - flashBomb_.center.y };
                        if (((dx * dx) + (dy * dy)) <= rSq) {
                            map.setTile(tc, TileType::Empty);
                            particles.emitDebris(WorldPos { static_cast<float>(tc.col) * 16.0f + 8.0f,
                                                     static_cast<float>(tc.row) * 16.0f + 8.0f },
                                4U);
                        }
                    }
                }
            }
        }

        if (flashBomb_.timer >= flashBomb_.duration || flashBomb_.currentRadius >= flashBomb_.maxRadius) {
            flashBomb_.active = false;
        }
    }
}

void SecondaryArsenal::clear() noexcept
{
    for (std::size_t i { 0U }; i < kMaxMissiles; ++i) {
        missiles_[i] = HomingMissile { };
    }
    activeMissileCount_ = 0U;
    flashBomb_ = FlashBombState { };
    fireCooldown_ = 0.0f;
}

} // namespace ShooterCore
