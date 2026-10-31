#include "EgaMapLoader.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#include <glog/logging.h>

#include "TileMap.h"
#include "Types.h"

namespace SdlAdapter {

bool EgaMapLoader::decodeMap(const std::uint8_t* ptrData, std::size_t ptrSize, const std::uint8_t* mapData,
    std::size_t mapSize, ShooterCore::TileMap& outMap) noexcept
{
    if (ptrData == nullptr || mapData == nullptr) {
        LOG(ERROR) << "EgaMapLoader::decodeMap: Null buffer pointers provided";
        return false;
    }

    if (ptrSize < kPtrTableBytes) {
        LOG(ERROR) << "EgaMapLoader::decodeMap: Insufficient pointer table size: " << ptrSize << " bytes (expected "
                   << kPtrTableBytes << ")";
        return false;
    }

    if (mapSize < kMapHeight) {
        LOG(ERROR) << "EgaMapLoader::decodeMap: Insufficient map data size: " << mapSize << " bytes";
        return false;
    }

    const auto targetCols { static_cast<std::size_t>(outMap.width()) };
    const auto targetRows { static_cast<std::size_t>(outMap.height()) };
    const std::size_t maxCols { std::min(targetCols, kMapWidth) };
    const std::size_t maxRows { std::min(targetRows, kMapHeight) };

    for (std::size_t col { 0U }; col < maxCols; ++col) {
        const std::size_t b { col / kMacroBlockCols };
        const std::size_t colInBlock { col % kMacroBlockCols };
        const std::size_t r { colInBlock / 4U };
        const std::size_t c { colInBlock % 4U };

        // 8 table rows of 128 words (256 bytes per row)
        const std::size_t wordIndex { (r * 128U) + (b * 4U) + c };
        const std::size_t byteOffset { wordIndex * 2U };

        if (byteOffset + 1U >= ptrSize) {
            continue;
        }

        // 16-bit little-endian column offset into mapData
        const auto lowByte { static_cast<std::uint16_t>(ptrData[byteOffset]) };
        const auto highByte { static_cast<std::uint16_t>(ptrData[byteOffset + 1U]) };
        const auto colOffset { static_cast<std::size_t>(lowByte | (static_cast<std::uint16_t>(highByte << 8U))) };

        for (std::size_t row { 0U }; row < maxRows; ++row) {
            const ShooterCore::TileCoord coord { static_cast<std::uint32_t>(col), static_cast<std::uint32_t>(row) };

            if ((colOffset + row) < mapSize) {
                const std::uint8_t rawTile { mapData[colOffset + row] };
                std::uint8_t tileIdx { 0U };
                const ShooterCore::TileType tType { parseTileByte(rawTile, tileIdx) };
                outMap.setTile(coord, tType);
            } else {
                outMap.setTile(coord, ShooterCore::TileType::Empty);
            }
        }
    }

    LOG(INFO) << "EgaMapLoader::decodeMap: Successfully decoded " << maxCols << "x" << maxRows << " labyrinth grid";
    return true;
}

bool EgaMapLoader::loadMap(
    const std::string& ptrPath, const std::string& mapPath, ShooterCore::TileMap& outMap) noexcept
{
    std::ifstream ptrFile(ptrPath, std::ios::binary);
    if (!ptrFile.is_open()) {
        LOG(WARNING) << "EgaMapLoader::loadMap: Failed to open pointer table: " << ptrPath;
        return false;
    }

    std::vector<std::uint8_t> ptrBuffer((std::istreambuf_iterator<char>(ptrFile)), std::istreambuf_iterator<char>());
    ptrFile.close();

    std::ifstream mapFile(mapPath, std::ios::binary);
    if (!mapFile.is_open()) {
        LOG(WARNING) << "EgaMapLoader::loadMap: Failed to open map data: " << mapPath;
        return false;
    }

    std::vector<std::uint8_t> mapBuffer((std::istreambuf_iterator<char>(mapFile)), std::istreambuf_iterator<char>());
    mapFile.close();

    return decodeMap(ptrBuffer.data(), ptrBuffer.size(), mapBuffer.data(), mapBuffer.size(), outMap);
}

} // namespace SdlAdapter
