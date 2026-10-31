#pragma once

#include <cstddef>
#include <cstdint>

namespace SdlAdapter {

/// @class SarDecompressor
/// @brief Zero-heap LZW decompressor for Firehawk (Thexder 2) SAR compressed resources.
/// @details Implements Game Arts 9..12-bit variable-length LZW decompression with zero runtime dynamic allocations.
class SarDecompressor final {
public:
    static constexpr std::uint16_t kClearCode { 256U };
    static constexpr std::uint16_t kEofCode { 257U };
    static constexpr std::uint16_t kFirstCode { 258U };
    static constexpr std::size_t kMaxDictEntries { 4096U };

    SarDecompressor() noexcept = default;

    /// @brief Decompresses an LZW-encoded SAR payload into a pre-allocated output buffer.
    /// @param[in] inData Pointer to compressed byte stream.
    /// @param[in] inLen Length of compressed data in bytes.
    /// @param[out] outBuf Destination buffer for uncompressed bytes.
    /// @param[in] outMax Maximum capacity of destination buffer.
    /// @param[out] outWritten Number of bytes successfully decompressed.
    /// @return true if decompression succeeded and produced valid output; false on corruption/overflow.
    [[nodiscard]] static bool decompress(const std::uint8_t* inData, std::size_t inLen, std::uint8_t* outBuf,
        std::size_t outMax, std::size_t& outWritten) noexcept;

private:
    struct DictEntry final {
        std::uint16_t prefix { 0U };
        std::uint8_t suffix { 0U };
    };
};

} // namespace SdlAdapter
