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
/// @brief Manages Firehawk (Thexder 2) complete 5-weapon secondary tactical arsenal:
///        Standard Missiles, Homing Missiles, Napalm Bombs, Flash Bombs, and Deflector Shield.
/// @details Pre-allocates all ordnance storage with strictly zero dynamic heap allocations (CERT ERR55-CPP).
class SecondaryArsenal final {
public:
    static constexpr std::size_t kMaxMissiles { kMaxActiveMissiles };
    static constexpr std::size_t kMaxStandardMissiles { ShooterCore::kMaxStandardMissiles };
    static constexpr std::size_t kMaxNapalmCanisters { ShooterCore::kMaxNapalmCanisters };
    static constexpr std::size_t kMaxNapalmPools { ShooterCore::kMaxNapalmPools };

    // Standard Missiles
    static constexpr float kStandardMissileSpeed { 400.0f };
    static constexpr float kStandardMissileLife { 2.0f };
    static constexpr std::uint16_t kStandardMissileDamage { 50U };

    // Homing Missiles
    static constexpr float kInitialMissileSpeed { 160.0f };
    static constexpr float kMissileThrust { 320.0f };
    static constexpr float kMissileTopSpeed { 360.0f };
    static constexpr float kMissileTurnRateDeg { 180.0f };
    static constexpr float kMissileFuelTime { 2.5f };
    static constexpr std::uint16_t kMissileDirectDamage { 40U };
    static constexpr std::uint16_t kMissileSplashDamage { 20U };
    static constexpr float kMissileSplashRadius { 24.0f };

    // Napalm Bombs
    static constexpr float kNapalmDropSpeed { 50.0f };
    static constexpr float kNapalmGravity { 250.0f };
    static constexpr float kNapalmSpreadSpeed { 48.0f };
    static constexpr float kNapalmMaxSpread { 40.0f };
    static constexpr float kNapalmDuration { 3.0f };
    static constexpr std::uint16_t kNapalmDps { 60U };

    // Flash Bombs
    static constexpr float kFlashBombSpeed { 900.0f };
    static constexpr float kFlashBombDuration { 0.35f };
    static constexpr std::uint16_t kFlashBombDamage { 150U };

    // Deflector Shield
    static constexpr float kDeflectorDuration { 4.0f };
    static constexpr std::uint16_t kDeflectorMaxHp { 100U };
    static constexpr float kDeflectorRadius { 24.0f };

    // Magazine Capacities
    static constexpr std::uint8_t kDefaultStandardMissileCap { 30U };
    static constexpr std::uint8_t kDefaultHomingMissileCap { 20U };
    static constexpr std::uint8_t kDefaultNapalmCap { 15U };
    static constexpr std::uint8_t kDefaultBombCap { 5U };
    static constexpr std::uint8_t kDefaultDeflectorCap { 10U };

    static constexpr float kMissileFireCooldown { 0.35f };

    SecondaryArsenal() noexcept = default;

    /// @brief Cycle through secondary weapon selector:
    ///        None -> StandardMissile -> HomingMissile -> NapalmBomb -> FlashBomb -> DeflectorShield -> None.
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

    /// @brief Retrieve reference to standard missile magazine.
    /// @return Const reference to SecondaryAmmo.
    [[nodiscard]] constexpr const SecondaryAmmo& standardMissileAmmo() const noexcept
    {
        return stdMissileAmmo_;
    }

    /// @brief Retrieve reference to homing missile ammunition magazine.
    /// @return Const reference to SecondaryAmmo.
    [[nodiscard]] constexpr const SecondaryAmmo& homingMissileAmmo() const noexcept
    {
        return homingMissileAmmo_;
    }

    /// @brief Compatibility accessor for homing missile ammunition magazine.
    /// @return Const reference to SecondaryAmmo.
    [[nodiscard]] constexpr const SecondaryAmmo& missileAmmo() const noexcept
    {
        return homingMissileAmmo_;
    }

    /// @brief Retrieve reference to napalm bomb ammunition magazine.
    /// @return Const reference to SecondaryAmmo.
    [[nodiscard]] constexpr const SecondaryAmmo& napalmAmmo() const noexcept
    {
        return napalmAmmo_;
    }

