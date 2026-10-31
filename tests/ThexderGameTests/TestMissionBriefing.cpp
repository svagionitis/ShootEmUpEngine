#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>
#include <string_view>

#include "AudioCommandQueue.h"
#include "MissionBriefingSystem.h"
#include "Types.h"

namespace ThexderGame::Test {

TEST(ThexderGameTests, MissionBriefingSystem)
{
    MissionBriefingSystem briefing { };
    ShooterCore::AudioCommandQueue audio { };

    briefing.startBriefing(1U);
    EXPECT_FALSE(briefing.isComplete());
    EXPECT_FALSE(briefing.isPageRevealed());
    EXPECT_EQ(briefing.stageNumber(), 1U);

    // Update for 0.5s (15 characters revealed)
    const ShooterCore::DeltaTime dt { 0.5f };
    briefing.update(dt, audio);
    EXPECT_FALSE(briefing.visibleText().empty());
    EXPECT_LT(briefing.visibleText().size(), 30U);

    // Fast-forward reveals current page fully
    briefing.fastForward();
    EXPECT_TRUE(briefing.isPageRevealed());

    // Advance to page 2 of Stage 1
    EXPECT_TRUE(briefing.advancePage());
    EXPECT_FALSE(briefing.isPageRevealed());

    // Fast forward page 2
    briefing.fastForward();
    EXPECT_TRUE(briefing.isPageRevealed());

    // Advance past page 2 finishes briefing
    EXPECT_FALSE(briefing.advancePage());
    EXPECT_TRUE(briefing.isComplete());
}

} // namespace ThexderGame::Test
