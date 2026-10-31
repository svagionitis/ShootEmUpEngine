#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "TileMap.h"

namespace SdlAdapter {

/// @class EgaMapLoader
/// @brief Decodes 1987 Game Arts Thexder 1 column-indexed labyrinth map assets.
/// @details Reconstructs contiguous 2D tile matrices from MAPxx.BIN and EGAPTRxx.BIN.
class EgaMapLoader final {
public:
    static constexpr std::size_t kMapWidth { 320U };
    static constexpr std::size_t kMapHeight { 32U };
    static constexpr std::size_t kMacroBlockCols { 32U };
    static constexpr std::size_t kMacroBlockCount { 10U };
    static constexpr std::size_t kPtrTableWords { 1024U };
    static constexpr std::size_t kPtrTableBytes { 2048U };
    static constexpr std::size_t kExpectedMinMapBytes { 5792U };

    /// @brief Decodes raw pointer table and map column buffers into target TileMap.
    /// @param[in] ptrData Pointer to 2,048-byte EGAPTRxx.BIN buffer.
    /// @param[in] ptrSize Length of pointer table buffer in bytes.
    /// @param[in] mapData Pointer to raw MAPxx.BIN column data buffer.
    /// @param[in] mapSize Length of map data buffer in bytes.
    /// @param[out] outMap Destination TileMap to receive decoded tile types and obstacles.
    /// @return true if successfully decoded; false if inputs invalid or corrupted.
    [[nodiscard]] static bool decodeMap(const std::uint8_t* ptrData, std::size_t ptrSize, const std::uint8_t* mapData,
        std::size_t mapSize, ShooterCore::TileMap& outMap) noexcept;

    /// @brief Loads companion pointer and map binary files from disk into target TileMap.
    /// @param[in] ptrPath Filesystem path to EGAPTRxx.BIN.
    /// @param[in] mapPath Filesystem path to MAPxx.BIN.
    /// @param[out] outMap Destination TileMap to populate.
    /// @return true if files were read and decoded cleanly; false otherwise.
    [[nodiscard]] static bool loadMap(
        const std::string& ptrPath, const std::string& mapPath, ShooterCore::TileMap& outMap) noexcept;

    /// @brief Translates an 8-bit DOS tile byte into engine TileType and attributes.
    /// @param[in] rawByte Raw byte from MAPxx.BIN.
    /// @param[out] tileIndex 6-bit graphic tile index in EGABITxx.BIN (0..63).
    /// @return Engine TileType classification.
    [[nodiscard]] static constexpr ShooterCore::TileType parseTileByte(
        std::uint8_t rawByte, std::uint8_t& tileIndex) noexcept
    {
        tileIndex = static_cast<std::uint8_t>(rawByte & 0x3FU);
        const std::uint8_t attr { static_cast<std::uint8_t>(rawByte & 0xC0U) };

        switch (attr) {
        case 0x40U:
            return ShooterCore::TileType::DestructibleWall;
        case 0x80U:
            return ShooterCore::TileType::SecretWall;
        case 0xC0U:
            return ShooterCore::TileType::Hazard;
        default:
            return (tileIndex == 0U) ? ShooterCore::TileType::Empty : ShooterCore::TileType::SolidWall;
        }
    }
};

} // namespace SdlAdapter