    /// @brief Retrieve reference to flash bomb ammunition magazine.
    /// @return Const reference to SecondaryAmmo.
    [[nodiscard]] constexpr const SecondaryAmmo& bombAmmo() const noexcept
    {
        return bombAmmo_;
    }

    /// @brief Retrieve reference to deflector shield battery magazine.
    /// @return Const reference to SecondaryAmmo.
    [[nodiscard]] constexpr const SecondaryAmmo& deflectorAmmo() const noexcept
    {
        return deflectorAmmo_;
    }

    /// @brief Query ammunition magazine for the currently equipped weapon.
    /// @return Const reference to active weapon's SecondaryAmmo.
    [[nodiscard]] constexpr const SecondaryAmmo& currentAmmo() const noexcept
    {
        switch (activeWeapon_) {
        case SecondaryWeaponType::StandardMissile:
            return stdMissileAmmo_;
        case SecondaryWeaponType::HomingMissile:
            return homingMissileAmmo_;
        case SecondaryWeaponType::NapalmBomb:
            return napalmAmmo_;
        case SecondaryWeaponType::FlashBomb:
            return bombAmmo_;
        case SecondaryWeaponType::DeflectorShield:
            return deflectorAmmo_;
        case SecondaryWeaponType::None:
        default:
            return kZeroAmmo_;
        }
    }

    /// @brief Replenish standard unguided demolition missiles.
    /// @param[in] count Number of rounds to add.
    void replenishStandardMissiles(std::uint8_t count) noexcept;

    /// @brief Replenish homing micro-missile ammunition.
    /// @param[in] count Number of rounds to add.
    void replenishHomingMissiles(std::uint8_t count) noexcept;

    /// @brief Compatibility alias for homing micro-missile replenishment.
    /// @param[in] count Number of rounds to add.
    void replenishMissiles(std::uint8_t count) noexcept;

    /// @brief Replenish napalm surface carpet bombs.
    /// @param[in] count Number of bombs to add.
    void replenishNapalm(std::uint8_t count) noexcept;

    /// @brief Replenish screen-clearing flash bomb ammunition.
    /// @param[in] count Number of bombs to add.
    void replenishBombs(std::uint8_t count) noexcept;

    /// @brief Replenish deflector shield power cells.
    /// @param[in] count Number of cells to add.
    void replenishDeflector(std::uint8_t count) noexcept;

    /// @brief Attempt to fire active secondary weapon.
    /// @param[in] craftPos Craft bottom-center world coordinates.
    /// @param[in] facing Direction craft is heading.
    /// @param[in] entities Pool of candidate targets for homing lock.
    /// @param[in,out] audio Queue for acoustic firing cues.
    /// @param[in,out] particles Particle pool for muzzle/smoke effects.
    /// @return true if ordnance was launched; false otherwise.
    bool triggerFire(WorldPos craftPos, FacingDirection facing, EntityPool& entities, AudioCommandQueue& audio,
        ParticlePool& particles) noexcept;

    /// @brief Advance simulation of active missiles, napalm pools, and shockwaves.
    /// @param[in] dt Frame simulation delta time.
    /// @param[in,out] entities Entity pool for target queries and damage.
    /// @param[in,out] map Tilemap for obstacle collisions and destructible wall breaches.
    /// @param[in,out] particles Particle pool for smoke trails and explosion debris.
    /// @param[in,out] audio Sound queue for impact and detonation audio cues.
    void update(
        DeltaTime dt, EntityPool& entities, TileMap& map, ParticlePool& particles, AudioCommandQueue& audio) noexcept;

    /// @brief Query array of active standard unguided missiles.
    /// @return Const reference to standard missile pool.
    [[nodiscard]] const std::array<StandardMissile, kMaxStandardMissiles>& getStandardMissiles() const noexcept
    {
        return standardMissiles_;
    }

    /// @brief Query array of active homing missile projectiles.
    /// @return Const reference to homing missile pool.
    [[nodiscard]] const std::array<HomingMissile, kMaxMissiles>& getMissiles() const noexcept
    {
        return missiles_;
    }

