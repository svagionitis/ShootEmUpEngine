#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include <string_view>

#include "StageProgressionSystem.h"
#include "Types.h"

namespace ShooterCore::Test {

static void testCampaignProgressionFlow() noexcept
{
    StageProgressionSystem prog { };
    prog.startCampaign();

    TEST_ASSERT(prog.currentStage() == 1U, "Initial stage must be 1");
    TEST_ASSERT(prog.currentZone() == 1U, "Initial zone must be 1");
    TEST_ASSERT(!prog.isStageComplete(), "Stage must not be complete initially");
    TEST_ASSERT(!prog.isCampaignComplete(), "Campaign must not be complete initially");
    TEST_ASSERT(prog.totalZonesInCurrentStage() == 4U, "Stage 1 must have 4 zones");

    // Advance through Stage 1 zones: 1 -> 2 -> 3 -> 4
    TEST_ASSERT(prog.advanceZone(), "Zone 1 -> 2 advance must succeed");
    TEST_ASSERT(prog.currentZone() == 2U, "Zone must be 2");

    TEST_ASSERT(prog.advanceZone(), "Zone 2 -> 3 advance must succeed");
    TEST_ASSERT(prog.currentZone() == 3U, "Zone must be 3");

    TEST_ASSERT(prog.advanceZone(), "Zone 3 -> 4 advance must succeed");
    TEST_ASSERT(prog.currentZone() == 4U, "Zone must be 4");

    // Advance past zone 4: triggers stage completion
    TEST_ASSERT(!prog.advanceZone(), "Zone 4 advance must signal stage clear");
    TEST_ASSERT(prog.isStageComplete(), "Stage 1 must be marked complete");
    TEST_ASSERT(!prog.isCampaignComplete(), "Campaign must not be complete after stage 1");

    // Advance stage to Stage 2
    TEST_ASSERT(prog.advanceStage(), "Advance stage 1 -> 2 must succeed");
    TEST_ASSERT(prog.currentStage() == 2U, "Stage must be 2");
    TEST_ASSERT(prog.currentZone() == 1U, "Zone must reset to 1");
    TEST_ASSERT(!prog.isStageComplete(), "Stage complete must reset on stage advance");

    // Advance through stages 2..8
    for (std::uint8_t s { 2U }; s <= 8U; ++s) {
        TEST_ASSERT(prog.currentStage() == s, "Current stage must match index");
        // Clear 4 zones
        for (std::uint8_t z { 1U }; z < 4U; ++z) {
            TEST_ASSERT(prog.advanceZone(), "Zone advancement must succeed");
        }
        (void)prog.advanceZone(); // Mark stage complete
        TEST_ASSERT(prog.isStageComplete(), "Stage must be complete");
        if (s < 8U) {
            TEST_ASSERT(prog.advanceStage(), "Stage advance must succeed");
        }
    }

    // Advance to final Stage 9 (Nediam Mothership Core)
    TEST_ASSERT(prog.advanceStage(), "Advance to Stage 9 must succeed");
    TEST_ASSERT(prog.currentStage() == 9U, "Must be Stage 9");
    TEST_ASSERT(prog.totalZonesInCurrentStage() == 2U, "Stage 9 must have exactly 2 zones");

    // Advance Zone 1 -> 2
    TEST_ASSERT(prog.advanceZone(), "Stage 9 Zone 1 -> 2 advance must succeed");
    TEST_ASSERT(prog.currentZone() == 2U, "Must be at final boss zone");

    // Advance past Zone 2 -> Stage 9 complete
    (void)prog.advanceZone();
    TEST_ASSERT(prog.isStageComplete(), "Stage 9 must be complete");

    // Advance stage at Stage 9 -> Victory
    TEST_ASSERT(!prog.advanceStage(), "Advance stage on Stage 9 must return false (campaign victory)");
    TEST_ASSERT(prog.isCampaignComplete(), "Campaign must be marked complete");
}

static void testCheckpointManagement() noexcept
{
    StageProgressionSystem prog { };
    TEST_ASSERT(!prog.hasCheckpoint(), "Initial state must have no checkpoint");

    prog.setCheckpoint(WorldPos { 250.0f, 180.0f });
    TEST_ASSERT(prog.hasCheckpoint(), "hasCheckpoint must return true after setCheckpoint");
    TEST_ASSERT(prog.checkpoint().x == 250.0f && prog.checkpoint().y == 180.0f, "Checkpoint coordinates must match");

    prog.clearCheckpoint();
    TEST_ASSERT(!prog.hasCheckpoint(), "hasCheckpoint must return false after clearCheckpoint");
}

static void testPortalTriggerDetection() noexcept
{
    const WorldPos portal { 200.0f, 150.0f };

    // Player within 10 px
    TEST_ASSERT(StageProgressionSystem::isAtPortal(WorldPos { 205.0f, 152.0f }, portal, 24.0f),
        "Player within 24 px must trigger portal");

    // Player outside 24 px
    TEST_ASSERT(!StageProgressionSystem::isAtPortal(WorldPos { 240.0f, 150.0f }, portal, 24.0f),
        "Player at 40 px must not trigger portal");
}

static void testStageCodenames() noexcept
{
    StageProgressionSystem prog { };
    prog.startCampaign();
    TEST_ASSERT(prog.currentStageName().find("STAGE 1") != std::string_view::npos, "Stage 1 name check");

    prog.advanceStage();
    TEST_ASSERT(prog.currentStageName().find("STAGE 2") != std::string_view::npos, "Stage 2 name check");
}

void runStageProgressionTests() noexcept
{
    testCampaignProgressionFlow();
    testCheckpointManagement();
    testPortalTriggerDetection();
    testStageCodenames();
}

} // namespace ShooterCore::Test
