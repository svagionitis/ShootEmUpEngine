#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "NestManager.h"
#include "Types.h"

namespace SdlAdapter {

/// @struct EgaBugRecord
/// @brief 12-byte entity spawn descriptor decoded from BUGDBxx.BIN.
struct EgaBugRecord final {
    std::uint16_t worldY { 0U }; ///< Vertical spawn origin in world space
    std::uint16_t worldX { 0U }; ///< Horizontal spawn origin in world space
    std::uint16_t spawnInterval { 0U }; ///< Baseline spawn cadence
    std::uint16_t initialHealth { 0U }; ///< Durability points
    std::uint16_t dropType { 0U }; ///< Loot table index
    std::uint16_t reserved { 0U }; ///< Reserved padding
};

/// @class EgaBugLoader
/// @brief Parses 1987 Game Arts BUGDBxx.BIN binary alien generator databases.
/// @details Instantiates authentic bio-nests and enemy spawners in ShooterCore::NestManager.
class EgaBugLoader final {
public:
    static constexpr std::size_t kExpectedFileSize { 3404U };
    static constexpr std::size_t kRecordSize { 74U };
    static constexpr std::size_t kTotalRecords { 46U };
    static constexpr std::size_t kEntityStrideBytes { 12U };
    static constexpr std::size_t kDataStartRecord { 4U };
    static constexpr std::size_t kDataStartOffset { (kDataStartRecord * kRecordSize) + 24U };

    /// @brief Decodes raw BUGDBxx.BIN buffer into target NestManager.
    /// @param[in] data Pointer to raw binary buffer.
    /// @param[in] size Size of binary buffer in bytes.
    /// @param[out] outNests Target NestManager to receive decoded bio-nests.
    /// @return Count of bio-nests successfully registered.
    [[nodiscard]] static std::size_t decodeBugDb(
        const std::uint8_t* data, std::size_t size, ShooterCore::NestManager& outNests) noexcept;

    /// @brief Loads and parses a BUGDBxx.BIN file from disk into target NestManager.
    /// @param[in] filePath Path to BUGDBxx.BIN on disk.
    /// @param[out] outNests Target NestManager to receive decoded bio-nests.
    /// @return Count of bio-nests successfully registered.
    [[nodiscard]] static std::size_t loadBugDb(
        const std::string& filePath, ShooterCore::NestManager& outNests) noexcept;
};

} // namespace SdlAdapter
