#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include "CutscenePlayer.h"
#include "SarArchiveReader.h"
#include "Types.h"

namespace SdlAdapter::Test {

void runCutscenePlayerTests() noexcept
{
    LOG(INFO) << "Executing CutscenePlayer Tests...";

    CutscenePlayer player;
    TEST_ASSERT_FALSE(player.hasLoadedData(), "Initially no data loaded");
    TEST_ASSERT_EQ(player.currentFrameIndex(), 0U, "Initial frame index must be 0");
    TEST_ASSERT_EQ(static_cast<std::uint8_t>(player.emotion()), static_cast<std::uint8_t>(ArthurEmotion::Normal),
        "Initial emotion must be Normal");

    // Load authentic KA00 cutscene frame from RES1.SAR
    SarArchiveReader reader;
    if (reader.open("assets/firehawk/RES1.IX", "assets/firehawk/RES1.SAR")) {
        const bool loaded { player.loadCutscene(reader, "KA00") };
        TEST_ASSERT_TRUE(loaded, "Loading KA00 must succeed");
        TEST_ASSERT_TRUE(player.hasLoadedData(), "hasLoadedData must return true");
        TEST_ASSERT_TRUE(player.rawFrameData() != nullptr, "rawFrameData pointer must be non-null");

        // Advance animation by 0.25s (2 frames at 8 FPS)
        const ShooterCore::DeltaTime dt { 0.25f };
        player.update(dt);
        TEST_ASSERT_TRUE(player.currentFrameIndex() > 0U, "Frame index should advance after update");

        // Set emotion
        player.setEmotion(ArthurEmotion::Critical);
        TEST_ASSERT_EQ(static_cast<std::uint8_t>(player.emotion()), static_cast<std::uint8_t>(ArthurEmotion::Critical),
            "Emotion should update to Critical");

        reader.close();
    }
}

} // namespace SdlAdapter::Test
