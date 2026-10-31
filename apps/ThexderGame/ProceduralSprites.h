#pragma once

#include <cstddef>
#include <cstdint>

#include "RenderCommandQueue.h"
#include "Types.h"

namespace ThexderGame {

/// @class ProceduralSprites
/// @brief Generates vector and pixel art primitives for title screens and nest structures.
/// @details Enables the application to run self-contained with zero external asset files.
class ProceduralSprites final {
public:
    ProceduralSprites() noexcept = default;

    /// @brief Render stylized retro title screen logo for Thexder into command queue.
    /// @param[out] queue Destination command queue.
    /// @param[in] x Screen X coordinate.
    /// @param[in] y Screen Y coordinate.
    static void drawTitleLogo(ShooterCore::RenderCommandQueue& queue, float x, float y) noexcept;

    /// @brief Render high-impact Game Over banner into command queue.
    /// @param[out] queue Destination command queue.
    /// @param[in] x Screen X coordinate.
    /// @param[in] y Screen Y coordinate.
    static void drawGameOver(ShooterCore::RenderCommandQueue& queue, float x, float y) noexcept;

    /// @brief Render Level 1 Complete victory banner into command queue.
    /// @param[out] queue Destination command queue.
    /// @param[in] x Screen X coordinate.
    /// @param[in] y Screen Y coordinate.
    static void drawVictory(ShooterCore::RenderCommandQueue& queue, float x, float y) noexcept;

    /// @brief Render stationary generator nest structure at cavern location.
    /// @param[out] queue Destination command queue.
    /// @param[in] screenPos Screen-space coordinate of nest anchor.
    /// @param[in] destroyed Whether nest core is collapsed.
    static void drawNestCore(
        ShooterCore::RenderCommandQueue& queue, ShooterCore::WorldPos screenPos, bool destroyed) noexcept;
};

} // namespace ThexderGame
