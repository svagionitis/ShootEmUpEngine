#pragma once

#include <cstddef>
#include <cstdint>

#include <array>

#include "AudioCommandQueue.h"
#include "EntityPool.h"
#include "ParticlePool.h"
#include "SupplyDropSystem.h"
#include "Types.h"

namespace ShooterCore {

/// @class NestManager
/// @brief Manages stationary bio-organic hive generators and proximity-escalated drone waves.
/// @details Pre-allocates 16 hive slots maintaining zero dynamic heap allocations (CERT ERR55-CPP).
class NestManager final {
public:
    static constexpr std::size_t kMaxNestSlots { kMaxNests };
    static constexpr float kProximityRadius { 160.0f };
    static constexpr float kDefaultInterval { 4.0f };
    static constexpr float kEscalatedInterval { 1.2f };
    static constexpr std::uint16_t kDefaultNestHealth { 80U };

    NestManager() noexcept = default;

    /// @brief Register a bio-nest generator hive at specified world coordinates.
    /// @param[in] pos Continuous world position of hive center.
    /// @param[in] health Initial structural durability hit points.
    /// @param[in] interval Baseline spawn interval in seconds.
    /// @return true if successfully allocated; false if maximum nests reached.
    bool addNest(WorldPos pos, std::uint16_t health = kDefaultNestHealth, float interval = kDefaultInterval) noexcept;

    /// @brief Advance simulation of hive spawn timers and trigger drone wave instantiation.
    /// @param[in] dt Frame simulation delta time.
    /// @param[in] playerPos World position of player craft for proximity detection.
    /// @param[in,out] entities Entity pool receiving spawned drones.
    /// @param[in,out] supplyDrops Supply drop system receiving loot drops upon hive destruction.
    /// @param[in,out] particles Particle pool for explosion and spawn FX.
    /// @param[in,out] audio Sound queue for hive audio cues.
    void update(DeltaTime dt, WorldPos playerPos, EntityPool& entities, SupplyDropSystem& supplyDrops,
        ParticlePool& particles, AudioCommandQueue& audio) noexcept;

    /// @brief Inflict damage on a specific generator hive.
    /// @param[in] idx Hive slot index.
    /// @param[in] dmg Damage magnitude.
    /// @param[in,out] supplyDrops Supply drop system for loot drops.
    /// @param[in,out] particles Particle pool for impact sparks and debris.
    /// @param[in,out] audio Sound queue for destruction SFX.
    /// @return true if hive was destroyed by this hit; false otherwise.
    bool damageNest(std::size_t idx, DamageValue dmg, SupplyDropSystem& supplyDrops, ParticlePool& particles,
        AudioCommandQueue& audio) noexcept;

    /// @brief Retrieve bounded array of bio-nests.
    /// @return Const reference to nest storage.
    [[nodiscard]] const std::array<BioNest, kMaxNestSlots>& getNests() const noexcept
    {
        return nests_;
    }

    /// @brief Retrieve count of registered bio-nests.
    /// @return Total nest count.
    [[nodiscard]] constexpr std::size_t nestCount() const noexcept
    {
        return nestCount_;
    }

    /// @brief Clear all registered nests from the manager.
    void clear() noexcept;

private:
    std::array<BioNest, kMaxNestSlots> nests_ { };
    std::size_t nestCount_ { 0U };
    std::uint8_t dropCounter_ { 0U };
};

} // namespace ShooterCore
