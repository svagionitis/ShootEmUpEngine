#pragma once

#include <cstddef>
#include <cstdint>

#include <string>
#include <string_view>
#include <vector>

#include "SarArchiveReader.h"
#include "TileMap.h"

namespace SdlAdapter {

/// @class SarStageLoader
/// @brief Ingests authentic Firehawk (Thexder 2) multi-zone stage layouts (KMxx) and tiles (CTxx).
/// @details Streams stage maps directly from RES1.SAR, RES2.SAR, and RES3.SAR archives.
class SarStageLoader final {
public:
    static constexpr std::uint32_t kDefaultZoneWidth { 128U };
    static constexpr std::uint32_t kDefaultZoneHeight { 32U };
    static constexpr float kTileSize { 16.0f };

    SarStageLoader() noexcept = default;

    /// @brief Determine which archive index (1..3) contains the given stage.
    /// @param[in] stageNum Stage number (1..9).
    /// @return Archive index (1 for RES1, 2 for RES2, 3 for RES3).
    [[nodiscard]] static constexpr std::uint8_t archiveIndexForStage(std::uint8_t stageNum) noexcept
    {
        if (stageNum <= 1U) {
            return 1U;
        }
        if (stageNum <= 5U) {
            return 2U;
        }
        return 3U;
    }

    /// @brief Retrieve filesystem paths for catalog and companion archive for a stage.
    /// @param[in] stageNum Stage number (1..9).
    /// @param[out] outIx Path to .IX catalog file.
    /// @param[out] outSar Path to .SAR archive file.
    static void getArchivePaths(std::uint8_t stageNum, std::string& outIx, std::string& outSar);

    /// @brief Generate the 4-character SAR token for a stage zone layout (e.g. "KM11", "KM24").
    /// @param[in] stageNum Stage number (1..9).
    /// @param[in] zoneNum Zone number (1..4).
    /// @param[out] outToken Destination buffer of at least 5 bytes.
    static void formatStageToken(std::uint8_t stageNum, std::uint8_t zoneNum, char* outToken) noexcept;

    /// @brief Load and parse a stage zone layout directly into a ShooterCore::TileMap.
    /// @param[in] reader Open SAR archive containing target stage assets.
    /// @param[in] stageNum Stage number (1..9).
    /// @param[in] zoneNum Zone number (1..4).
    /// @param[out] outMap Destination TileMap to populate.
    /// @return true if successfully parsed and populated.
    [[nodiscard]] static bool loadStageZone(
        const SarArchiveReader& reader, std::uint8_t stageNum, std::uint8_t zoneNum, ShooterCore::TileMap& outMap);
};

} // namespace SdlAdapter
