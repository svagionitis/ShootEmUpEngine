#pragma once

#include <cstddef>
#include <cstdint>

#include <array>

#include "AudioCommandQueue.h"
#include "EntityPool.h"
#include "Hitbox.h"
#include "ParticlePool.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore {

/// @class SecondaryArsenal
/// @brief Manages Fire Hawk secondary ordnance: weapon selector, homing missiles, and flash bombs.
/// @details Pre-allocates missile storage with strictly zero dynamic heap allocations (CERT ERR55-CPP).
class SecondaryArsenal final {
public:
    static constexpr std::size_t kMaxMissiles { kMaxActiveMissiles };
    static constexpr float kInitialMissileSpeed { 160.0f };
    static constexpr float kMissileThrust { 320.0f };
    static constexpr float kMissileTopSpeed { 360.0f };
    static constexpr float kMissileTurnRateDeg { 180.0f };
    static constexpr float kMissileFuelTime { 2.5f };
    static constexpr std::uint16_t kMissileDirectDamage { 40U };
    static constexpr std::uint16_t kMissileSplashDamage { 20U };
    static constexpr float kMissileSplashRadius { 24.0f };
    static constexpr float kFlashBombSpeed { 900.0f };
    static constexpr float kFlashBombDuration { 0.35f };
    static constexpr std::uint16_t kFlashBombDamage { 150U };
    static constexpr std::uint8_t kDefaultMissileCap { 24U };
    static constexpr std::uint8_t kDefaultBombCap { 3U };
    static constexpr float kMissileFireCooldown { 0.35f };

    SecondaryArsenal() noexcept = default;

    /// @brief Cycle through secondary weapon selector: None -> HomingMissile -> FlashBomb -> None.
    void cycleWeapon() noexcept;

    /// @brief Explicitly set the active secondary weapon mode.
    /// @param[in] type Target ordnance bay.
    void setWeapon(SecondaryWeaponType type) noexcept;

    /// @brief Query active secondary weapon selection.
    /// @return Current weapon type.
    [[nodiscard]] constexpr SecondaryWeaponType activeWeapon() const noexcept
    {
        return activeWeapon_;
    }

    /// @brief Retrieve reference to missile ammunition magazine.
    /// @return Const reference to SecondaryAmmo.
    [[nodiscard]] constexpr const SecondaryAmmo& missileAmmo() const noexcept
    {
        return missileAmmo_;
    }

    /// @brief Retrieve reference to flash bomb ammunition magazine.
    /// @return Const reference to SecondaryAmmo.
    [[nodiscard]] constexpr const SecondaryAmmo& bombAmmo() const noexcept
    {
        return bombAmmo_;
    }

    /// @brief Replenish homing micro-missile ammunition.
    /// @param[in] count Number of rounds to add.
    void replenishMissiles(std::uint8_t count) noexcept;

    /// @brief Replenish screen-clearing flash bomb ammunition.
    /// @param[in] count Number of bombs to add.
    void replenishBombs(std::uint8_t count) noexcept;

    /// @brief Attempt to fire active secondary weapon.
    /// @param[in] craftPos Craft bottom-center world coordinates.
    /// @param[in] facing Direction craft is heading.
    /// @param[in] entities Pool of candidate targets for homing lock.
    /// @param[in,out] audio Queue for acoustic firing cues.
    /// @param[in,out] particles Particle pool for muzzle/smoke effects.
    /// @return true if ordnance was launched; false otherwise.
    bool triggerFire(WorldPos craftPos, FacingDirection facing, EntityPool& entities, AudioCommandQueue& audio,
        ParticlePool& particles) noexcept;

    /// @brief Advance simulation of active missiles and shockwaves.
    /// @param[in] dt Frame simulation delta time.
    /// @param[in,out] entities Entity pool for target queries and damage.
    /// @param[in,out] map Tilemap for obstacle collisions and destructible wall breaches.
    /// @param[in,out] particles Particle pool for smoke trails and explosion debris.
    /// @param[in,out] audio Sound queue for impact and detonation audio cues.
    void update(
        DeltaTime dt, EntityPool& entities, TileMap& map, ParticlePool& particles, AudioCommandQueue& audio) noexcept;

    /// @brief Query array of active missile projectiles.
    /// @return Const reference to missile pool.
    [[nodiscard]] const std::array<HomingMissile, kMaxMissiles>& getMissiles() const noexcept
    {
        return missiles_;
    }

    /// @brief Query current state of flash bomb shockwave.
    /// @return Const reference to FlashBombState.
    [[nodiscard]] constexpr const FlashBombState& getFlashBomb() const noexcept
    {
        return flashBomb_;
    }

    /// @brief Retrieve count of currently airborne missiles.
    /// @return Active missile count.
    [[nodiscard]] constexpr std::size_t activeMissileCount() const noexcept
    {
        return activeMissileCount_;
    }

    /// @brief Clear all airborne missiles and reset active shockwaves.
    void clear() noexcept;

private:
    [[nodiscard]] EntityId findMissileTarget(WorldPos missilePos, const EntityPool& entities) const noexcept;
    void explodeMissile(std::size_t idx, EntityPool& entities, TileMap& map, ParticlePool& particles,
        AudioCommandQueue& audio) noexcept;

    SecondaryWeaponType activeWeapon_ { SecondaryWeaponType::None };
    SecondaryAmmo missileAmmo_ { 12U, kDefaultMissileCap };
    SecondaryAmmo bombAmmo_ { 1U, kDefaultBombCap };
    std::array<HomingMissile, kMaxMissiles> missiles_ { };
    std::size_t activeMissileCount_ { 0U };
    FlashBombState flashBomb_ { };
    float fireCooldown_ { 0.0f };
};

} // namespace ShooterCore
