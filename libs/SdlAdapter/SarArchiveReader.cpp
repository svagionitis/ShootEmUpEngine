#include "SarArchiveReader.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <fstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "SarDecompressor.h"

namespace SdlAdapter {

namespace {

    constexpr std::size_t kCatalogRecordSize { 18U };

    bool compareToken(const char* entryToken, std::string_view searchToken) noexcept
    {
        std::size_t i { 0U };
        while (i < 8U && entryToken[i] != '\0' && entryToken[i] != ' ' && i < searchToken.size()) {
            if (entryToken[i] != searchToken[i]) {
                return false;
            }
            ++i;
        }

        const bool entryEnd { i >= 8U || entryToken[i] == '\0' || entryToken[i] == ' ' };
        const bool searchEnd { i >= searchToken.size() };
        return entryEnd && searchEnd;
    }

} // namespace

SarArchiveReader::~SarArchiveReader() noexcept
{
    close();
}

SarArchiveReader::SarArchiveReader(SarArchiveReader&& other) noexcept
    : sarPath_(std::move(other.sarPath_))
    , entries_(std::move(other.entries_))
{
}

SarArchiveReader& SarArchiveReader::operator=(SarArchiveReader&& other) noexcept
{
    if (this != &other) {
        sarPath_ = std::move(other.sarPath_);
        entries_ = std::move(other.entries_);
    }
    return *this;
}

bool SarArchiveReader::open(const std::string& ixPath, const std::string& sarPath)
{
    close();

    std::ifstream ixFile(ixPath, std::ios::binary);
    if (!ixFile.is_open()) {
        return false;
    }

    std::ifstream sarCheck(sarPath, std::ios::binary);
    if (!sarCheck.is_open()) {
        return false;
    }
    sarCheck.close();

    ixFile.seekg(0, std::ios::end);
    const auto ixSize { static_cast<std::size_t>(ixFile.tellg()) };
    ixFile.seekg(0, std::ios::beg);

    if (ixSize == 0U || (ixSize % kCatalogRecordSize) != 0U) {
        return false;
    }

    const std::size_t count { ixSize / kCatalogRecordSize };
    entries_.resize(count);

    std::vector<std::uint8_t> record(kCatalogRecordSize);
    for (std::size_t i { 0U }; i < count; ++i) {
        ixFile.read(reinterpret_cast<char*>(record.data()), static_cast<std::streamsize>(kCatalogRecordSize));
        if (static_cast<std::size_t>(ixFile.gcount()) < kCatalogRecordSize) {
            close();
            return false;
        }

        SarEntry& entry { entries_[i] };
        for (std::size_t c { 0U }; c < 8U; ++c) {
            entry.token[c] = static_cast<char>(record[c]);
        }
        entry.token[8] = '\0';

        entry.flags = static_cast<std::uint16_t>(record[8] | (static_cast<std::uint16_t>(record[9]) << 8U));
        entry.uncompressed = static_cast<std::uint16_t>(record[10] | (static_cast<std::uint16_t>(record[11]) << 8U));
        entry.compressed = static_cast<std::uint16_t>(record[12] | (static_cast<std::uint16_t>(record[13]) << 8U));

        const auto offLow { static_cast<std::uint32_t>(record[14] | (static_cast<std::uint32_t>(record[15]) << 8U)) };
        const auto offHigh { static_cast<std::uint32_t>(record[16] | (static_cast<std::uint32_t>(record[17]) << 8U)) };
        entry.offset = (offHigh << 16U) | offLow;
    }

    sarPath_ = sarPath;
    return true;
}

void SarArchiveReader::close() noexcept
{
    sarPath_.clear();
    entries_.clear();
}

bool SarArchiveReader::isOpen() const noexcept
{
    return !sarPath_.empty() && !entries_.empty();
}

std::size_t SarArchiveReader::entryCount() const noexcept
{
    return entries_.size();
}

bool SarArchiveReader::hasEntry(std::string_view token) const noexcept
{
    return findEntry(token) != nullptr;
}

const SarEntry* SarArchiveReader::findEntry(std::string_view token) const noexcept
{
    for (const auto& entry : entries_) {
        if (compareToken(entry.token, token)) {
            return &entry;
        }
    }
    return nullptr;
}

bool SarArchiveReader::extractEntry(std::string_view token, std::vector<std::uint8_t>& outData) const
{
    const SarEntry* entry { findEntry(token) };
    if (entry == nullptr) {
        return false;
    }

    std::ifstream sarFile(sarPath_, std::ios::binary);
    if (!sarFile.is_open()) {
        return false;
    }

    sarFile.seekg(static_cast<std::streamoff>(entry->offset), std::ios::beg);
    outData.resize(entry->compressed);

    sarFile.read(reinterpret_cast<char*>(outData.data()), static_cast<std::streamsize>(entry->compressed));
    const auto readLen { static_cast<std::size_t>(sarFile.gcount()) };
    if (readLen < entry->compressed) {
        outData.resize(readLen);
        return false;
    }

    return true;
}

bool SarArchiveReader::readEntryRaw(
    std::string_view token, std::uint8_t* outBuf, std::size_t bufSize, std::size_t& bytesRead) const noexcept
{
    bytesRead = 0U;
    if (outBuf == nullptr || bufSize == 0U) {
        return false;
    }

    const SarEntry* entry { findEntry(token) };
    if (entry == nullptr) {
        return false;
    }

    std::ifstream sarFile(sarPath_, std::ios::binary);
    if (!sarFile.is_open()) {
        return false;
    }

    sarFile.seekg(static_cast<std::streamoff>(entry->offset), std::ios::beg);
    const auto toRead { std::min(bufSize, static_cast<std::size_t>(entry->compressed)) };

    sarFile.read(reinterpret_cast<char*>(outBuf), static_cast<std::streamsize>(toRead));
    bytesRead = static_cast<std::size_t>(sarFile.gcount());
    return bytesRead > 0U;
}

bool SarArchiveReader::decompressEntry(std::string_view token, std::vector<std::uint8_t>& outData) const
{
    const SarEntry* entry { findEntry(token) };
    if (entry == nullptr) {
        return false;
    }

    if (entry->compressed == entry->uncompressed) {
        return extractEntry(token, outData);
    }

    std::vector<std::uint8_t> compressedData;
    if (!extractEntry(token, compressedData)) {
        return false;
    }

    outData.resize(entry->uncompressed);
    std::size_t written { 0U };
    if (!SarDecompressor::decompress(
            compressedData.data(), compressedData.size(), outData.data(), outData.size(), written)) {
        outData.clear();
        return false;
    }

    outData.resize(written);
    return written == static_cast<std::size_t>(entry->uncompressed);
}

bool SarArchiveReader::readEntryDecompressed(
    std::string_view token, std::uint8_t* outBuf, std::size_t bufSize, std::size_t& bytesWritten) const noexcept
{
    bytesWritten = 0U;
    if (outBuf == nullptr || bufSize == 0U) {
        return false;
    }

    const SarEntry* entry { findEntry(token) };
    if (entry == nullptr) {
        return false;
    }

    if (entry->compressed == entry->uncompressed) {
        return readEntryRaw(token, outBuf, bufSize, bytesWritten);
    }

    // Stack buffer for compressed payload (max compressed size in Firehawk is < 48KB)
    std::vector<std::uint8_t> compBuf;
    if (!extractEntry(token, compBuf)) {
        return false;
    }

    return SarDecompressor::decompress(compBuf.data(), compBuf.size(), outBuf, bufSize, bytesWritten);
}

} // namespace SdlAdapter
