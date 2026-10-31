#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include <vector>

#include "EgaMapLoader.h"
#include "TileMap.h"
#include "Types.h"

namespace SdlAdapter::Test {

void runEgaMapLoaderTests() noexcept
{
    LOG(INFO) << "Executing EgaMapLoader Column Decoding Tests...";

    ShooterCore::TileMap testMap { 320U, 32U };

    // 1. Rejection of invalid inputs
    TEST_ASSERT(!EgaMapLoader::decodeMap(nullptr, 2048U, nullptr, 5803U, testMap), "Must reject null buffers");
    std::vector<std::uint8_t> dummy(100U, 0U);
    TEST_ASSERT(!EgaMapLoader::decodeMap(dummy.data(), 100U, dummy.data(), 100U, testMap),
        "Must reject undersized pointer table");

    // 2. Unit test for parseTileByte
    std::uint8_t tIdx { 0U };
    TEST_ASSERT_EQ(static_cast<std::uint8_t>(EgaMapLoader::parseTileByte(0x00U, tIdx)),
        static_cast<std::uint8_t>(ShooterCore::TileType::Empty), "0x00 must parse as Empty");
    TEST_ASSERT_EQ(tIdx, 0U, "Tile index for 0x00 must be 0");

    TEST_ASSERT_EQ(static_cast<std::uint8_t>(EgaMapLoader::parseTileByte(0x0EU, tIdx)),
        static_cast<std::uint8_t>(ShooterCore::TileType::SolidWall), "0x0E must parse as SolidWall");
    TEST_ASSERT_EQ(tIdx, 14U, "Tile index for 0x0E must be 14");

    TEST_ASSERT_EQ(static_cast<std::uint8_t>(EgaMapLoader::parseTileByte(0x40U | 14U, tIdx)),
        static_cast<std::uint8_t>(ShooterCore::TileType::DestructibleWall), "0x40|14 must parse as DestructibleWall");
    TEST_ASSERT_EQ(tIdx, 14U, "Tile index for 0x4E must be 14");

    TEST_ASSERT_EQ(static_cast<std::uint8_t>(EgaMapLoader::parseTileByte(0x80U | 18U, tIdx)),
        static_cast<std::uint8_t>(ShooterCore::TileType::SecretWall), "0x80|18 must parse as SecretWall");
    TEST_ASSERT_EQ(tIdx, 18U, "Tile index for 0x92 must be 18");

    TEST_ASSERT_EQ(static_cast<std::uint8_t>(EgaMapLoader::parseTileByte(0xC0U | 3U, tIdx)),
        static_cast<std::uint8_t>(ShooterCore::TileType::Hazard), "0xC0|3 must parse as Hazard");
    TEST_ASSERT_EQ(tIdx, 3U, "Tile index for 0xC3 must be 3");

    // 3. Controlled synthetic column pointer decoding
    std::vector<std::uint8_t> ptrBuf(EgaMapLoader::kPtrTableBytes, 0U);
    std::vector<std::uint8_t> mapBuf(1024U, 0U);

    // Point Column 0 (Row 0, Block 0, Col 0 -> Word 0) to map offset 32
    ptrBuf[0U] = 32U;
    ptrBuf[1U] = 0U;

    // Fill offset 32..63 in mapBuf: row 0 is solid (0x0E), row 1 is destructible (0x40 | 14)
    mapBuf[32U + 0U] = 14U;
    mapBuf[32U + 1U] = 0x40U | 14U;

    const bool decodedOk { EgaMapLoader::decodeMap(
        ptrBuf.data(), ptrBuf.size(), mapBuf.data(), mapBuf.size(), testMap) };
    TEST_ASSERT(decodedOk, "Synthetic decodeMap must succeed");

    TEST_ASSERT_EQ(static_cast<std::uint8_t>(testMap.getTile(ShooterCore::TileCoord { 0U, 0U })),
        static_cast<std::uint8_t>(ShooterCore::TileType::SolidWall), "Col 0, Row 0 must be SolidWall");
    TEST_ASSERT_EQ(static_cast<std::uint8_t>(testMap.getTile(ShooterCore::TileCoord { 0U, 1U })),
        static_cast<std::uint8_t>(ShooterCore::TileType::DestructibleWall), "Col 0, Row 1 must be DestructibleWall");

    // 4. Load authentic assets from disk if available
    ShooterCore::TileMap authenticMap { 320U, 32U };
    const bool loaded { EgaMapLoader::loadMap(
        "assets/thexder/EGAPTR01.BIN", "assets/thexder/MAP01.BIN", authenticMap) };
    if (loaded) {
        TEST_ASSERT_EQ(authenticMap.width(), 320U, "Authentic map width must be 320");
        TEST_ASSERT_EQ(authenticMap.height(), 32U, "Authentic map height must be 32");
        LOG(INFO) << "Verified authentic EGAPTR01.BIN & MAP01.BIN disk ingestion cleanly!";
    }
}

} // namespace SdlAdapter::Test
