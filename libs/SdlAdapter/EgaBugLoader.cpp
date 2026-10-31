#include "EgaBugLoader.h"

#include <cstddef>
#include <cstdint>

#include <fstream>
#include <string>
#include <vector>

#include <glog/logging.h>

#include "NestManager.h"
#include "Types.h"

namespace SdlAdapter {

std::size_t EgaBugLoader::decodeBugDb(
    const std::uint8_t* data, std::size_t size, ShooterCore::NestManager& outNests) noexcept
{
    if (data == nullptr || size < kDataStartOffset + kEntityStrideBytes) {
        LOG(ERROR) << "EgaBugLoader::decodeBugDb: Invalid or truncated buffer (size: " << size << ")";
        return 0U;
    }

    std::size_t nestsAdded { 0U };
    std::size_t offset { kDataStartOffset };

    while (offset + kEntityStrideBytes <= size && outNests.nestCount() < ShooterCore::NestManager::kMaxNestSlots) {
        // Read 16-bit little-endian worldY and worldX
        const auto yLow { static_cast<std::uint16_t>(data[offset]) };
        const auto yHigh { static_cast<std::uint16_t>(data[offset + 1U]) };
        const auto worldY { static_cast<std::uint16_t>(yLow | (static_cast<std::uint16_t>(yHigh << 8U))) };

        const auto xLow { static_cast<std::uint16_t>(data[offset + 2U]) };
        const auto xHigh { static_cast<std::uint16_t>(data[offset + 3U]) };
        const auto worldX { static_cast<std::uint16_t>(xLow | (static_cast<std::uint16_t>(xHigh << 8U))) };

        // Authentic coordinate validation: X in [16..5120], Y in [16..512]
        if (worldX >= 16U && worldX <= 5120U && worldY >= 16U && worldY <= 512U) {
            const ShooterCore::WorldPos nestPos { static_cast<float>(worldX), static_cast<float>(worldY) };

            const bool ok { outNests.addNest(nestPos, ShooterCore::NestManager::kDefaultNestHealth, 3.5f) };
            if (ok) {
                ++nestsAdded;
            }
        }

        offset += kEntityStrideBytes;
    }

    LOG(INFO) << "EgaBugLoader::decodeBugDb: Ingested " << nestsAdded << " authentic alien nests";
    return nestsAdded;
}

std::size_t EgaBugLoader::loadBugDb(const std::string& filePath, ShooterCore::NestManager& outNests) noexcept
{
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
        LOG(WARNING) << "EgaBugLoader::loadBugDb: Failed to open: " << filePath;
        return 0U;
    }

    std::vector<std::uint8_t> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    return decodeBugDb(buffer.data(), buffer.size(), outNests);
}

} // namespace SdlAdapter
