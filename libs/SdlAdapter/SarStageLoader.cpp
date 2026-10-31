#include "SarStageLoader.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

#include "SarArchiveReader.h"
#include "TileMap.h"
#include "Types.h"

namespace SdlAdapter {

void SarStageLoader::getArchivePaths(std::uint8_t stageNum, std::string& outIx, std::string& outSar)
{
    const std::uint8_t idx { archiveIndexForStage(stageNum) };
    if (idx == 1U) {
        outIx = "assets/firehawk/RES1.IX";
        outSar = "assets/firehawk/RES1.SAR";
    } else if (idx == 2U) {
        outIx = "assets/firehawk/RES2.IX";
        outSar = "assets/firehawk/RES2.SAR";
    } else {
        outIx = "assets/firehawk/RES3.IX";
        outSar = "assets/firehawk/RES3.SAR";
    }
}

void SarStageLoader::formatStageToken(std::uint8_t stageNum, std::uint8_t zoneNum, char* outToken) noexcept
{
    if (outToken == nullptr) {
        return;
    }
    outToken[0] = 'K';
    outToken[1] = 'M';
    outToken[2] = static_cast<char>('0' + (stageNum % 10U));
    outToken[3] = static_cast<char>('0' + (zoneNum % 10U));
    outToken[4] = '\0';
}

bool SarStageLoader::loadStageZone(
    const SarArchiveReader& reader, std::uint8_t stageNum, std::uint8_t zoneNum, ShooterCore::TileMap& outMap)
{
    char token[5] { 0 };
    formatStageToken(stageNum, zoneNum, token);

    std::vector<std::uint8_t> mapBytes;
    if (!reader.decompressEntry(token, mapBytes) || mapBytes.empty()) {
        return false;
    }

    const std::uint32_t mapCols { outMap.width() };
    const std::uint32_t mapRows { outMap.height() };

    // Clear map to empty
    for (std::uint32_t r { 0U }; r < mapRows; ++r) {
        for (std::uint32_t c { 0U }; c < mapCols; ++c) {
            outMap.setTile(ShooterCore::TileCoord { c, r }, ShooterCore::TileType::Empty);
        }
    }

    // Populate interior labyrinth tiles from decoded KMxx stream
    std::size_t byteIdx { 0U };
    for (std::uint32_t c { 0U }; c < mapCols && byteIdx < mapBytes.size(); ++c) {
        for (std::uint32_t r { 0U }; r < mapRows && byteIdx < mapBytes.size(); ++r) {
            const std::uint8_t rawByte { mapBytes[byteIdx++] };
            const std::uint8_t tileAttr { static_cast<std::uint8_t>(rawByte & 0xC0U) };
            const std::uint8_t tileId { static_cast<std::uint8_t>(rawByte & 0x3FU) };

            ShooterCore::TileType type { ShooterCore::TileType::Empty };
            if (tileAttr == 0x80U || (tileAttr == 0x00U && tileId > 0U && tileId <= 24U)) {
                type = ShooterCore::TileType::SolidWall;
            } else if (tileAttr == 0x40U) {
                type = ShooterCore::TileType::DestructibleWall;
            } else if (tileAttr == 0xC0U) {
                type = ShooterCore::TileType::Hazard;
            }

            outMap.setTile(ShooterCore::TileCoord { c, r }, type);
        }
    }

    // Enforce top and bottom boundaries for playable corridors
    for (std::uint32_t c { 0U }; c < mapCols; ++c) {
        outMap.setTile(ShooterCore::TileCoord { c, 0U }, ShooterCore::TileType::SolidWall);
        outMap.setTile(ShooterCore::TileCoord { c, mapRows - 1U }, ShooterCore::TileType::SolidWall);
    }
    for (std::uint32_t r { 0U }; r < mapRows; ++r) {
        outMap.setTile(ShooterCore::TileCoord { 0U, r }, ShooterCore::TileType::SolidWall);
        outMap.setTile(ShooterCore::TileCoord { mapCols - 1U, r }, ShooterCore::TileType::SolidWall);
    }

    return true;
}

} // namespace SdlAdapter
