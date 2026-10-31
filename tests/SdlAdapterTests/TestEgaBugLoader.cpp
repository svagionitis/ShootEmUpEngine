#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include <vector>

#include "EgaBugLoader.h"
#include "NestManager.h"
#include "Types.h"

namespace SdlAdapter::Test {

void runEgaBugLoaderTests() noexcept
{
    LOG(INFO) << "Executing EgaBugLoader Alien Database Tests...";

    ShooterCore::NestManager nestMgr { };

    // 1. Rejection of invalid / truncated buffers
    TEST_ASSERT_EQ(EgaBugLoader::decodeBugDb(nullptr, 3404U, nestMgr), 0U, "Must return 0 on null pointer");
    std::vector<std::uint8_t> smallBuf(100U, 0U);
    TEST_ASSERT_EQ(
        EgaBugLoader::decodeBugDb(smallBuf.data(), smallBuf.size(), nestMgr), 0U, "Must return 0 on undersized buffer");

    // 2. Synthetic controlled record decoding
    // kDataStartOffset is (4 * 74) + 24 = 320 bytes.
    // Allocate 320 + 24 bytes (enough for 2 entries of 12 bytes each)
    std::vector<std::uint8_t> synthBuf(320U + 24U, 0U);

    // Entry 0: worldY = 128 (0x0080), worldX = 696 (0x02B8)
    synthBuf[320U + 0U] = 0x80U;
    synthBuf[320U + 1U] = 0x00U;
    synthBuf[320U + 2U] = 0xB8U;
    synthBuf[320U + 3U] = 0x02U;

    // Entry 1: worldY = 160 (0x00A0), worldX = 1454 (0x05AE)
    synthBuf[320U + 12U] = 0xA0U;
    synthBuf[320U + 13U] = 0x00U;
    synthBuf[320U + 14U] = 0xAEU;
    synthBuf[320U + 15U] = 0x05U;

    nestMgr.clear();
    const std::size_t added { EgaBugLoader::decodeBugDb(synthBuf.data(), synthBuf.size(), nestMgr) };
    TEST_ASSERT_EQ(added, 2U, "Must decode exactly 2 synthetic alien nests");
    TEST_ASSERT_EQ(nestMgr.nestCount(), 2U, "Nest count in manager must be 2");

    const auto& nests { nestMgr.getNests() };
    TEST_ASSERT_EQ(nests[0].pos.x, 696.0f, "Nest 0 X must match 696.0");
    TEST_ASSERT_EQ(nests[0].pos.y, 128.0f, "Nest 0 Y must match 128.0");
    TEST_ASSERT_EQ(nests[1].pos.x, 1454.0f, "Nest 1 X must match 1454.0");
    TEST_ASSERT_EQ(nests[1].pos.y, 160.0f, "Nest 1 Y must match 160.0");

    // 3. Load authentic BUGDB01.BIN from disk if present
    nestMgr.clear();
    const std::size_t diskNests { EgaBugLoader::loadBugDb("assets/thexder/BUGDB01.BIN", nestMgr) };
    if (diskNests > 0U) {
        TEST_ASSERT(nestMgr.nestCount() > 0U, "NestManager must contain loaded nests");
        TEST_ASSERT(
            nestMgr.nestCount() <= ShooterCore::NestManager::kMaxNestSlots, "Loaded nests must not exceed max slots");
        LOG(INFO) << "Verified authentic BUGDB01.BIN ingestion: " << diskNests << " nests loaded cleanly!";
    }
}

} // namespace SdlAdapter::Test
