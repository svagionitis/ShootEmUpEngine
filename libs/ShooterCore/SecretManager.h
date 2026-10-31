#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "AudioCommandQueue.h"
#include "Hitbox.h"
#include "RenderCommandQueue.h"
#include "Types.h"

namespace ShooterCore {

class TileMap;

/// @class SecretManager
/// @brief Manages concealed masonry passages, secret chambers, and permanent Energy Tank upgrades.
class SecretManager final {
public:
    static constexpr std::size_t kMaxEnergyTanks { 8U };
    static constexpr std::size_t kMaxSecretWalls { 16U };

    SecretManager() noexcept = default;

    /// @brief Register a discrete secret wall tile coordinate.
    bool registerSecretWall(TileCoord coord) noexcept;

    /// @brief Register a permanent Energy Tank canister in a secret alcove.
    bool registerEnergyTank(WorldPos pos) noexcept;

    /// @brief Apply damage to a secret wall; emits crumbling audio and particles upon breach.
    bool damageSecretWall(TileCoord coord, TileMap& map, AudioCommandQueue& audio) noexcept;

    /// @brief Test overlap between player hitbox and uncollected energy tanks.
    /// @return true if an Energy Tank was collected.
    bool checkPickup(const Hitbox& playerBox, EnergyPoints& playerEnergy, std::uint16_t& maxEnergy,
        AudioCommandQueue& audio) noexcept;

    /// @brief Emit render commands for uncollected Energy Tanks.
    void emitRender(RenderCommandQueue& renderQueue) const noexcept;

    /// @brief Access array of registered energy tanks.
    [[nodiscard]] const std::array<EnergyTank, kMaxEnergyTanks>& tanks() const noexcept
    {
        return tanks_;
    }

    /// @brief Query total registered tank count.
    [[nodiscard]] std::size_t tankCount() const noexcept
    {
        return tankCount_;
    }

private:
    std::array<EnergyTank, kMaxEnergyTanks> tanks_ { };
    std::size_t tankCount_ { 0U };

    std::array<TileCoord, kMaxSecretWalls> secretWalls_ { };
    std::size_t secretWallCount_ { 0U };
};

} // namespace ShooterCore
