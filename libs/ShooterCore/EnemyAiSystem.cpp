#include "EnemyAiSystem.h"

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

void EnemyAiSystem::update(DeltaTime dt, WorldPos playerPos, Hitbox playerBox, bool shieldActive,
    EnergyPoints& playerEnergy, EntityPool& pool, const TileMap& map, AudioCommandQueue& audio,
    ParticlePool& particles) noexcept
{
    const float dtSec { dt.seconds() };
    if (dtSec <= 0.0f) {
        return;
    }

    updateInterceptors(dtSec, playerPos, pool, map);
    updateCrawlers(dtSec, pool, map);
    updateTurrets(dtSec, playerPos, pool, map, audio, particles);
    updateHostileBullets(dtSec, playerBox, shieldActive, playerEnergy, pool, map, audio, particles);
}

void EnemyAiSystem::updateInterceptors(float dtSec, WorldPos playerPos, EntityPool& pool, const TileMap& map) noexcept
{
    for (std::size_t i { 0U }; i < pool.capacity(); ++i) {
        const EntitySlot& slot { pool.slot(i) };
        if (!slot.active) {
            continue;
        }
        Entity* e { pool.get(EntityId { static_cast<std::uint16_t>(i), slot.generation }) };
        if (e == nullptr || e->type != EntityType::DivingInterceptor || e->health == 0U) {
            continue;
        }

        const float dx { playerPos.x - e->pos.x };
        const float dy { playerPos.y - e->pos.y };
        const float dist { std::sqrt((dx * dx) + (dy * dy)) };

        if (e->stateTimer == 0.0f) {
            // Cruise / Stalk Mode
            if (std::abs(e->vel.dx) < 1.0f) {
                e->vel.dx = e->facing == FacingDirection::Left ? -80.0f : 80.0f;
            }

            // Check if player is detected in downward sector
            if (dist <= kInterceptorDetectDist && dy > 15.0f && std::abs(dx) < 140.0f) {
                e->stateTimer = 1.0f; // Switch to Swoop
            }
        }

        if (e->stateTimer > 0.0f) {
            // High-Speed Swoop Attack Mode
            if (dist > 1.0f) {
                const float desiredDx { (dx / dist) * kInterceptorSwoopSpeed };
                const float desiredDy { (dy / dist) * kInterceptorSwoopSpeed };
                e->vel.dx += (desiredDx - e->vel.dx) * std::min(1.0f, 5.0f * dtSec);
                e->vel.dy += (desiredDy - e->vel.dy) * std::min(1.0f, 5.0f * dtSec);
            }

            // Recovery when diving past the player
            if (e->pos.y > playerPos.y + 40.0f) {
                e->vel.dy = -120.0f;
                e->stateTimer = 0.0f; // Return to cruise
            }
        }

        e->pos.x += e->vel.dx * dtSec;
        e->pos.y += e->vel.dy * dtSec;

        e->bounds
            = Hitbox { WorldPos { e->pos.x - 8.0f, e->pos.y - 8.0f }, WorldPos { e->pos.x + 8.0f, e->pos.y + 8.0f } };

        // Terrain bounce check
        const TileCoord tile { e->pos.toTileCoord(16.0f) };
        if (map.isSolid(tile)) {
            e->vel.dx = -e->vel.dx;
            e->vel.dy = -80.0f;
            e->stateTimer = 0.0f;
        }

        e->facing = e->vel.dx < 0.0f ? FacingDirection::Left : FacingDirection::Right;
    }
}

void EnemyAiSystem::updateCrawlers(float dtSec, EntityPool& pool, const TileMap& map) noexcept
{
    for (std::size_t i { 0U }; i < pool.capacity(); ++i) {
        const EntitySlot& slot { pool.slot(i) };
        if (!slot.active) {
            continue;
        }
        Entity* e { pool.get(EntityId { static_cast<std::uint16_t>(i), slot.generation }) };
        if (e == nullptr || e->type != EntityType::WallCrawler || e->health == 0U) {
            continue;
        }

        // e->stateTimer encodes surface normal:
        // 0 = Floor (moving horiz), 1 = RightWall (moving vert),
        // 2 = Ceiling (moving horiz), 3 = LeftWall (moving vert)
        const auto normalMode { static_cast<std::uint8_t>(e->stateTimer) % 4U };

        float moveDx { 0.0f };
        float moveDy { 0.0f };

        if (normalMode == 0U) {
            // Floor
            moveDx = e->facing == FacingDirection::Left ? -kCrawlerSpeed : kCrawlerSpeed;
            const TileCoord frontTile { WorldPos { e->pos.x + (moveDx > 0.0f ? 9.0f : -9.0f), e->pos.y }.toTileCoord(
                16.0f) };
            if (map.isSolid(frontTile)) {
                // Inside corner: rotate 90 deg up wall
                e->stateTimer = (e->facing == FacingDirection::Right) ? 1.0f : 3.0f;
            }
        } else if (normalMode == 1U) {
            // Right Wall (climbing up)
            moveDy = -kCrawlerSpeed;
            const TileCoord frontTile { WorldPos { e->pos.x, e->pos.y - 9.0f }.toTileCoord(16.0f) };
            if (map.isSolid(frontTile)) {
                // Inside ceiling corner
                e->stateTimer = 2.0f;
                e->facing = FacingDirection::Left;
            }
        } else if (normalMode == 2U) {
            // Ceiling
            moveDx = e->facing == FacingDirection::Left ? -kCrawlerSpeed : kCrawlerSpeed;
            const TileCoord frontTile { WorldPos { e->pos.x + (moveDx > 0.0f ? 9.0f : -9.0f), e->pos.y }.toTileCoord(
                16.0f) };
            if (map.isSolid(frontTile)) {
                e->stateTimer = 3.0f;
            }
        } else {
            // Left Wall (climbing down)
            moveDy = kCrawlerSpeed;
            const TileCoord frontTile { WorldPos { e->pos.x, e->pos.y + 9.0f }.toTileCoord(16.0f) };
            if (map.isSolid(frontTile)) {
                e->stateTimer = 0.0f;
                e->facing = FacingDirection::Right;
            }
        }

        e->pos.x += moveDx * dtSec;
        e->pos.y += moveDy * dtSec;

        e->bounds
            = Hitbox { WorldPos { e->pos.x - 8.0f, e->pos.y - 8.0f }, WorldPos { e->pos.x + 8.0f, e->pos.y + 8.0f } };
    }
}

