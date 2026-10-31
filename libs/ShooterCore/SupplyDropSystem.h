#pragma once

#include <cstddef>
#include <cstdint>

#include <array>

#include "AudioCommandQueue.h"
#include "Hitbox.h"
#include "ParticlePool.h"
#include "SecondaryArsenal.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore {

/// @class SupplyDropSystem
/// @brief Manages dropped cargo canisters, magnetic collection kinematics, and supply replenishment.
/// @details Pre-allocates 16 canister slots guaranteeing zero dynamic allocation (CERT ERR55-CPP).
class SupplyDropSystem final {
public:
    static constexpr std::size_t kMaxCanisterSlots { kMaxCanisters };
    static constexpr float kGravity { 120.0f };
    static constexpr float kTerminalVelocity { 60.0f };
    static constexpr float kMagnetRadius { 32.0f };
    static constexpr float kMagnetSpeed { 240.0f };
    static constexpr float kCanisterSize { 12.0f };

    SupplyDropSystem() noexcept = default;

    /// @brief Instantiate a cargo canister supply drop.
    /// @param[in] pos World position of drop origin.
    /// @param[in] type Canister payload category.
    /// @param[in] initialVel Initial ejection velocity vector.
    /// @return true if canister successfully allocated; false if pool saturated.
    bool spawnCanister(WorldPos pos, CanisterType type, Velocity initialVel = Velocity { 0.0f, -40.0f }) noexcept;

    /// @brief Advance simulation of active supply drops, kinematics, and collection.
    /// @param[in] dt Frame simulation delta time.
    /// @param[in] playerBox Hitbox of player craft for collection overlap.
    /// @param[in] map Tilemap for ground collision bouncing.
    /// @param[in,out] arsenal Secondary arsenal receiving missile/bomb ammo.
    /// @param[in,out] playerEnergy Energy pool receiving energy restock.
    /// @param[in,out] playerShield Shield gauge receiving force barrier recharge.
    /// @param[in,out] audio Sound queue for pickup chimes.
    /// @param[in,out] particles Particle pool for golden pickup sparkle FX.
    void update(DeltaTime dt, Hitbox playerBox, const TileMap& map, SecondaryArsenal& arsenal,
        EnergyPoints& playerEnergy, ShieldGauge& playerShield, AudioCommandQueue& audio,
        ParticlePool& particles) noexcept;

    /// @brief Retrieve bounded array of active canisters.
    /// @return Const reference to canister storage.
    [[nodiscard]] const std::array<CargoCanister, kMaxCanisterSlots>& getCanisters() const noexcept
    {
        return canisters_;
    }

    /// @brief Retrieve count of currently active canisters.
    /// @return Active canister count.
    [[nodiscard]] constexpr std::size_t activeCount() const noexcept
    {
        return activeCount_;
    }

    /// @brief Clear all active canisters from the system.
    void clear() noexcept;

private:
    std::array<CargoCanister, kMaxCanisterSlots> canisters_ { };
    std::size_t activeCount_ { 0U };
};

} // namespace ShooterCore
