#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "RenderCommandQueue.h"
#include "Types.h"

namespace ShooterCore {

class SecuritySystem;

/// @class MinimapRadar
/// @brief Compact, zero-heap tactical radar managing fog-of-war and target telemetry.
class MinimapRadar final {
public:
    static constexpr std::size_t kChunkCols { 32U };
    static constexpr std::size_t kChunkRows { 16U };
    static constexpr float kChunkPixelExtent { 128.0f }; // 8 tiles * 16px

    MinimapRadar() noexcept = default;

    /// @brief Update fog-of-war discovery around player coordinates.
    void updateDiscovery(WorldPos playerPos) noexcept;

    /// @brief Query whether a specific chunk has been traversed.
    [[nodiscard]] bool isDiscovered(std::size_t col, std::size_t row) const noexcept;

    /// @brief Mark a specific chunk as discovered.
    void setDiscovered(std::size_t col, std::size_t row) noexcept;

    /// @brief Emit hardware-neutral draw commands for HUD radar overlay.
    void emitCommands(RenderCommandQueue& renderQueue, WorldPos playerPos, const SecuritySystem& security,
        WorldPos bossPos, bool bossActive) const noexcept;

private:
    // 512 bits = 8 * 64-bit integers = 64 bytes total
    std::array<std::uint64_t, 8U> discoveryBitmask_ { };
};

} // namespace ShooterCore
