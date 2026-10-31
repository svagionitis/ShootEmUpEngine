#pragma once

#include <cstddef>
#include <cstdint>

#include <string_view>

#include "Types.h"

namespace ShooterCore {

/// @class StageProgressionSystem
/// @brief Manages 9-stage multi-zone campaign flow, checkpoints, and victory states for Firehawk.
/// @details Operates with strictly zero dynamic heap allocations (CERT ERR55-CPP).
class StageProgressionSystem final {
public:
    static constexpr std::uint8_t kMinStage { 1U };
    static constexpr std::uint8_t kMaxStage { 9U };
    static constexpr std::uint8_t kDefaultZonesPerStage { 4U };
    static constexpr std::uint8_t kFinalStageZones { 2U };

    constexpr StageProgressionSystem() noexcept = default;

    /// @brief Initialize or restart the campaign at Stage 1, Zone 1.
    void startCampaign() noexcept;

    /// @brief Query active campaign stage number (1..9).
    /// @return Current stage.
    [[nodiscard]] constexpr std::uint8_t currentStage() const noexcept
    {
        return currentStage_;
    }

    /// @brief Query active zone within the current stage (1..4).
    /// @return Current zone.
    [[nodiscard]] constexpr std::uint8_t currentZone() const noexcept
    {
        return currentZone_;
    }

    /// @brief Query total number of zones in the current stage.
    /// @return Zone count (4 for stages 1..8, 2 for stage 9).
    [[nodiscard]] constexpr std::uint8_t totalZonesInCurrentStage() const noexcept
    {
        return currentStage_ == kMaxStage ? kFinalStageZones : kDefaultZonesPerStage;
    }

    /// @brief Advance to the next zone in the current stage or trigger stage victory.
    /// @return true if transitioned to a new zone; false if the stage was completed.
    bool advanceZone() noexcept;

    /// @brief Advance to the next stage after completing the current boss encounter.
    /// @return true if transitioned to next stage; false if campaign was won.
    bool advanceStage() noexcept;

    /// @brief Check whether the current stage boss was defeated and stage is cleared.
    /// @return true if stage is completed.
    [[nodiscard]] constexpr bool isStageComplete() const noexcept
    {
        return isStageComplete_;
    }

    /// @brief Check whether the entire 9-stage campaign was successfully finished.
    /// @return true if campaign completed.
    [[nodiscard]] constexpr bool isCampaignComplete() const noexcept
    {
        return isCampaignComplete_;
    }

    /// @brief Retrieve human-readable tactical codename for the active stage.
    /// @return Static string view of stage name.
    [[nodiscard]] std::string_view currentStageName() const noexcept;

    /// @brief Register a respawn checkpoint position.
    /// @param[in] pos Continuous world position of checkpoint portal.
    constexpr void setCheckpoint(WorldPos pos) noexcept
    {
        checkpointPos_ = pos;
        hasCheckpoint_ = true;
    }

    /// @brief Query active checkpoint position.
    /// @return WorldPos coordinates of last saved checkpoint.
    [[nodiscard]] constexpr WorldPos checkpoint() const noexcept
    {
        return checkpointPos_;
    }

    /// @brief Check whether a valid checkpoint is registered.
    /// @return true if checkpoint exists.
    [[nodiscard]] constexpr bool hasCheckpoint() const noexcept
    {
        return hasCheckpoint_;
    }

    /// @brief Clear active checkpoint.
    constexpr void clearCheckpoint() noexcept
    {
        hasCheckpoint_ = false;
        checkpointPos_ = WorldPos { 0.0f, 0.0f };
    }

    /// @brief Test proximity to an airlock or stage exit warp portal.
    /// @param[in] playerPos Mecha world coordinate.
    /// @param[in] portalPos Portal trigger coordinate.
    /// @param[in] triggerRadius Trigger proximity radius in pixels.
    /// @return true if player is within trigger range of portal.
    [[nodiscard]] static bool isAtPortal(WorldPos playerPos, WorldPos portalPos, float triggerRadius = 24.0f) noexcept;

private:
    WorldPos checkpointPos_ { 0.0f, 0.0f };
    std::uint8_t currentStage_ { 1U };
    std::uint8_t currentZone_ { 1U };
    bool isStageComplete_ { false };
    bool isCampaignComplete_ { false };
    bool hasCheckpoint_ { false };
};

} // namespace ShooterCore
