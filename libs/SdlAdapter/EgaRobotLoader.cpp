#include "EgaRobotLoader.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

namespace SdlAdapter {

std::size_t EgaRobotLoader::decodeSprites(const std::uint8_t* data, std::size_t size, std::uint32_t* outRgba,
    std::size_t maxFrames, std::uint32_t fgColor, std::uint32_t bgColor) noexcept
{
    if (data == nullptr || outRgba == nullptr || size < kBytesPerFrame || maxFrames == 0U) {
        return 0U;
    }

    const std::size_t availableFrames { size / kBytesPerFrame };
    const std::size_t frameCount { std::min({ maxFrames, availableFrames, kTotalFrames }) };

    for (std::size_t fIdx { 0U }; fIdx < frameCount; ++fIdx) {
        const std::size_t frameOffset { fIdx * kBytesPerFrame };
        std::uint32_t* frameDest { outRgba + fIdx * (kFrameWidth * kFrameHeight) };

        for (std::size_t y { 0U }; y < kFrameHeight; ++y) {
            const std::size_t rowOffset { frameOffset + y * kBytesPerRow };

            for (std::size_t bx { 0U }; bx < kBytesPerRow; ++bx) {
                const std::uint8_t byteVal { data[rowOffset + bx] };

                for (std::size_t bit { 0U }; bit < 8U; ++bit) {
                    const auto shift { static_cast<std::uint8_t>(7U - bit) };
                    const bool isLit { ((byteVal >> shift) & 1U) != 0U };

                    const std::size_t pixelX { bx * 8U + bit };
                    frameDest[y * kFrameWidth + pixelX] = isLit ? fgColor : bgColor;
                }
            }
        }
    }

    return frameCount;
}

std::size_t EgaRobotLoader::loadSprites(const std::string& filePath, std::uint32_t* outRgba, std::size_t maxFrames,
    std::uint32_t fgColor, std::uint32_t bgColor)
{
    if (outRgba == nullptr || maxFrames == 0U) {
        return 0U;
    }

    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        file.open("../../../" + filePath, std::ios::binary);
    }
    if (!file.is_open()) {
        file.open("../../" + filePath, std::ios::binary);
    }
    if (!file.is_open()) {
        file.open("../" + filePath, std::ios::binary);
    }
    if (!file.is_open()) {
        return 0U;
    }

    std::vector<std::uint8_t> buffer(kRobotTotalSize);
    file.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()));
    const auto bytesRead { static_cast<std::size_t>(file.gcount()) };
    if (bytesRead < kBytesPerFrame) {
        return 0U;
    }

    return decodeSprites(buffer.data(), bytesRead, outRgba, maxFrames, fgColor, bgColor);
}

} // namespace SdlAdapter
