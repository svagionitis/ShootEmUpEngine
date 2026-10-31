#include "TestHarness.h"

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "SarArchiveReader.h"
#include "SarStageLoader.h"
#include "TileMap.h"

namespace SdlAdapter::Test {

void runSarStageLoaderTests() noexcept
{
    LOG(INFO) << "Executing SarStageLoader Tests...";

    // 1. Test token formatting
    char token[5] { 0 };
    SarStageLoader::formatStageToken(1U, 1U, token);
    TEST_ASSERT_EQ(std::string_view(token), "KM11", "Stage 1 Zone 1 must format to KM11");

    SarStageLoader::formatStageToken(2U, 4U, token);
    TEST_ASSERT_EQ(std::string_view(token), "KM24", "Stage 2 Zone 4 must format to KM24");

    // 2. Test archive indexing
    TEST_ASSERT_EQ(SarStageLoader::archiveIndexForStage(1U), 1U, "Stage 1 is in archive 1");
    TEST_ASSERT_EQ(SarStageLoader::archiveIndexForStage(3U), 2U, "Stage 3 is in archive 2");
    TEST_ASSERT_EQ(SarStageLoader::archiveIndexForStage(7U), 3U, "Stage 7 is in archive 3");

    // 3. Load authentic Stage 1 Zone 1 into a TileMap
    SarArchiveReader reader;
    if (reader.open("assets/firehawk/RES1.IX", "assets/firehawk/RES1.SAR")) {
        ShooterCore::TileMap map { 128U, 32U, 16.0f };
        const bool loaded { SarStageLoader::loadStageZone(reader, 1U, 1U, map) };
        TEST_ASSERT_TRUE(loaded, "Loading KM11 into TileMap must succeed");

        // Verify map boundaries are solid
        TEST_ASSERT_TRUE(map.isSolid(ShooterCore::TileCoord { 0U, 0U }), "Top-left boundary must be solid");
        TEST_ASSERT_TRUE(map.isSolid(ShooterCore::TileCoord { 127U, 31U }), "Bottom-right boundary must be solid");

        reader.close();
    }
}

} // namespace SdlAdapter::Test
