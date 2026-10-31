#pragma once

#include <cstddef>
#include <cstdint>

#include <array>

#include "Types.h"

namespace ShooterCore {

/// @class ParticlePool
/// @brief Zero-heap bounded particle container for combat visual effects.
/// @details Pre-allocates 128 particle slots adhering to CERT ERR55-CPP.
class ParticlePool final {
public:
    static constexpr std::size_t kCapacity { kMaxParticles };

    constexpr ParticlePool() noexcept = default;

    /// @brief Advance particle simulation, applying velocity, drag, and alpha decay.
    /// @param[in] dt Frame simulation delta time.
    void update(DeltaTime dt) noexcept;

    /// @brief Emit a directional burst of high-velocity laser impact sparks.
    /// @param[in] origin Emitter source position.
    /// @param[in] normalAngle Angle of reflection normal in radians.
    /// @param[in] count Number of spark particles to emit.
    void emitSparks(WorldPos origin, float normalAngle, std::size_t count) noexcept;

    /// @brief Emit tumbling masonry rubble fragments for destructible walls.
    /// @param[in] origin Emitter source position.
    /// @param[in] count Number of debris particles to emit.
    void emitDebris(WorldPos origin, std::size_t count) noexcept;

    /// @brief Emit radial energy barrier deflection sparks.
    /// @param[in] origin Emitter source position.
    /// @param[in] hitAngle Deflection angle in radians.
    /// @param[in] count Number of shield sparks to emit.
    void emitShieldSparks(WorldPos origin, float hitAngle, std::size_t count) noexcept;

    /// @brief Emit a puff of rocket propellant smoke behind an accelerating missile.
    /// @param[in] origin Emitter source position.
    void emitSmoke(WorldPos origin) noexcept;

    /// @brief Emit expanding shockwave debris/plasma particles for flash bomb detonations.
    /// @param[in] origin Center of detonation shockwave.
    /// @param[in] count Number of shockwave particles to emit.
    void emitShockwave(WorldPos origin, std::size_t count) noexcept;

    /// @brief Retrieve bounded pool array.
    /// @return Const reference to particle storage.
    [[nodiscard]] const std::array<Particle, kCapacity>& particles() const noexcept
    {
        return pool_;
    }

    /// @brief Retrieve count of currently active particles.
    /// @return Active particle count.
    [[nodiscard]] constexpr std::size_t activeCount() const noexcept
    {
        return activeCount_;
    }

    /// @brief Clear and deactivate all particles in the pool.
    void clear() noexcept;

private:
    [[nodiscard]] std::size_t allocateSlot() noexcept;
    [[nodiscard]] float randomFloat(float minVal, float maxVal) noexcept;

    std::array<Particle, kCapacity> pool_ { };
    std::size_t activeCount_ { 0U };
    std::size_t nextSlot_ { 0U };
    std::uint32_t rngState_ { 0x12345678U };
};

} // namespace ShooterCore
