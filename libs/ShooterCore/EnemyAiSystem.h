#pragma once

#include <cstddef>
#include <cstdint>

#include "AudioCommandQueue.h"
#include "EntityPool.h"
#include "Hitbox.h"
#include "ParticlePool.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore {

/// @class EnemyAiSystem
/// @brief Orchestrates tactical AI state machines: Diving Interceptors, Wall Crawlers, and Ceiling Turrets.
/// @details Strictly zero-heap execution on the active simulation update loop (CERT ERR55-CPP).
class EnemyAiSystem final {
public:
    static constexpr float kInterceptorSwoopSpeed { 280.0f };
    static constexpr float kInterceptorDetectDist { 180.0f };
    static constexpr float kCrawlerSpeed { 50.0f };
    static constexpr float kTurretRange { 200.0f };
    static constexpr float kTurretFireInterval { 2.0f };
    static constexpr float kBulletSpeed { 140.0f };
    static constexpr std::uint16_t kBulletDamage { 15U };

    EnemyAiSystem() noexcept = default;

    /// @brief Primary simulation tick updating all active enemy AI behaviors.
    /// @param[in] dt Frame simulation delta time.
    /// @param[in] playerPos World position of player craft.
    /// @param[in] playerBox Hitbox of player craft.
    /// @param[in] shieldActive Whether player shield barrier is deployed.
    /// @param[in,out] playerEnergy Player energy pool to deduct damage if hit.
    /// @param[in,out] pool Entity pool containing active hostiles and projectiles.
    /// @param[in] map Tilemap for obstacle collisions and wall crawling.
    /// @param[in,out] audio Sound queue for bullet launches, deflections, and hits.
    /// @param[in,out] particles Particle pool for muzzle sparks and impact debris.
    void update(DeltaTime dt, WorldPos playerPos, Hitbox playerBox, bool shieldActive, EnergyPoints& playerEnergy,
        EntityPool& pool, const TileMap& map, AudioCommandQueue& audio, ParticlePool& particles) noexcept;

private:
    void updateInterceptors(float dtSec, WorldPos playerPos, EntityPool& pool, const TileMap& map) noexcept;

    void updateCrawlers(float dtSec, EntityPool& pool, const TileMap& map) noexcept;

    void updateTurrets(float dtSec, WorldPos playerPos, EntityPool& pool, const TileMap& map, AudioCommandQueue& audio,
        ParticlePool& particles) noexcept;

    void updateHostileBullets(float dtSec, Hitbox playerBox, bool shieldActive, EnergyPoints& playerEnergy,
        EntityPool& pool, const TileMap& map, AudioCommandQueue& audio, ParticlePool& particles) noexcept;
};

} // namespace ShooterCore