    /// @brief Query array of falling napalm canisters.
    /// @return Const reference to napalm canister pool.
    [[nodiscard]] const std::array<NapalmCanister, kMaxNapalmCanisters>& getNapalmCanisters() const noexcept
    {
        return napalmCanisters_;
    }

    /// @brief Query array of active burning napalm pools.
    /// @return Const reference to napalm pool array.
    [[nodiscard]] const std::array<NapalmPool, kMaxNapalmPools>& getNapalmPools() const noexcept
    {
        return napalmPools_;
    }

    /// @brief Query current state of flash bomb shockwave.
    /// @return Const reference to FlashBombState.
    [[nodiscard]] constexpr const FlashBombState& getFlashBomb() const noexcept
    {
        return flashBomb_;
    }

    /// @brief Query current state of deflector shield.
    /// @return Const reference to DeflectorState.
    [[nodiscard]] constexpr const DeflectorState& getDeflector() const noexcept
    {
        return deflector_;
    }

    /// @brief Check whether deflector shield is currently active.
    /// @return true if active.
    [[nodiscard]] constexpr bool isDeflectorActive() const noexcept
    {
        return deflector_.active;
    }

    /// @brief Retrieve count of currently airborne standard missiles.
    /// @return Active standard missile count.
    [[nodiscard]] constexpr std::size_t activeStandardMissileCount() const noexcept
    {
        return activeStandardMissileCount_;
    }

    /// @brief Retrieve count of currently airborne homing missiles.
    /// @return Active homing missile count.
    [[nodiscard]] constexpr std::size_t activeMissileCount() const noexcept
    {
        return activeMissileCount_;
    }

    /// @brief Retrieve count of active burning napalm pools.
    /// @return Active napalm pool count.
    [[nodiscard]] constexpr std::size_t activeNapalmPoolCount() const noexcept
    {
        return activeNapalmPoolCount_;
    }

    /// @brief Clear all airborne ordnance and reset active states.
    void clear() noexcept;

private:
    [[nodiscard]] EntityId findMissileTarget(WorldPos missilePos, const EntityPool& entities) const noexcept;
    void explodeHomingMissile(std::size_t idx, EntityPool& entities, TileMap& map, ParticlePool& particles,
        AudioCommandQueue& audio) noexcept;
    void explodeStandardMissile(std::size_t idx, EntityPool& entities, TileMap& map, ParticlePool& particles,
        AudioCommandQueue& audio) noexcept;
    void igniteNapalm(WorldPos impactPos, TileMap& map, ParticlePool& particles, AudioCommandQueue& audio) noexcept;

    static constexpr SecondaryAmmo kZeroAmmo_ { 0U, 0U };

    SecondaryWeaponType activeWeapon_ { SecondaryWeaponType::None };
    SecondaryAmmo stdMissileAmmo_ { 15U, kDefaultStandardMissileCap };
    SecondaryAmmo homingMissileAmmo_ { 12U, kDefaultHomingMissileCap };
    SecondaryAmmo napalmAmmo_ { 6U, kDefaultNapalmCap };
    SecondaryAmmo bombAmmo_ { 2U, kDefaultBombCap };
    SecondaryAmmo deflectorAmmo_ { 3U, kDefaultDeflectorCap };

    std::array<StandardMissile, kMaxStandardMissiles> standardMissiles_ { };
    std::size_t activeStandardMissileCount_ { 0U };

    std::array<HomingMissile, kMaxMissiles> missiles_ { };
    std::size_t activeMissileCount_ { 0U };

    std::array<NapalmCanister, kMaxNapalmCanisters> napalmCanisters_ { };
    std::size_t activeNapalmCanisterCount_ { 0U };

    std::array<NapalmPool, kMaxNapalmPools> napalmPools_ { };
    std::size_t activeNapalmPoolCount_ { 0U };

    FlashBombState flashBomb_ { };
    DeflectorState deflector_ { };
    float fireCooldown_ { 0.0f };
};

} // namespace ShooterCore
