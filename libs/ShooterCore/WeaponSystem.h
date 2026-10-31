#pragma once

#include <cstddef>
#include <cstdint>

#include <array>

#include "CollisionSolver.h"
#include "Hitbox.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore {

/// @struct TargetCandidate
/// @brief Bounded spatial representation of a targetable entity for auto-aiming.
struct TargetCandidate final {
    std::uint32_t id { 0U };
    Hitbox bounds { };
    TargetType type { TargetType::Hostile };
    bool active { false };

    /// @brief Calculate center coordinate of candidate bounding box.
    /// @return Center WorldPos in world units.
    [[nodiscard]] constexpr WorldPos center() const noexcept
    {
        return WorldPos { bounds.min().x + (bounds.width() * 0.5f), bounds.min().y + (bounds.height() * 0.5f) };
    }
};

/// @class WeaponSystem
/// @brief Orchestrates auto-aim homing laser, shield barrier, and single-pool energy consumption.
/// @details Executes with 0 dynamic heap allocations per CERT ERR55-CPP.
class WeaponSystem final {
public:
    static constexpr std::size_t kMaxTargets { 32U };
    static constexpr float kLaserRangeRobot { 240.0f };
    static constexpr float kLaserRangeJet { 288.0f };
    static constexpr float kRobotConeCos { -1.0f }; // 360-degree omni-directional
    static constexpr float kJetConeCos { 0.50f }; // cos(60 deg) forward cone
    static constexpr float kLaserDrainRate { 10.0f }; // points / sec
    static constexpr float kShieldDrainRate { 15.0f }; // points / sec
    static constexpr float kTileDamageRate { 5.0f }; // hits / sec
    static constexpr float kShieldMargin { 4.0f }; // pixels beyond craft
    static constexpr float kTargetStickinessBonus { 0.25f };

    constexpr WeaponSystem() noexcept = default;

    /// @brief Query whether laser is currently actively firing.
    /// @return true if firing and energy available.
    [[nodiscard]] constexpr bool isLaserActive() const noexcept
    {
        return laserActive_;
    }

    /// @brief Query whether shield barrier is currently deployed.
    /// @return true if shield deployed and energy available.
    [[nodiscard]] constexpr bool isShieldActive() const noexcept
    {
        return shieldActive_;
    }

    /// @brief Retrieve active laser beam spatial coordinates for presentation.
    /// @return Active BeamSegment struct.
    [[nodiscard]] constexpr BeamSegment getBeamSegment() const noexcept
    {
        return beam_;
    }

    /// @brief Retrieve active multi-segment lightning arc for presentation.
    /// @return Const reference to active LightningArc struct.
    [[nodiscard]] constexpr const LightningArc& getLightningArc() const noexcept
    {
        return lightningArc_;
    }

    /// @brief Retrieve dynamic forcefield barrier presentation parameters.
    /// @return Current ShieldFxState struct.
    [[nodiscard]] constexpr const ShieldFxState& getShieldFx() const noexcept
    {
        return shieldFx_;
    }

    /// @brief Retrieve defensive perimeter bounding box for the shield barrier.
    /// @param[in] craftBox Current craft bounding box.
    /// @return Expanded AABB surrounding the craft.
    [[nodiscard]] static constexpr Hitbox computeShield(const Hitbox& craftBox) noexcept
    {
        return Hitbox { WorldPos { craftBox.min().x - kShieldMargin, craftBox.min().y - kShieldMargin },
            WorldPos { craftBox.max().x + kShieldMargin, craftBox.max().y + kShieldMargin } };
    }

    /// @brief Primary simulation tick updating weapons, targeting, shield, and energy pool.
    /// @param[in] dt Simulation delta time.
    /// @param[in] fireIntent Player weapon trigger intent.
    /// @param[in] shieldIntent Player shield deployment intent.
    /// @param[in] isTransforming Whether transformation lock is active.
    /// @param[in] mode Active transformation mode (Robot vs Jet).
    /// @param[in] facing Current facing direction.
    /// @param[in] craftBox Current craft bounding box.
    /// @param[in,out] map Discrete tilemap environment for LOS and destructibles.
    /// @param[in] targets Bounded array of active target candidates.
    /// @param[in] targetCount Number of valid candidates in targets array.
    /// @param[in,out] energy Single-pool energy points reservoir.
    void updateWeapons(DeltaTime dt, bool fireIntent, bool shieldIntent, bool isTransforming, TransformMode mode,
        FacingDirection facing, const Hitbox& craftBox, TileMap& map,
        const std::array<TargetCandidate, kMaxTargets>& targets, std::size_t targetCount,
        EnergyPoints& energy) noexcept;

    /// @brief Process an impact against the craft or active shield barrier.
    /// @param[in] damage Incoming damage value.
    /// @param[in,out] energy Single-pool energy points reservoir.
    /// @return true if impact was absorbed by shield; false if it penetrated to hull.
    bool absorbImpact(DamageValue damage, EnergyPoints& energy) noexcept;

    /// @brief Reset targeting, beam, and accumulator states.
    void reset() noexcept;

private:
    [[nodiscard]] static WorldPos computeMuzzle(const Hitbox& craftBox, FacingDirection facing) noexcept;

    [[nodiscard]] static std::int32_t findBestTarget(WorldPos muzzle, FacingDirection facing, TransformMode mode,
        const TileMap& map, const std::array<TargetCandidate, kMaxTargets>& targets, std::size_t targetCount,
        bool hasLockedTarget, std::uint32_t lastLockedTargetId) noexcept;

    void generateLightning(WorldPos muzzle, WorldPos target, bool locked, std::uint32_t targetId) noexcept;

    bool laserActive_ { false };
    bool shieldActive_ { false };
    BeamSegment beam_ { };
    LightningArc lightningArc_ { };
    ShieldFxState shieldFx_ { };
    DrainAccumulator laserDrain_ { };
    DrainAccumulator shieldDrain_ { };
    DrainAccumulator tileDamagePacer_ { };
    std::uint32_t lastLockedTargetId_ { 0U };
    bool hasLockedTarget_ { false };
    std::uint32_t simTicks_ { 0U };
    float timeAccumulator_ { 0.0f };
};

} // namespace ShooterCore
