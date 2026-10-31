#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include <vector>

#include "SarArchiveReader.h"

namespace SdlAdapter::Test {

void runSarArchiveReaderTests() noexcept
{
    LOG(INFO) << "Executing SarArchiveReader Tests...";

    SarArchiveReader reader;
    TEST_ASSERT_FALSE(reader.isOpen(), "New reader must not be open");
    TEST_ASSERT_EQ(reader.entryCount(), 0U, "Initial count must be 0");

    // 1. Invalid file path handling
    TEST_ASSERT_FALSE(reader.open("nonexistent.ix", "nonexistent.sar"), "Must fail on nonexistent files");

    // 2. Load authentic RES1.IX and RES1.SAR if present
    if (reader.open("assets/firehawk/RES1.IX", "assets/firehawk/RES1.SAR")) {
        TEST_ASSERT_TRUE(reader.isOpen(), "Must be open with valid files");
        TEST_ASSERT_EQ(reader.entryCount(), 46U, "RES1.IX must contain exactly 46 catalog entries");

        // Verify key tokens from reverse engineering
        TEST_ASSERT_TRUE(reader.hasEntry("KA00"), "Must contain KA00 entry");
        TEST_ASSERT_TRUE(reader.hasEntry("AD01"), "Must contain AD01 entry");
        TEST_ASSERT_TRUE(reader.hasEntry("CG01"), "Must contain CG01 entry");
        TEST_ASSERT_TRUE(reader.hasEntry("CT01"), "Must contain CT01 entry");
        TEST_ASSERT_TRUE(reader.hasEntry("KM01"), "Must contain KM01 stage map entry");
        TEST_ASSERT_FALSE(reader.hasEntry("NONEXIST"), "Must return false for unknown token");

        const SarEntry* ka00 { reader.findEntry("KA00") };
        TEST_ASSERT_TRUE(ka00 != nullptr, "KA00 metadata pointer must be non-null");
        if (ka00 != nullptr) {
            TEST_ASSERT_EQ(ka00->uncompressed, 768U, "KA00 uncompressed size must be 768");
            TEST_ASSERT_EQ(ka00->compressed, 501U, "KA00 compressed size must be 501");
            TEST_ASSERT_EQ(ka00->offset, 13U, "KA00 offset in SAR must be 13");
        }

        // Test extraction into vector
        std::vector<std::uint8_t> ka00Data;
        TEST_ASSERT_TRUE(reader.extractEntry("KA00", ka00Data), "extractEntry for KA00 must succeed");
        TEST_ASSERT_EQ(ka00Data.size(), 501U, "Extracted data length must match compressed size 501");

        // Test zero-heap readEntryRaw
        std::vector<std::uint8_t> rawBuf(1024U, 0U);
        std::size_t bytesRead { 0U };
        TEST_ASSERT_TRUE(
            reader.readEntryRaw("KA00", rawBuf.data(), rawBuf.size(), bytesRead), "readEntryRaw must succeed");
        TEST_ASSERT_EQ(bytesRead, 501U, "readEntryRaw bytesRead must be 501");

        reader.close();
        TEST_ASSERT_FALSE(reader.isOpen(), "Must be closed after close()");
    }

    // 3. Test multi-volume offset handling with RES2.IX
    SarArchiveReader reader2;
    if (reader2.open("assets/firehawk/RES2.IX", "assets/firehawk/RES2.SAR")) {
        TEST_ASSERT_TRUE(reader2.isOpen(), "RES2 reader must be open");
        TEST_ASSERT_EQ(reader2.entryCount(), 119U, "RES2 must have 119 entries");

        const SarEntry* ka10 { reader2.findEntry("KA10") };
        TEST_ASSERT_TRUE(ka10 != nullptr, "KA10 must exist in RES2");
        LOG(INFO) << "Verified RES2 archive opened cleanly with 119 entries";
    }
}

} // namespace SdlAdapter::Test
