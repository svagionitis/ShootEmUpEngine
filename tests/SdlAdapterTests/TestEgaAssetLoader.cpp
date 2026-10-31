#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include <array>
#include <vector>

#include "EgaAssetLoader.h"

namespace SdlAdapter::Test {

void runEgaAssetLoaderTests() noexcept
{
    LOG(INFO) << "Executing EgaAssetLoader 4-Plane Bitplane Tests...";

    // 1. Rejection of invalid / null buffers
    std::array<std::uint32_t, 16 * 16> outTile { };
    TEST_ASSERT_EQ(
        EgaAssetLoader::decodeTiles(nullptr, 8192U, outTile.data(), 1U), 0U, "decodeTiles must return 0 on null data");
    TEST_ASSERT_EQ(
        EgaAssetLoader::decodeTiles(reinterpret_cast<const std::uint8_t*>(&outTile), 100U, outTile.data(), 1U), 0U,
        "decodeTiles must return 0 on undersized buffer");
    TEST_ASSERT_EQ(EgaAssetLoader::decodeTiles(reinterpret_cast<const std::uint8_t*>(&outTile), 8192U, nullptr, 1U), 0U,
        "decodeTiles must return 0 on null output buffer");

    // 2. Controlled 4-plane bitplane decoding
    // Create an 8192-byte bank with 4 planes:
    // We will set Tile 0, row 0, pixel 0 to White (color 15: all 4 planes bit 7 = 1)
    // and pixel 1 to Red (color 4: Plane 2 bit 6 = 1)
    // and pixel 2 to Blue (color 1: Plane 0 bit 5 = 1)
    // and pixel 3 to Green (color 2: Plane 1 bit 4 = 1)
    std::vector<std::uint8_t> testBank(EgaAssetLoader::kBankTotalSize, 0U);

    // Plane 0 (Blue) at offset 0
    testBank[0U * 2048U + 0U] = 0b10100000U; // Pixels 0 and 2
    // Plane 1 (Green) at offset 2048
    testBank[1U * 2048U + 0U] = 0b10010000U; // Pixels 0 and 3
    // Plane 2 (Red) at offset 4096
    testBank[2U * 2048U + 0U] = 0b11000000U; // Pixels 0 and 1
    // Plane 3 (Intensity) at offset 6144
    testBank[3U * 2048U + 0U] = 0b10000000U; // Pixel 0

    std::vector<std::uint32_t> decodedTiles(64U * 16U * 16U, 0U);
    const std::size_t decodedCount { EgaAssetLoader::decodeTiles(
        testBank.data(), testBank.size(), decodedTiles.data(), 64U) };
    TEST_ASSERT_EQ(decodedCount, 64U, "Must decode exactly 64 tiles");

    // Verify Pixel 0 is White (15)
    TEST_ASSERT_EQ(decodedTiles[0], kEgaPaletteRgba[15], "Pixel 0 must match White (0xFFFFFFFF)");
    // Verify Pixel 1 is Red (4)
    TEST_ASSERT_EQ(decodedTiles[1], kEgaPaletteRgba[4], "Pixel 1 must match Red (0xFF0000AA)");
    // Verify Pixel 2 is Blue (1)
    TEST_ASSERT_EQ(decodedTiles[2], kEgaPaletteRgba[1], "Pixel 2 must match Blue (0xFFAA0000)");
    // Verify Pixel 3 is Green (2)
    TEST_ASSERT_EQ(decodedTiles[3], kEgaPaletteRgba[2], "Pixel 3 must match Green (0xFF00AA00)");
    // Verify Pixel 4 is Black (0)
    TEST_ASSERT_EQ(decodedTiles[4], kEgaPaletteRgba[0], "Pixel 4 must match Black (0xFF000000)");

    // 3. Load actual EGABIT01.BIN from disk if present in assets/thexder
    std::vector<std::uint32_t> realTiles(64U * 16U * 16U, 0U);
    const std::size_t loadedReal { EgaAssetLoader::loadTiles("assets/thexder/EGABIT01.BIN", realTiles.data(), 64U) };
    if (loadedReal == 64U) {
        LOG(INFO) << "Verified authentic EGABIT01.BIN load: 64 tiles decoded";
        TEST_ASSERT_EQ(loadedReal, 64U, "Must load 64 tiles from authentic EGABIT01.BIN");
        // Check that Tile 0 (rounded masonry wall) has non-black pixels
        std::size_t nonZeroCount { 0U };
        for (std::size_t i { 0U }; i < 256U; ++i) {
            if (realTiles[i] != kEgaPaletteRgba[0]) {
                ++nonZeroCount;
            }
        }
        TEST_ASSERT_TRUE(nonZeroCount > 100U, "Tile 0 must have solid masonry pixels");
    }

    // 4. Test Title Screen Decoding
    std::vector<std::uint8_t> titleBuffer(EgaAssetLoader::kTitleTotalSize, 0U);
    std::vector<std::uint32_t> titleRgba(EgaAssetLoader::kTitleWidth * EgaAssetLoader::kTitleHeight, 0U);
    TEST_ASSERT_FALSE(EgaAssetLoader::decodeTitle(nullptr, 31360U, titleRgba.data()), "decodeTitle must fail on null");
    TEST_ASSERT_FALSE(EgaAssetLoader::decodeTitle(titleBuffer.data(), 1000U, titleRgba.data()),
        "decodeTitle must fail on small buffer");
    TEST_ASSERT_TRUE(EgaAssetLoader::decodeTitle(titleBuffer.data(), titleBuffer.size(), titleRgba.data()),
        "decodeTitle must succeed on valid buffer");

    // Load actual EGATITLE.BIN if present
    if (EgaAssetLoader::loadTitle("assets/thexder/EGATITLE.BIN", titleRgba.data())) {
        LOG(INFO) << "Verified authentic EGATITLE.BIN load (320x196)";
    }
}

} // namespace SdlAdapter::Test
