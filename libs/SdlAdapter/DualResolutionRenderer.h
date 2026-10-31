#pragma once

#include <cstddef>
#include <cstdint>

#include "Hitbox.h"
#include "Types.h"

namespace SdlAdapter {

/// @class DualResolutionRenderer
/// @brief Compositor for Firehawk (Thexder 2) hybrid MCGA 320x200 / EGA 640x200 video modes.
/// @details Combines a 320x168 MCGA 256-color action viewport with a high-density 640x32 EGA dashboard
///          with strictly zero runtime heap allocations (CERT ERR55-CPP).
class DualResolutionRenderer final {
public:
    static constexpr std::uint32_t kMcgaWidth { 320U };
    static constexpr std::uint32_t kMcgaHeight { 200U };
    static constexpr std::uint32_t kPlayfieldHeight { 168U };
    static constexpr std::uint32_t kEgaWidth { 640U };
    static constexpr std::uint32_t kEgaHeight { 200U };
    static constexpr std::uint32_t kDashboardHeight { 32U };
    static constexpr float kDashboardStartY { 168.0f };

    DualResolutionRenderer() noexcept = default;

    /// @brief Transform a continuous coordinate in 320x168 MCGA playfield space into 640x200 composite space.
    /// @param[in] mcgaPos Position in 320x168 coordinate space.
    /// @return Transformed coordinate in 640x200 composite space.
    [[nodiscard]] static constexpr ShooterCore::WorldPos playfieldToComposite(ShooterCore::WorldPos mcgaPos) noexcept
    {
        return ShooterCore::WorldPos { mcgaPos.x * 2.0f, mcgaPos.y };
    }

    /// @brief Transform a coordinate in 640x200 composite space back into 320x168 playfield space.
    /// @param[in] compPos Position in 640x200 composite space.
    /// @return Position in 320x168 MCGA space.
    [[nodiscard]] static constexpr ShooterCore::WorldPos compositeToPlayfield(ShooterCore::WorldPos compPos) noexcept
    {
        return ShooterCore::WorldPos { compPos.x * 0.5f, compPos.y };
    }

    /// @brief Check whether a coordinate lies within the 320x168 active action viewport.
    /// @param[in] pos Continuous position.
    /// @return true if inside active gameplay playfield.
    [[nodiscard]] static constexpr bool isInsidePlayfield(ShooterCore::WorldPos pos) noexcept
    {
        return pos.x >= 0.0f && pos.x < static_cast<float>(kMcgaWidth) && pos.y >= 0.0f
            && pos.y < static_cast<float>(kPlayfieldHeight);
    }

    /// @brief Check whether a composite coordinate lies inside the high-density bottom dashboard.
    /// @param[in] compPos Position in 640x200 composite space.
    /// @return true if inside bottom status dashboard.
    [[nodiscard]] static constexpr bool isInsideDashboard(ShooterCore::WorldPos compPos) noexcept
    {
        return compPos.x >= 0.0f && compPos.x < static_cast<float>(kEgaWidth) && compPos.y >= kDashboardStartY
            && compPos.y < static_cast<float>(kEgaHeight);
    }

    /// @brief Compute letterboxed aspect-correct (4:3) destination rect for any window dimension.
    /// @param[in] winWidth Current window width in pixels.
    /// @param[in] winHeight Current window height in pixels.
    /// @param[out] outX Scaled viewport origin X.
    /// @param[out] outY Scaled viewport origin Y.
    /// @param[out] outW Scaled viewport width.
    /// @param[out] outH Scaled viewport height.
    static void computeLetterboxRect(std::int32_t winWidth, std::int32_t winHeight, std::int32_t& outX,
        std::int32_t& outY, std::int32_t& outW, std::int32_t& outH) noexcept;
};

} // namespace SdlAdapter
