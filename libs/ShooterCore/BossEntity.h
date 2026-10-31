#pragma once

#include <cstddef>
#include <cstdint>

#include <array>

#include "AudioCommandQueue.h"
#include "Hitbox.h"
#include "ParticlePool.h"
#include "Types.h"

namespace ShooterCore {

class EntityPool;

/// @enum class BossArchetype
/// @brief Classification of Firehawk campaign boss titans.
enum class BossArchetype : std::uint8_t {
    OrbitalCruiser = 1U, ///< Stage 1: Earth Orbit Heavy Cruiser
    MagmaCrustacean = 3U, ///< Stage 3: Subterranean Lava Titan
    HiveQueen = 5U, ///< Stage 5: Bio-Organic Labyrinth Heart
    NediamCore = 9U ///< Stage 9: Alien Mothership Central Core
};

/// @enum class BossPartType
/// @brief Sub-component classification for hierarchical hitboxes.
enum class BossPartType : std::uint8_t {
    WeakCore = 0U,
    ArmorPlating = 1U,
    Thruster = 2U,
    Turret = 3U,
    ShieldDrone = 4U
};

/// @struct BossPart
/// @brief Independent physical module of a modular boss entity.
struct BossPart final {
    BossPartType type { BossPartType::WeakCore };
    WorldPos offset { 0.0f, 0.0f };
    Hitbox bounds { { 0.0f, 0.0f }, { 32.0f, 32.0f } };
    std::uint16_t currentHp { 100U };
    std::uint16_t maxHp { 100U };
    bool isArmored { true };
    bool isDestroyed { false };
};

/// @class BossEntity
/// @brief Modular hierarchical boss titan with multi-part destructible hitboxes.
/// @details Pre-allocates all sub-modules with strictly zero dynamic heap allocations (CERT ERR55-CPP).
class BossEntity final {
public:
    static constexpr std::size_t kMaxParts { 8U };

    BossEntity() noexcept = default;

    /// @brief Initialize boss configuration for a given archetype at spawn coordinates.
    /// @param[in] archetype Boss configuration profile.
    /// @param[in] spawnPos Initial world coordinates.
    void init(BossArchetype archetype, WorldPos spawnPos) noexcept;

    /// @brief Master frame tick for movement, attack patterns, and phase progression.
    /// @param[in] dt Frame delta time.
    /// @param[in] playerPos Mecha world coordinate.
    /// @param[in,out] entities Entity pool for projectile spawning.
    /// @param[in,out] particles Particle pool for exhaust and explosion bursts.
    /// @param[in,out] audio Sound queue for combat acoustic cues.
    void update(DeltaTime dt, WorldPos playerPos, EntityPool& entities, ParticlePool& particles,
        AudioCommandQueue& audio) noexcept;

    /// @brief Apply weapon damage to intersecting boss parts.
    /// @param[in] hitArea Weapon damage area.
    /// @param[in] damage Damage value to inflict.
    /// @param[in,out] audio Audio queue for impact sounds.
    /// @return true if damage was absorbed by any part.
    bool applyDamage(const Hitbox& hitArea, std::uint16_t damage, AudioCommandQueue& audio) noexcept;

    /// @brief Query active combat phase.
    /// @return Current BossPhase.
    [[nodiscard]] constexpr BossPhase phase() const noexcept
    {
        return phase_;
    }

    /// @brief Check whether boss core has been destroyed and boss is defeated.
    /// @return true if defeated.
    [[nodiscard]] constexpr bool isDefeated() const noexcept
    {
        return phase_ == BossPhase::Defeated;
    }

    /// @brief Query boss archetype.
    /// @return Active BossArchetype.
    [[nodiscard]] constexpr BossArchetype archetype() const noexcept
    {
        return archetype_;
    }

    /// @brief Query root world position of boss anchor.
    /// @return Continuous WorldPos.
    [[nodiscard]] constexpr WorldPos rootPosition() const noexcept
    {
        return rootPos_;
    }

    /// @brief Query number of configured parts.
    /// @return Active part count.
    [[nodiscard]] constexpr std::size_t partCount() const noexcept
    {
        return partCount_;
    }

    /// @brief Retrieve reference to specific boss part by index.
    /// @param[in] idx Part index.
    /// @return Const reference to BossPart.
    [[nodiscard]] const BossPart& getPart(std::size_t idx) const noexcept;

    /// @brief Query overall core health percentage [0..100].
    /// @return Health percentage.
    [[nodiscard]] std::uint8_t coreHealthPercent() const noexcept;

private:
    void updateOrbitalCruiser(DeltaTime dt, WorldPos playerPos, EntityPool& entities, ParticlePool& particles,
        AudioCommandQueue& audio) noexcept;
    void updateDetonation(DeltaTime dt, ParticlePool& particles, AudioCommandQueue& audio) noexcept;

    BossArchetype archetype_ { BossArchetype::OrbitalCruiser };
    BossPhase phase_ { BossPhase::Dormant };
    WorldPos rootPos_ { 0.0f, 0.0f };
    Velocity vel_ { 0.0f, 0.0f };
    float hoverTimer_ { 0.0f };
    float attackTimer_ { 0.0f };
    float deathTimer_ { 0.0f };
    std::size_t partCount_ { 0U };
    std::array<BossPart, kMaxParts> parts_ { };
};

} // namespace ShooterCore