void EnemyAiSystem::updateTurrets(float dtSec, WorldPos playerPos, EntityPool& pool, const TileMap& map,
    AudioCommandQueue& audio, ParticlePool& particles) noexcept
{
    (void)map;

    for (std::size_t i { 0U }; i < pool.capacity(); ++i) {
        const EntitySlot& slot { pool.slot(i) };
        if (!slot.active) {
            continue;
        }
        Entity* e { pool.get(EntityId { static_cast<std::uint16_t>(i), slot.generation }) };
        if (e == nullptr || e->type != EntityType::CeilingTurret || e->health == 0U) {
            continue;
        }

        const float dx { playerPos.x - e->pos.x };
        const float dy { playerPos.y - e->pos.y };
        const float dist { std::sqrt((dx * dx) + (dy * dy)) };

        // Turret only fires downward at player within range
        if (dy > 10.0f && dist <= kTurretRange) {
            e->stateTimer += dtSec;
            if (e->stateTimer >= kTurretFireInterval) {
                e->stateTimer = 0.0f;

                if (!pool.isFull()) {
                    const Hitbox bulletBox { WorldPos { e->pos.x - 3.0f, e->pos.y + 6.0f },
                        WorldPos { e->pos.x + 3.0f, e->pos.y + 12.0f } };

                    const EntityId bId { pool.spawn(
                        EntityType::HostileBullet, WorldPos { e->pos.x, e->pos.y + 8.0f }, bulletBox, 1U) };

                    if (bId.isValid()) {
                        Entity* bullet { pool.get(bId) };
                        if (bullet != nullptr) {
                            bullet->vel = Velocity { (dx / dist) * kBulletSpeed, (dy / dist) * kBulletSpeed };
                        }
                        audio.pushSound(SoundEffectId::EnemyShoot);
                        particles.emitSparks(WorldPos { e->pos.x, e->pos.y + 8.0f }, 1.57f, 4U);
                    }
                }
            }
        }
    }
}

void EnemyAiSystem::updateHostileBullets(float dtSec, Hitbox playerBox, bool shieldActive, EnergyPoints& playerEnergy,
    EntityPool& pool, const TileMap& map, AudioCommandQueue& audio, ParticlePool& particles) noexcept
{
    for (std::size_t i { 0U }; i < pool.capacity(); ++i) {
        const EntitySlot& slot { pool.slot(i) };
        if (!slot.active) {
            continue;
        }
        const EntityId bId { static_cast<std::uint16_t>(i), slot.generation };
        Entity* bullet { pool.get(bId) };
        if (bullet == nullptr || bullet->type != EntityType::HostileBullet || bullet->health == 0U) {
            continue;
        }

        bullet->pos.x += bullet->vel.dx * dtSec;
        bullet->pos.y += bullet->vel.dy * dtSec;

        bullet->bounds = Hitbox { WorldPos { bullet->pos.x - 3.0f, bullet->pos.y - 3.0f },
            WorldPos { bullet->pos.x + 3.0f, bullet->pos.y + 3.0f } };

        // Check Terrain Impact
        const TileCoord tile { bullet->pos.toTileCoord(16.0f) };
        if (map.isSolid(tile)) {
            bullet->health = 0U;
            particles.emitSparks(bullet->pos, 0.0f, 6U);
            pool.despawn(bId);
            continue;
        }

        // Check Player Impact
        if (playerBox.intersects(bullet->bounds)) {
            bullet->health = 0U;
            if (shieldActive) {
                audio.pushSound(SoundEffectId::ShieldDeflect);
                particles.emitShieldSparks(bullet->pos, 0.0f, 8U);
            } else {
                playerEnergy.deduct(EnergyPoints { kBulletDamage });
                audio.pushSound(SoundEffectId::PlayerDamage);
                particles.emitSparks(bullet->pos, 0.0f, 8U);
            }
            pool.despawn(bId);
        }
    }
}

} // namespace ShooterCore
