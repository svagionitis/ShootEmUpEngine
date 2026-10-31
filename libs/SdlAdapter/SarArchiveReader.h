#pragma once

#include <cstddef>
#include <cstdint>

#include <string>
#include <string_view>
#include <vector>

namespace SdlAdapter {

/// @brief Header entry for a resource inside a Sierra/Game Arts SAR archive.
struct SarEntry {
    char token[9] { 0 }; ///< Null-terminated 8-character ASCII asset tag
    std::uint16_t flags { 0U }; ///< Internal flags (typically 0x7000)
    std::uint16_t uncompressed { 0U }; ///< Uncompressed size in bytes
    std::uint16_t compressed { 0U }; ///< Compressed size in bytes
    std::uint32_t offset { 0U }; ///< 32-bit byte offset inside companion .SAR
};

/// @brief Parser and reader for Firehawk (Thexder 2) RESx.SAR and RESx.IX archives.
class SarArchiveReader {
public:
    SarArchiveReader() noexcept = default;
    ~SarArchiveReader() noexcept;

    SarArchiveReader(const SarArchiveReader&) = delete;
    SarArchiveReader& operator=(const SarArchiveReader&) = delete;

    SarArchiveReader(SarArchiveReader&& other) noexcept;
    SarArchiveReader& operator=(SarArchiveReader&& other) noexcept;

    /// @brief Opens an archive catalog (.IX) and companion data file (.SAR).
    /// @param ixPath Path to .IX catalog file.
    /// @param sarPath Path to .SAR archive file.
    /// @return True if successfully parsed.
    [[nodiscard]] bool open(const std::string& ixPath, const std::string& sarPath);

    /// @brief Closes open file streams and clears entries.
    void close() noexcept;

    /// @brief Checks if an archive is currently open.
    [[nodiscard]] bool isOpen() const noexcept;

    /// @brief Returns the total number of catalogued entries.
    [[nodiscard]] std::size_t entryCount() const noexcept;

    /// @brief Checks whether an entry exists by token name.
    /// @param token Asset token (e.g. "KM01", "CG01", "KA00").
    [[nodiscard]] bool hasEntry(std::string_view token) const noexcept;

    /// @brief Finds metadata for a specific entry.
    /// @param token Asset token.
    /// @return Pointer to entry metadata, or nullptr if not found.
    [[nodiscard]] const SarEntry* findEntry(std::string_view token) const noexcept;

    /// @brief Extracts raw data of an entry into a vector buffer.
    /// @param token Asset token.
    /// @param outData Destination vector.
    /// @return True if extracted successfully.
    [[nodiscard]] bool extractEntry(std::string_view token, std::vector<std::uint8_t>& outData) const;

    /// @brief Reads raw entry data into a caller-supplied buffer with zero heap allocation.
    /// @param token Asset token.
    /// @param outBuf Pointer to destination buffer.
    /// @param bufSize Size of destination buffer.
    /// @param bytesRead Output receiving actual bytes transferred.
    /// @return True if read successfully.
    [[nodiscard]] bool readEntryRaw(
        std::string_view token, std::uint8_t* outBuf, std::size_t bufSize, std::size_t& bytesRead) const noexcept;

    /// @brief Extracts and decompresses an entry into a vector buffer.
    /// @param token Asset token.
    /// @param outData Destination vector receiving uncompressed bytes.
    /// @return True if decompressed successfully.
    [[nodiscard]] bool decompressEntry(std::string_view token, std::vector<std::uint8_t>& outData) const;

    /// @brief Reads and decompresses entry into a caller-supplied buffer with zero heap allocation.
    /// @param token Asset token.
    /// @param outBuf Pointer to destination buffer.
    /// @param bufSize Maximum capacity of destination buffer.
    /// @param bytesWritten Output receiving actual decompressed byte count.
    /// @return True if decompressed successfully.
    [[nodiscard]] bool readEntryDecompressed(
        std::string_view token, std::uint8_t* outBuf, std::size_t bufSize, std::size_t& bytesWritten) const noexcept;

private:
    std::string sarPath_ { };
    std::vector<SarEntry> entries_ { };
};

} // namespace SdlAdapter
