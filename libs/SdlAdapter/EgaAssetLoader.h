#pragma once

#include <cstddef>
#include <cstdint>

#include <array>
#include <string>

namespace SdlAdapter {

/// @brief Authentic IBM PC EGA 16-color palette (RGBA32 format).
constexpr std::array<std::uint32_t, 16> kEgaPaletteRgba {
    0xFF000000U, // 0: Black
    0xFFAA0000U, // 1: Blue (0x0000AA)
    0xFF00AA00U, // 2: Green (0x00AA00)
    0xFFAAAA00U, // 3: Cyan (0x00AAAA)
    0xFF0000AAU, // 4: Red (0xAA0000)
    0xFFAA00AAU, // 5: Magenta (0xAA00AA)
    0xFF0055AAU, // 6: Brown (0xAA5500)
    0xFFAAAAAAU, // 7: Light Gray (0xAAAAAA)
    0xFF555555U, // 8: Dark Gray (0x555555)
    0xFFFF5555U, // 9: Bright Blue (0x5555FF)
    0xFF55FF55U, // 10: Bright Green (0x55FF55)
    0xFFFFFF55U, // 11: Bright Cyan (0x55FFFF)
    0xFF5555FFU, // 12: Bright Red (0xFF5555)
    0xFFFF55FFU, // 13: Bright Magenta (0xFF55FF)
    0xFF55FFFFU, // 14: Yellow (0xFFFF55)
    0xFFFFFFFFU // 15: White (0xFFFFFF)
};

/// @brief Decoder for 1980s Game Arts / Sierra 4-plane EGA binary assets.
class EgaAssetLoader {
public:
    static constexpr std::size_t kTileWidth { 16U };
    static constexpr std::size_t kTileHeight { 16U };
    static constexpr std::size_t kTilePlaneBytes { 32U };
    static constexpr std::size_t kTileTotalBytes { 128U };
    static constexpr std::size_t kBankPlaneSize { 2048U };
    static constexpr std::size_t kBankTotalSize { 8192U };
    static constexpr std::size_t kTilesPerBank { 64U };

    static constexpr std::size_t kTitleWidth { 320U };
    static constexpr std::size_t kTitleHeight { 196U };
    static constexpr std::size_t kTitleTotalSize { 31360U };

    /// @brief Decodes 64 16x16 4-plane EGA tiles into 32-bit RGBA pixel buffer.
    /// @param data Pointer to raw 8192-byte bank buffer.
    /// @param size Total length of buffer in bytes.
    /// @param outRgba Destination buffer receiving 16x16x64 RGBA pixels.
    /// @param maxTiles Maximum tiles to decode.
    /// @return Number of tiles successfully decoded.
    [[nodiscard]] static std::size_t decodeTiles(
        const std::uint8_t* data, std::size_t size, std::uint32_t* outRgba, std::size_t maxTiles) noexcept;

    /// @brief Loads and decodes an EGABITxx.BIN file from disk.
    /// @param filePath Path to binary file on disk.
    /// @param outRgba Destination buffer receiving RGBA pixels.
    /// @param maxTiles Maximum tiles to decode.
    /// @return Number of tiles successfully decoded.
    [[nodiscard]] static std::size_t loadTiles(
        const std::string& filePath, std::uint32_t* outRgba, std::size_t maxTiles);

    /// @brief Decodes a 320x196 4-plane EGA title screen (EGATITLE.BIN).
    /// @param data Pointer to raw 31360-byte title buffer.
    /// @param size Buffer size in bytes.
    /// @param outRgba Destination buffer (at least 320x196 uint32 pixels).
    /// @return True if successfully decoded.
    [[nodiscard]] static bool decodeTitle(const std::uint8_t* data, std::size_t size, std::uint32_t* outRgba) noexcept;

    /// @brief Loads and decodes EGATITLE.BIN from disk.
    /// @param filePath Path to binary file on disk.
    /// @param outRgba Destination buffer.
    /// @return True if successfully loaded and decoded.
    [[nodiscard]] static bool loadTitle(const std::string& filePath, std::uint32_t* outRgba);
};

} // namespace SdlAdapter
