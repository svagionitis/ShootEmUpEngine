#pragma once

#include <cstdint>

#include "Types.h"

struct SDL_Renderer;
struct SDL_Texture;

namespace SdlAdapter {

/// @class CanvasScaler
/// @brief Calculates pixel-perfect integer aspect ratio scaling and letterboxing viewports.
/// @details Eliminates non-uniform pixel stretching and shimmering on high-resolution displays.
class CanvasScaler final {
public:
    static constexpr std::int32_t kNativeWidth { 320 };
    static constexpr std::int32_t kNativeHeight { 240 };

    CanvasScaler() noexcept = default;

    /// @brief Calculate destination letterbox/pillarbox viewport for window dimensions.
    /// @param[in] winWidth Physical window width in pixels.
    /// @param[in] winHeight Physical window height in pixels.
    /// @param[in] mode Aspect ratio scaling strategy.
    /// @return Computed DisplayViewport parameters.
    [[nodiscard]] static ShooterCore::DisplayViewport calculateViewport(
        std::int32_t winWidth, std::int32_t winHeight, ShooterCore::AspectRatioMode mode) noexcept;

    /// @brief Blit native offscreen canvas texture to physical window with letterboxing.
    /// @param[in] renderer Hardware SDL renderer pointer.
    /// @param[in] canvasTexture Intermediate 320x240 render target texture.
    /// @param[in] winWidth Physical window width.
    /// @param[in] winHeight Physical window height.
    /// @param[in] mode Aspect ratio scaling strategy.
    static void presentCanvas(SDL_Renderer* renderer, SDL_Texture* canvasTexture, std::int32_t winWidth,
        std::int32_t winHeight, ShooterCore::AspectRatioMode mode) noexcept;
};

} // namespace SdlAdapter
