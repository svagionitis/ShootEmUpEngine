#pragma once

#include <cstddef>
#include <cstdint>

#include <string>

namespace SdlAdapter {

/// @brief Decoder for authentic 48x32 Thexder mecha frames (EGAROBOT.BIN).
class EgaRobotLoader {
public:
    static constexpr std::size_t kFrameWidth { 48U };
    static constexpr std::size_t kFrameHeight { 32U };
    static constexpr std::size_t kBytesPerRow { 6U };
    static constexpr std::size_t kBytesPerFrame { 192U }; // 6 bytes * 32 rows
    static constexpr std::size_t kTotalFrames { 88U };
    static constexpr std::size_t kRobotTotalSize { 16992U };

    /// @brief Decodes 88 48x32 mecha animation frames into 32-bit RGBA buffer.
    /// @param data Pointer to raw 16992-byte EGAROBOT.BIN buffer.
    /// @param size Buffer size in bytes.
    /// @param outRgba Destination buffer receiving 48x32xN RGBA pixels.
    /// @param maxFrames Maximum frames to decode.
    /// @param fgColor RGBA color for active foreground pixels.
    /// @param bgColor RGBA color for background pixels (default: transparent).
    /// @return Number of frames successfully decoded.
    [[nodiscard]] static std::size_t decodeSprites(const std::uint8_t* data, std::size_t size, std::uint32_t* outRgba,
        std::size_t maxFrames, std::uint32_t fgColor = 0xFFFF5555U, std::uint32_t bgColor = 0x00000000U) noexcept;

    /// @brief Loads and decodes an EGAROBOT.BIN file from disk.
    /// @param filePath Path to binary file on disk.
    /// @param outRgba Destination buffer receiving RGBA pixels.
    /// @param maxFrames Maximum frames to decode.
    /// @param fgColor RGBA color for active foreground pixels.
    /// @param bgColor RGBA color for background pixels.
    /// @return Number of frames successfully decoded.
    [[nodiscard]] static std::size_t loadSprites(const std::string& filePath, std::uint32_t* outRgba,
        std::size_t maxFrames, std::uint32_t fgColor = 0xFFFF5555U, std::uint32_t bgColor = 0x00000000U);
};

} // namespace SdlAdapter
