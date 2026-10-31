#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "RenderCommandQueue.h"
#include "Types.h"

namespace ThexderGame {

/// @class HudOverlay
/// @brief Formats and renders authentic Thexder bottom console status metrics, gauges, and retro text.
/// @details Emits render commands directly to RenderCommandQueue with zero heap allocations (CERT ERR55-CPP).
class HudOverlay final {
public:
    static constexpr float kConsoleStartY { 202.0f };
    static constexpr float kConsoleHeight { 38.0f };

    HudOverlay() noexcept = default;

    /// @brief Emit all HUD graphics and text strings into the render command queue.
    /// @param[out] queue Render command queue to receive draw commands.
    /// @param[in] energy Current player energy reserve.
    /// @param[in] shield Current barrier percentage.
    /// @param[in] score Accrued game score points.
    /// @param[in] hiScore Persistent session high score.
    /// @param[in] mode Active mechanical transformation mode.
    /// @param[in] warningTimer Warning flash cycle accumulator.
    /// @param[in] secType Active secondary weapon.
    /// @param[in] secAmmo Secondary ordnance inventory.
    /// @param[in] sectorName Operational sector title string.
    /// @param[in] lockWarning Whether missile lock annunciator is active.
    void renderHud(ShooterCore::RenderCommandQueue& queue, ShooterCore::EnergyPoints energy,
        ShooterCore::ShieldGauge shield, std::uint32_t score, std::uint32_t hiScore, ShooterCore::TransformMode mode,
        float warningTimer, ShooterCore::SecondaryWeaponType secType = ShooterCore::SecondaryWeaponType::None,
        std::uint8_t secAmmo = 0U, std::string_view sectorName = "SEC 01: CAVERN", bool lockWarning = false) noexcept;

    /// @brief Render discrete segmented LED ladder gauge with unlit cathode traces.
    /// @param[out] queue Render command queue.
    /// @param[in] startX Starting screen X coordinate.
    /// @param[in] startY Starting screen Y coordinate.
    /// @param[in] totalSegments Total discrete physical LED segments.
    /// @param[in] litSegments Count of currently illuminated segments.
    /// @param[in] litColor RGBA tint for illuminated segments.
    /// @param[in] unlitColor RGBA tint for unlit cathode segments.
    static void drawSegmentedBar(ShooterCore::RenderCommandQueue& queue, float startX, float startY,
        std::uint8_t totalSegments, std::uint8_t litSegments, ShooterCore::ColorRgba litColor,
        ShooterCore::ColorRgba unlitColor) noexcept;

    /// @brief Rasterize plain ASCII string into command queue using 5x7 font.
    /// @param[out] queue Render command queue.
    /// @param[in] text Plain text string to draw (alphanumeric and symbols).
    /// @param[in] screenX Screen-space X coordinate.
    /// @param[in] screenY Screen-space Y coordinate.
    /// @param[in] scale Pixel multiplier (1 = 5x7 px, 2 = 10x14 px).
    /// @param[in] color Text foreground RGBA tint.
    static void drawText(ShooterCore::RenderCommandQueue& queue, std::string_view text, float screenX, float screenY,
        std::uint8_t scale, ShooterCore::ColorRgba color) noexcept;

    /// @brief Format an unsigned 32-bit integer into a zero-padded ASCII stack buffer.
    /// @param[out] buf Destination character buffer.
    /// @param[in] bufSize Buffer capacity in bytes.
    /// @param[in] value Integer value to format.
    /// @param[in] minDigits Minimum digit count (padded with leading zeros).
    /// @return Length of formatted string, or 0 on error.
    static std::size_t formatDecimal(
        char* buf, std::size_t bufSize, std::uint32_t value, std::size_t minDigits) noexcept;

private:
    static void drawGlyph(ShooterCore::RenderCommandQueue& queue, char c, float x, float y, std::uint8_t scale,
        ShooterCore::ColorRgba color) noexcept;
};

} // namespace ThexderGame
