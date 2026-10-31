#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include <vector>

#include "SarArchiveReader.h"
#include "SarDecompressor.h"

namespace SdlAdapter::Test {

void runSarDecompressorTests() noexcept
{
    LOG(INFO) << "Executing SarDecompressor Tests...";

    // 1. Invalid input error handling
    std::size_t written { 0U };
    TEST_ASSERT_FALSE(
        SarDecompressor::decompress(nullptr, 0U, nullptr, 0U, written), "Null inputs must safely return false");
    TEST_ASSERT_EQ(written, 0U, "Written count must remain 0 on error");

    // 2. Decompress authentic assets from RES1.SAR
    SarArchiveReader reader;
    if (reader.open("assets/firehawk/RES1.IX", "assets/firehawk/RES1.SAR")) {
        // Test KA00 cutscene frame (compressed: 501, uncompressed: 768)
        std::vector<std::uint8_t> ka00;
        TEST_ASSERT_TRUE(reader.decompressEntry("KA00", ka00), "KA00 decompression must succeed");
        TEST_ASSERT_EQ(ka00.size(), 768U, "KA00 uncompressed size must be 768 bytes");

        // Test KM11 stage layout (compressed: 1130, uncompressed: 1599)
        std::vector<std::uint8_t> km11;
        TEST_ASSERT_TRUE(reader.decompressEntry("KM11", km11), "KM11 decompression must succeed");
        TEST_ASSERT_EQ(km11.size(), 1599U, "KM11 uncompressed size must be 1599 bytes");

        // Test CT11 stage tileset (compressed: 336, uncompressed: 1536)
        std::vector<std::uint8_t> ct11;
        TEST_ASSERT_TRUE(reader.decompressEntry("CT11", ct11), "CT11 decompression must succeed");
        TEST_ASSERT_EQ(ct11.size(), 1536U, "CT11 uncompressed size must be 1536 bytes");

        // Test EG11 enemy sprite atlas (compressed: 332, uncompressed: 736)
        std::vector<std::uint8_t> eg11;
        TEST_ASSERT_TRUE(reader.decompressEntry("EG11", eg11), "EG11 decompression must succeed");
        TEST_ASSERT_EQ(eg11.size(), 736U, "EG11 uncompressed size must be 736 bytes");

        // Test zero-heap readEntryDecompressed
        std::vector<std::uint8_t> staticBuf(2048U, 0U);
        std::size_t bytesOut { 0U };
        TEST_ASSERT_TRUE(reader.readEntryDecompressed("KA00", staticBuf.data(), staticBuf.size(), bytesOut),
            "readEntryDecompressed for KA00 must succeed");
        TEST_ASSERT_EQ(bytesOut, 768U, "Zero-heap read must output exactly 768 bytes");

        reader.close();
    }
}

} // namespace SdlAdapter::Test
