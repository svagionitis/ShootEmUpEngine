#include "MinimapRadar.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>

#include "RenderCommandQueue.h"
#include "SecuritySystem.h"
#include "Types.h"

namespace ShooterCore {

void MinimapRadar::setDiscovered(std::size_t col, std::size_t row) noexcept
{
    if (col >= kChunkCols || row >= kChunkRows) {
        return;
    }

    const std::size_t idx { row * kChunkCols + col };
    const std::size_t wordIdx { idx / 64U };
    const std::size_t bitIdx { idx % 64U };

    discoveryBitmask_[wordIdx] |= (1ULL << bitIdx);
}

bool MinimapRadar::isDiscovered(std::size_t col, std::size_t row) const noexcept
{
    if (col >= kChunkCols || row >= kChunkRows) {
        return false;
    }

    const std::size_t idx { row * kChunkCols + col };
    const std::size_t wordIdx { idx / 64U };
    const std::size_t bitIdx { idx % 64U };

    return (discoveryBitmask_[wordIdx] & (1ULL << bitIdx)) != 0ULL;
}

void MinimapRadar::updateDiscovery(WorldPos playerPos) noexcept
{
    if (playerPos.x < 0.0f || playerPos.y < 0.0f) {
        return;
    }

    const auto centerCol { static_cast<std::int32_t>(playerPos.x / kChunkPixelExtent) };
    const auto centerRow { static_cast<std::int32_t>(playerPos.y / kChunkPixelExtent) };

    for (std::int32_t dr { -1 }; dr <= 1; ++dr) {
        for (std::int32_t dc { -1 }; dc <= 1; ++dc) {
            const std::int32_t r { centerRow + dr };
            const std::int32_t c { centerCol + dc };
            if (r >= 0 && r < static_cast<std::int32_t>(kChunkRows) && c >= 0
                && c < static_cast<std::int32_t>(kChunkCols)) {
                setDiscovered(static_cast<std::size_t>(c), static_cast<std::size_t>(r));
            }
        }
    }
}

void MinimapRadar::emitCommands(RenderCommandQueue& renderQueue, WorldPos playerPos, const SecuritySystem& security,
    WorldPos bossPos, bool bossActive) const noexcept
{
    constexpr float kRadarX { 250.0f };
    constexpr float kRadarY { 6.0f };
    constexpr float kRadarW { 64.0f };
    constexpr float kRadarH { 32.0f };
    constexpr float kWorldW { 4096.0f };
    constexpr float kWorldH { 2048.0f };

    // 1. Radar background panel
    renderQueue.pushBar(WorldPos { kRadarX, kRadarY }, kRadarW, kRadarH, 1.0f, ColorRgba { 10U, 24U, 36U, 210U });

    // 2. Discovered chunk fills (2x2 pixels per chunk)
    for (std::size_t r { 0U }; r < kChunkRows; ++r) {
        for (std::size_t c { 0U }; c < kChunkCols; ++c) {
            if (isDiscovered(c, r)) {
                const float cx { kRadarX + static_cast<float>(c) * 2.0f };
                const float cy { kRadarY + static_cast<float>(r) * 2.0f };
                renderQueue.pushBar(WorldPos { cx, cy }, 2.0f, 2.0f, 1.0f, ColorRgba { 30U, 70U, 110U, 180U });
            }
        }
    }

    // Helper lambda for projecting world position to radar coordinates
    const auto toRadarPos = [](WorldPos wPos) noexcept -> WorldPos {
        const float rx { kRadarX + std::clamp((wPos.x / kWorldW) * kRadarW, 0.0f, kRadarW - 2.0f) };
        const float ry { kRadarY + std::clamp((wPos.y / kWorldH) * kRadarH, 0.0f, kRadarH - 2.0f) };
        return WorldPos { rx, ry };
    };

    // 3. Generators
    const auto& gens = security.generators();
    for (std::size_t i { 0U }; i < SecuritySystem::kMaxGenerators; ++i) {
        if (gens[i].active) {
            const WorldPos rp { toRadarPos(gens[i].pos) };
            renderQueue.pushBar(rp, 2.0f, 2.0f, 1.0f, ColorRgba { 255U, 230U, 50U, 255U }); // Yellow
        }
    }

    // 4. Locked Barriers
    const auto& barriers = security.barriers();
    for (std::size_t b { 0U }; b < security.barrierCount(); ++b) {
        if (barriers[b].state != SecurityGateState::Open) {
            const WorldPos rp { toRadarPos(barriers[b].pos) };
            renderQueue.pushBar(rp, 2.0f, 2.0f, 1.0f, ColorRgba { 255U, 50U, 50U, 255U }); // Red
        }
    }

    // 5. Boss Blip
    if (bossActive) {
        const WorldPos rp { toRadarPos(bossPos) };
        renderQueue.pushBar(rp, 3.0f, 3.0f, 1.0f, ColorRgba { 255U, 40U, 255U, 255U }); // Purple
    }

    // 6. Player Blip
    const WorldPos pRp { toRadarPos(playerPos) };
    renderQueue.pushBar(pRp, 2.0f, 2.0f, 1.0f, ColorRgba { 50U, 255U, 70U, 255U }); // Bright green
}

} // namespace ShooterCore
