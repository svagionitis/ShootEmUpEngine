#include "StageProgressionSystem.h"

#include <cstddef>
#include <cstdint>

#include <string_view>

#include "Types.h"

namespace ShooterCore {

void StageProgressionSystem::startCampaign() noexcept
{
    currentStage_ = 1U;
    currentZone_ = 1U;
    isStageComplete_ = false;
    isCampaignComplete_ = false;
    hasCheckpoint_ = false;
    checkpointPos_ = WorldPos { 0.0f, 0.0f };
}

bool StageProgressionSystem::advanceZone() noexcept
{
    if (isCampaignComplete_ || isStageComplete_) {
        return false;
    }

    const std::uint8_t maxZones { totalZonesInCurrentStage() };
    if (currentZone_ < maxZones) {
        ++currentZone_;
        return true;
    }

    // Reached end of final zone in stage
    isStageComplete_ = true;
    return false;
}

bool StageProgressionSystem::advanceStage() noexcept
{
    if (isCampaignComplete_) {
        return false;
    }

    if (currentStage_ < kMaxStage) {
        ++currentStage_;
        currentZone_ = 1U;
        isStageComplete_ = false;
        hasCheckpoint_ = false;
        checkpointPos_ = WorldPos { 0.0f, 0.0f };
        return true;
    }

    isCampaignComplete_ = true;
    isStageComplete_ = true;
    return false;
}

std::string_view StageProgressionSystem::currentStageName() const noexcept
{
    switch (currentStage_) {
    case 1U:
        return "STAGE 1: EARTH ORBIT DEFENSE";
    case 2U:
        return "STAGE 2: SURFACE DESOLATION BASE";
    case 3U:
        return "STAGE 3: SUBTERRANEAN LAVA CAVERNS";
    case 4U:
        return "STAGE 4: SUNKEN BIOSPHERE RUINS";
    case 5U:
        return "STAGE 5: ALIEN BIO-LABYRINTH HIVE";
    case 6U:
        return "STAGE 6: ASTEROID TRENCH FORTRESS";
    case 7U:
        return "STAGE 7: PLASMA REACTOR CONDUITS";
    case 8U:
        return "STAGE 8: CENTRAL AUTOMATED BASTION";
    case 9U:
        return "STAGE 9: MOTHERSHIP NEDIAM CORE";
    default:
        return "UNKNOWN SECTOR";
    }
}

bool StageProgressionSystem::isAtPortal(WorldPos playerPos, WorldPos portalPos, float triggerRadius) noexcept
{
    const float dx { playerPos.x - portalPos.x };
    const float dy { playerPos.y - portalPos.y };
    return ((dx * dx) + (dy * dy)) <= (triggerRadius * triggerRadius);
}

} // namespace ShooterCore
