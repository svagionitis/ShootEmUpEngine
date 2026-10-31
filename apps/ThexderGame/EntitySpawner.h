#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "EntityPool.h"
#include "Types.h"

namespace ThexderGame {

/// @struct SpawnerNode
/// @brief Stationary generator nest located in cavern walls.
struct SpawnerNode final {
    ShooterCore::WorldPos pos { 0.0f, 0.0f };
    ShooterCore::EntityType spawnType { ShooterCore::EntityType::PatrolWalker };
    float cooldownSec { 3.5f };
    float timerSec { 0.0f };
    std::uint16_t health { 30U };
    bool destroyed { false };
};

/// @class EntitySpawner
/// @brief Manages stationary generator nests and dynamic drone wave dispatching.
/// @details Strictly zero-heap operation guaranteeing safety bounds on EntityPool (CERT ERR55-CPP).
class EntitySpawner final {
public:
    static constexpr std::size_t kMaxNests { 16U };

    EntitySpawner() noexcept;

    /// @brief Initialize nests at authentic Level 1 cavern locations.
    void initLevel1Nests() noexcept;

    /// @brief Update spawn timers and instantiate drones when player is near.
    /// @param[in] dt Simulation delta time.
    /// @param[in] playerPos World coordinates of player craft.
    /// @param[in,out] pool Object pool receiving spawned entities.
    void updateSpawners(
        ShooterCore::DeltaTime dt, ShooterCore::WorldPos playerPos, ShooterCore::EntityPool& pool) noexcept;

    /// @brief Apply damage to generator nest if struck by weapons.
    /// @param[in] nestIndex Index of hit nest.
    /// @param[in] dmg Damage magnitude.
    /// @return true if nest was destroyed by this hit; false otherwise.
    bool damageNest(std::size_t nestIndex, ShooterCore::DamageValue dmg) noexcept;

    /// @brief Retrieve total count of generator nests.
    [[nodiscard]] constexpr std::size_t nestCount() const noexcept
    {
        return nestCount_;
    }

    /// @brief Retrieve reference to specific nest node.
    [[nodiscard]] const SpawnerNode& getNest(std::size_t idx) const noexcept;

private:
    std::array<SpawnerNode, kMaxNests> nests_ { };
    std::size_t nestCount_ { 0U };
    float aerialWaveTimer_ { 0.0f };
};

} // namespace ThexderGame
