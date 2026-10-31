#include "EgaAssetLoader.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

namespace SdlAdapter {

std::size_t EgaAssetLoader::decodeTiles(
    const std::uint8_t* data, std::size_t size, std::uint32_t* outRgba, std::size_t maxTiles) noexcept
{
    if (data == nullptr || outRgba == nullptr || size < kBankTotalSize || maxTiles == 0U) {
        return 0U;
    }

    const std::size_t tileCount { std::min(maxTiles, kTilesPerBank) };

    const std::uint8_t* plane0 { data + 0U * kBankPlaneSize };
    const std::uint8_t* plane1 { data + 1U * kBankPlaneSize };
    const std::uint8_t* plane2 { data + 2U * kBankPlaneSize };
    const std::uint8_t* plane3 { data + 3U * kBankPlaneSize };

    for (std::size_t tileIdx { 0U }; tileIdx < tileCount; ++tileIdx) {
        const std::size_t tileBasePlaneOff { tileIdx * kTilePlaneBytes };
        std::uint32_t* tileDest { outRgba + tileIdx * (kTileWidth * kTileHeight) };

        for (std::size_t y { 0U }; y < kTileHeight; ++y) {
            const std::size_t rowOff { tileBasePlaneOff + y * 2U };

            for (std::size_t bx { 0U }; bx < 2U; ++bx) {
                const std::uint8_t b0 { plane0[rowOff + bx] };
                const std::uint8_t b1 { plane1[rowOff + bx] };
                const std::uint8_t b2 { plane2[rowOff + bx] };
                const std::uint8_t b3 { plane3[rowOff + bx] };

                for (std::size_t bit { 0U }; bit < 8U; ++bit) {
                    const auto shift { static_cast<std::uint8_t>(7U - bit) };
                    const auto bit0 { static_cast<std::uint8_t>((b0 >> shift) & 1U) };
                    const auto bit1 { static_cast<std::uint8_t>((b1 >> shift) & 1U) };
                    const auto bit2 { static_cast<std::uint8_t>((b2 >> shift) & 1U) };
                    const auto bit3 { static_cast<std::uint8_t>((b3 >> shift) & 1U) };

                    const auto colIdx { static_cast<std::size_t>(bit0 | (bit1 << 1U) | (bit2 << 2U) | (bit3 << 3U)) };
                    const std::size_t pixelX { bx * 8U + bit };
                    tileDest[y * kTileWidth + pixelX] = kEgaPaletteRgba[colIdx];
                }
            }
        }
    }

    return tileCount;
}

std::size_t EgaAssetLoader::loadTiles(const std::string& filePath, std::uint32_t* outRgba, std::size_t maxTiles)
{
    if (outRgba == nullptr || maxTiles == 0U) {
        return 0U;
    }

    std::string actualPath { filePath };
    std::ifstream file(actualPath, std::ios::binary);
    if (!file.is_open()) {
        actualPath = "../../../" + filePath;
        file.open(actualPath, std::ios::binary);
    }
    if (!file.is_open()) {
        actualPath = "../../" + filePath;
        file.open(actualPath, std::ios::binary);
    }
    if (!file.is_open()) {
        actualPath = "../" + filePath;
        file.open(actualPath, std::ios::binary);
    }
    if (!file.is_open()) {
        return 0U;
    }

    std::vector<std::uint8_t> buffer(kBankTotalSize);
    file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
    const auto bytesRead { static_cast<std::size_t>(file.gcount()) };
    if (bytesRead < kBankTotalSize) {
        return 0U;
    }

    return decodeTiles(buffer.data(), buffer.size(), outRgba, maxTiles);
}

bool EgaAssetLoader::decodeTitle(const std::uint8_t* data, std::size_t size, std::uint32_t* outRgba) noexcept
{
    if (data == nullptr || outRgba == nullptr || size < kTitleTotalSize) {
        return false;
    }

    constexpr std::size_t planeSize { kTitleTotalSize / 4U }; // 7840 bytes per plane
    constexpr std::size_t bytesPerRow { kTitleWidth / 8U }; // 40 bytes per row

    const std::uint8_t* plane0 { data + 0U * planeSize };
    const std::uint8_t* plane1 { data + 1U * planeSize };
    const std::uint8_t* plane2 { data + 2U * planeSize };
    const std::uint8_t* plane3 { data + 3U * planeSize };

    for (std::size_t y { 0U }; y < kTitleHeight; ++y) {
        for (std::size_t bx { 0U }; bx < bytesPerRow; ++bx) {
            const std::size_t offset { y * bytesPerRow + bx };
            const std::uint8_t b0 { plane0[offset] };
            const std::uint8_t b1 { plane1[offset] };
            const std::uint8_t b2 { plane2[offset] };
            const std::uint8_t b3 { plane3[offset] };

            for (std::size_t bit { 0U }; bit < 8U; ++bit) {
                const auto shift { static_cast<std::uint8_t>(7U - bit) };
                const auto bit0 { static_cast<std::uint8_t>((b0 >> shift) & 1U) };
                const auto bit1 { static_cast<std::uint8_t>((b1 >> shift) & 1U) };
                const auto bit2 { static_cast<std::uint8_t>((b2 >> shift) & 1U) };
                const auto bit3 { static_cast<std::uint8_t>((b3 >> shift) & 1U) };

                const auto colIdx { static_cast<std::size_t>(bit0 | (bit1 << 1U) | (bit2 << 2U) | (bit3 << 3U)) };
                const std::size_t pixelX { bx * 8U + bit };
                outRgba[y * kTitleWidth + pixelX] = kEgaPaletteRgba[colIdx];
            }
        }
    }

    return true;
}

bool EgaAssetLoader::loadTitle(const std::string& filePath, std::uint32_t* outRgba)
{
    if (outRgba == nullptr) {
        return false;
    }

    std::string actualPath { filePath };
    std::ifstream file(actualPath, std::ios::binary);
    if (!file.is_open()) {
        actualPath = "../../../" + filePath;
        file.open(actualPath, std::ios::binary);
    }
    if (!file.is_open()) {
        actualPath = "../../" + filePath;
        file.open(actualPath, std::ios::binary);
    }
    if (!file.is_open()) {
        actualPath = "../" + filePath;
        file.open(actualPath, std::ios::binary);
    }
    if (!file.is_open()) {
        return false;
    }

    std::vector<std::uint8_t> buffer(kTitleTotalSize);
    file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
    const auto bytesRead { static_cast<std::size_t>(file.gcount()) };
    if (bytesRead < kTitleTotalSize) {
        return false;
    }

    return decodeTitle(buffer.data(), buffer.size(), outRgba);
}

} // namespace SdlAdapter
