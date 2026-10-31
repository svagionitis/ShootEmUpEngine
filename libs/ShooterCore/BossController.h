#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "AudioCommandQueue.h"
#include "Hitbox.h"
#include "RenderCommandQueue.h"
#include "Types.h"

namespace ShooterCore {

class EntityPool;
class ParticlePool;

/// @struct BossSegment
/// @brief Physical sub-component of the multi-segment guardian.
struct BossSegment final {
    BossSegmentType type { BossSegmentType::CentralCore };
    WorldPos relativeOffset { 0.0f, 0.0f };
    Hitbox bounds { { 0.0f, 0.0f }, { 32.0f, 32.0f } };
    BossHealth health { 100U, 100U };
    bool isArmored { true };
};

/// @class BossController
/// @brief Master simulation controller for the Dreadnought Guardian multi-segment boss.
class BossController final {
public:
    static constexpr std::size_t kSegmentCount { 5U };

    BossController() noexcept;

    /// @brief Initialize boss spawn position in Sector Delta.
    void spawn(WorldPos spawnPos) noexcept;

    /// @brief Master tick: updates kinematics, attacks, and phase transitions.
    void update(DeltaTime dt, WorldPos playerPos, EntityPool& entities, ParticlePool& particles,
        AudioCommandQueue& audioQueue) noexcept;

    /// @brief Apply damage to boss segments overlapping a weapon hitbox.
    /// @return true if damage was accepted by a segment.
    bool applyDamage(const Hitbox& hitArea, std::uint16_t damage, AudioCommandQueue& audio) noexcept;

    /// @brief Emit render commands for all active boss segments.
    void emitRender(RenderCommandQueue& renderQueue) const noexcept;

    /// @brief Query active combat phase.
    [[nodiscard]] constexpr BossPhase phase() const noexcept
    {
        return phase_;
    }

    /// @brief Query whether boss is defeated and chamber is clear.
    [[nodiscard]] constexpr bool isDefeated() const noexcept
    {
        return phase_ == BossPhase::Defeated;
    }

    /// @brief Query world center of the guardian core.
    [[nodiscard]] constexpr WorldPos corePosition() const noexcept
    {
        return rootPos_;
    }

    /// @brief Access individual boss segment by index.
    [[nodiscard]] const BossSegment& getSegment(std::size_t idx) const noexcept
    {
        return segments_[idx];
    }

    /// @brief Query total segment count.
    [[nodiscard]] static constexpr std::size_t segmentCount() noexcept
    {
        return kSegmentCount;
    }

private:
    void initSegments() noexcept;
    void updatePhase1(DeltaTime dt, AudioCommandQueue& audio) noexcept;
    void updatePhase2(DeltaTime dt, AudioCommandQueue& audio) noexcept;
    void updatePhase3(DeltaTime dt, WorldPos playerPos, AudioCommandQueue& audio) noexcept;

    WorldPos rootPos_ { 0.0f, 0.0f };
    float initialY_ { 0.0f };
    Velocity velocity_ { 0.0f, 0.0f };
    float hoverTimer_ { 0.0f };
    float attackTimer_ { 0.0f };

    BossPhase phase_ { BossPhase::Dormant };
    std::array<BossSegment, kSegmentCount> segments_ { };
};

} // namespace ShooterCore
