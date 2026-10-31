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

/// @struct SecurityGenerator
/// @brief Stationary high-voltage power node supplying energy to barrier gates.
struct SecurityGenerator final {
    WorldPos pos { 0.0f, 0.0f };
    Hitbox bounds { { 0.0f, 0.0f }, { 32.0f, 32.0f } };
    GeneratorHealth health { 100U };
    std::uint8_t linkedBarrierId { 0U };
    bool active { true };
};

/// @struct SecurityBarrier
/// @brief Impassable forcefield blocking access to locked sectors.
struct SecurityBarrier final {
    WorldPos pos { 0.0f, 0.0f };
    Hitbox bounds { { 0.0f, 0.0f }, { 16.0f, 48.0f } };
    SecurityGateState state { SecurityGateState::Locked };
    std::uint8_t barrierId { 0U };
    std::uint8_t requiredGenerators { 3U };
    std::uint8_t destroyedGenerators { 0U };
};

/// @class SecuritySystem
/// @brief Manages generator destruction, barrier unlocking, and collision synchronization.
class SecuritySystem final {
public:
    static constexpr std::size_t kMaxGenerators { 8U };
    static constexpr std::size_t kMaxBarriers { 4U };

    SecuritySystem() noexcept = default;

    /// @brief Register a new generator node in the world.
    bool registerGenerator(WorldPos pos, std::uint8_t barrierId) noexcept;

    /// @brief Register a security barrier gate in the world.
    bool registerBarrier(WorldPos pos, Hitbox bounds, std::uint8_t barrierId, std::uint8_t reqGen) noexcept;

    /// @brief Apply weapon damage to generators overlapping an area-of-effect or projectile.
    /// @param[in] hitArea Bounding box of the attack.
    /// @param[in] damage Amount of damage to inflict.
    /// @param[out] audioQueue Audio queue for destruction sounds.
    /// @return true if a generator was damaged.
    bool damageAt(const Hitbox& hitArea, std::uint16_t damage, AudioCommandQueue& audioQueue) noexcept;

    /// @brief Synchronize barrier open states with the physical TileMap grid.
    void syncTiles(TileMap& map) noexcept;

    /// @brief Emit render commands for forcefield shimmer and generator cores.
    void emitRender(RenderCommandQueue& renderQueue) const noexcept;

    /// @brief Query whether a specific barrier is disengaged.
    [[nodiscard]] bool isBarrierOpen(std::uint8_t barrierId) const noexcept;

    /// @brief Get active generator count.
    [[nodiscard]] std::size_t activeCount() const noexcept;

    /// @brief Get generator reference for minimap telemetry.
    [[nodiscard]] const std::array<SecurityGenerator, kMaxGenerators>& generators() const noexcept
    {
        return generators_;
    }

    /// @brief Get barrier reference for telemetry.
    [[nodiscard]] const std::array<SecurityBarrier, kMaxBarriers>& barriers() const noexcept
    {
        return barriers_;
    }

    /// @brief Get total barrier count.
    [[nodiscard]] std::size_t barrierCount() const noexcept
    {
        return barrierCount_;
    }

private:
    std::array<SecurityGenerator, kMaxGenerators> generators_ { };
    std::size_t generatorCount_ { 0U };

    std::array<SecurityBarrier, kMaxBarriers> barriers_ { };
    std::size_t barrierCount_ { 0U };
};

} // namespace ShooterCore
