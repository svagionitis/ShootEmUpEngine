#include "CanvasScaler.h"

#include <cstdint>

#include <algorithm>

#include <SDL.h>

#include "Types.h"

namespace SdlAdapter {

ShooterCore::DisplayViewport CanvasScaler::calculateViewport(
    std::int32_t winWidth, std::int32_t winHeight, ShooterCore::AspectRatioMode mode) noexcept
{
    if (winWidth <= 0 || winHeight <= 0) {
        return ShooterCore::DisplayViewport { 0, 0, kNativeWidth, kNativeHeight, 1U };
    }

    if (mode == ShooterCore::AspectRatioMode::Stretch) {
        return ShooterCore::DisplayViewport { 0, 0, winWidth, winHeight, 1U };
    }

    if (mode == ShooterCore::AspectRatioMode::IntegerOnly) {
        const std::int32_t scaleX { winWidth / kNativeWidth };
        const std::int32_t scaleY { winHeight / kNativeHeight };
        const std::int32_t factor { std::max<std::int32_t>(1, std::min(scaleX, scaleY)) };

        const std::int32_t scaledW { factor * kNativeWidth };
        const std::int32_t scaledH { factor * kNativeHeight };

        const std::int32_t offsetX { (winWidth - scaledW) / 2 };
        const std::int32_t offsetY { (winHeight - scaledH) / 2 };

        return ShooterCore::DisplayViewport { offsetX, offsetY, scaledW, scaledH, static_cast<std::uint32_t>(factor) };
    }

    // UniformFit (Aspect-Correct Uniform Floating Point Scale)
    const float scaleF { std::min(static_cast<float>(winWidth) / static_cast<float>(kNativeWidth),
        static_cast<float>(winHeight) / static_cast<float>(kNativeHeight)) };

    const auto scaledW { std::max<std::int32_t>(
        1, static_cast<std::int32_t>(scaleF * static_cast<float>(kNativeWidth))) };
    const auto scaledH { std::max<std::int32_t>(
        1, static_cast<std::int32_t>(scaleF * static_cast<float>(kNativeHeight))) };

    const std::int32_t offsetX { (winWidth - scaledW) / 2 };
    const std::int32_t offsetY { (winHeight - scaledH) / 2 };
    const auto intFactor { static_cast<std::uint32_t>(std::max<std::int32_t>(1, static_cast<std::int32_t>(scaleF))) };

    return ShooterCore::DisplayViewport { offsetX, offsetY, scaledW, scaledH, intFactor };
}

void CanvasScaler::presentCanvas(SDL_Renderer* renderer, SDL_Texture* canvasTexture, std::int32_t winWidth,
    std::int32_t winHeight, ShooterCore::AspectRatioMode mode) noexcept
{
    if (renderer == nullptr || canvasTexture == nullptr) {
        return;
    }

    // Reset render target to physical window
    (void)SDL_SetRenderTarget(renderer, nullptr);

    // Matte black letterbox/pillarbox background
    (void)SDL_SetRenderDrawColor(renderer, 0U, 0U, 0U, 255U);
    (void)SDL_RenderClear(renderer);

    const ShooterCore::DisplayViewport vp { calculateViewport(winWidth, winHeight, mode) };

    const SDL_Rect dstRect { vp.x, vp.y, vp.width, vp.height };

    (void)SDL_RenderCopy(renderer, canvasTexture, nullptr, &dstRect);
}

} // namespace SdlAdapter
