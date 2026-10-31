#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include <vector>

#include "EgaRobotLoader.h"

namespace SdlAdapter::Test {

void runEgaRobotLoaderTests() noexcept
{
    LOG(INFO) << "Executing EgaRobotLoader 48x32 Mecha Frame Tests...";

    // 1. Rejection of invalid / null buffers
    std::vector<std::uint32_t> outRgba(48U * 32U * 2U, 0U);
    TEST_ASSERT_EQ(EgaRobotLoader::decodeSprites(nullptr, 16992U, outRgba.data(), 1U), 0U,
        "decodeSprites must return 0 on null pointer");
    TEST_ASSERT_EQ(
        EgaRobotLoader::decodeSprites(reinterpret_cast<const std::uint8_t*>(outRgba.data()), 50U, outRgba.data(), 1U),
        0U, "decodeSprites must return 0 on undersized buffer");
    TEST_ASSERT_EQ(
        EgaRobotLoader::decodeSprites(reinterpret_cast<const std::uint8_t*>(outRgba.data()), 192U, nullptr, 1U), 0U,
        "decodeSprites must return 0 on null output");

    // 2. Controlled 48-bit scanline unpack
    // Create 1 frame (192 bytes = 32 rows * 6 bytes)
    // Row 0: 6 bytes = 0xAA, 0x55, 0x00, 0x00, 0x00, 0x00
    // 0xAA = 10101010, 0x55 = 01010101
    std::vector<std::uint8_t> testSprite(192U, 0U);
    testSprite[0] = 0xAAU;
    testSprite[1] = 0x55U;

    constexpr std::uint32_t kFgColor { 0xFF00FF00U }; // Bright Green
    constexpr std::uint32_t kBgColor { 0x00000000U }; // Transparent

    std::vector<std::uint32_t> framePixels(48U * 32U, 0U);
    const std::size_t decoded { EgaRobotLoader::decodeSprites(
        testSprite.data(), testSprite.size(), framePixels.data(), 1U, kFgColor, kBgColor) };

    TEST_ASSERT_EQ(decoded, 1U, "Must decode 1 test frame");

    // Check first 8 pixels (0xAA = 1, 0, 1, 0, 1, 0, 1, 0)
    TEST_ASSERT_EQ(framePixels[0], kFgColor, "Pixel 0 must be FG");
    TEST_ASSERT_EQ(framePixels[1], kBgColor, "Pixel 1 must be BG");
    TEST_ASSERT_EQ(framePixels[2], kFgColor, "Pixel 2 must be FG");
    TEST_ASSERT_EQ(framePixels[3], kBgColor, "Pixel 3 must be BG");
    TEST_ASSERT_EQ(framePixels[4], kFgColor, "Pixel 4 must be FG");
    TEST_ASSERT_EQ(framePixels[5], kBgColor, "Pixel 5 must be BG");
    TEST_ASSERT_EQ(framePixels[6], kFgColor, "Pixel 6 must be FG");
    TEST_ASSERT_EQ(framePixels[7], kBgColor, "Pixel 7 must be BG");

    // Check next 8 pixels (0x55 = 0, 1, 0, 1, 0, 1, 0, 1)
    TEST_ASSERT_EQ(framePixels[8], kBgColor, "Pixel 8 must be BG");
    TEST_ASSERT_EQ(framePixels[9], kFgColor, "Pixel 9 must be FG");
    TEST_ASSERT_EQ(framePixels[10], kBgColor, "Pixel 10 must be BG");
    TEST_ASSERT_EQ(framePixels[11], kFgColor, "Pixel 11 must be FG");

    // 3. Load authentic EGAROBOT.BIN if available
    std::vector<std::uint32_t> realSprites(88U * 48U * 32U, 0U);
    const std::size_t loadedReal { EgaRobotLoader::loadSprites(
        "assets/thexder/EGAROBOT.BIN", realSprites.data(), 88U) };
    if (loadedReal == 88U) {
        LOG(INFO) << "Verified authentic EGAROBOT.BIN load: 88 frames decoded";
        TEST_ASSERT_EQ(loadedReal, 88U, "Must load 88 frames from authentic EGAROBOT.BIN");

        // Verify that Frame 0 (Robot head) contains lit pixels
        std::size_t litCount { 0U };
        for (std::size_t i { 0U }; i < 48U * 32U; ++i) {
            if (realSprites[i] != 0U) {
                ++litCount;
            }
        }
        TEST_ASSERT_TRUE(litCount > 100U, "Frame 0 must contain lit mecha pixels");
    }
}

} // namespace SdlAdapter::Test
