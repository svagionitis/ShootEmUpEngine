#include "CrtPostProcessor.h"

#include <cmath>
#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <array>
#include <memory>
#include <utility>

#include <SDL.h>

#include "Types.h"

namespace SdlAdapter {

namespace {

    [[nodiscard]] constexpr std::uint32_t packRgba(
        std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a) noexcept
    {
        return (static_cast<std::uint32_t>(a) << 24U) | (static_cast<std::uint32_t>(b) << 16U)
            | (static_cast<std::uint32_t>(g) << 8U) | static_cast<std::uint32_t>(r);
    }

} // namespace

CrtPostProcessor::~CrtPostProcessor() noexcept
{
    shutdown();
}

CrtPostProcessor::CrtPostProcessor(CrtPostProcessor&& other) noexcept
    : mode_ { other.mode_ }
    , textures_ { other.textures_ }
{
    other.textures_.fill(nullptr);
}

CrtPostProcessor& CrtPostProcessor::operator=(CrtPostProcessor&& other) noexcept
{
    if (this != &other) {
        shutdown();
        mode_ = other.mode_;
        textures_ = other.textures_;
        other.textures_.fill(nullptr);
    }
    return *this;
}

bool CrtPostProcessor::init(SDL_Renderer* renderer) noexcept
{
    shutdown();

    if (renderer == nullptr) {
        return false;
    }

    // Index 0: Disabled (nullptr)
    textures_[0] = nullptr;

    // Index 1: Subtle (15% scanlines, light vignette)
    generateOverlay(renderer, ShooterCore::CrtFilterMode::Subtle, 38.0f, 40.0f);

    // Index 2: Arcade (33% scanlines, moderate vignette)
    generateOverlay(renderer, ShooterCore::CrtFilterMode::Arcade, 85.0f, 80.0f);

    // Index 3: RetroCrt (50% scanlines, heavy vignette)
    generateOverlay(renderer, ShooterCore::CrtFilterMode::RetroCrt, 130.0f, 140.0f);

    return true;
}

void CrtPostProcessor::generateOverlay(
    SDL_Renderer* renderer, ShooterCore::CrtFilterMode filterMode, float scanlineAlpha, float vigFactor) noexcept
{
    const auto idx { static_cast<std::size_t>(filterMode) };
    if (idx >= textures_.size()) {
        return;
    }

    const auto pixelCount { static_cast<std::size_t>(kWidth * kHeight) };
    auto pixelBuf { std::make_unique<std::uint32_t[]>(pixelCount) };

    const float cx { static_cast<float>(kWidth) * 0.5f };
    const float cy { static_cast<float>(kHeight) * 0.5f };

    for (std::int32_t y { 0 }; y < kHeight; ++y) {
        const float v { (static_cast<float>(y) - cy) / cy };
        const float v4 { v * v * v * v };
        const bool isOddRow { (y % 2) != 0 };

        for (std::int32_t x { 0 }; x < kWidth; ++x) {
            const float u { (static_cast<float>(x) - cx) / cx };
            const float u4 { u * u * u * u };
            const float vig { std::clamp((u4 + v4) * vigFactor, 0.0f, 255.0f) };

            float alpha { vig };
            if (isOddRow) {
                alpha = std::clamp(scanlineAlpha + (vig * (1.0f - (scanlineAlpha / 255.0f))), 0.0f, 255.0f);
            }

            const auto aByte { static_cast<std::uint8_t>(alpha) };
            const std::size_t pidx { static_cast<std::size_t>(y * kWidth + x) };

            // Pure black attenuation with computed alpha transparency
            pixelBuf[pidx] = packRgba(0U, 0U, 0U, aByte);
        }
    }

    SDL_Surface* surface { SDL_CreateRGBSurfaceWithFormatFrom(pixelBuf.get(), kWidth, kHeight, 32,
        kWidth * static_cast<int>(sizeof(std::uint32_t)), SDL_PIXELFORMAT_RGBA32) };

    if (surface != nullptr) {
        textures_[idx] = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);

        if (textures_[idx] != nullptr) {
            (void)SDL_SetTextureBlendMode(textures_[idx], SDL_BLENDMODE_BLEND);
        }
    }
}

void CrtPostProcessor::shutdown() noexcept
{
    for (std::size_t i { 0U }; i < textures_.size(); ++i) {
        if (textures_[i] != nullptr) {
            SDL_DestroyTexture(textures_[i]);
            textures_[i] = nullptr;
        }
    }
}

ShooterCore::CrtFilterMode CrtPostProcessor::cycleMode() noexcept
{
    const auto curVal { static_cast<std::uint8_t>(mode_) };
    const std::uint8_t nextVal { static_cast<std::uint8_t>((curVal + 1U) % 4U) };
    mode_ = static_cast<ShooterCore::CrtFilterMode>(nextVal);
    return mode_;
}

void CrtPostProcessor::render(SDL_Renderer* renderer) noexcept
{
    if (renderer == nullptr || mode_ == ShooterCore::CrtFilterMode::Disabled) {
        return;
    }

    const auto idx { static_cast<std::size_t>(mode_) };
    if (idx < textures_.size() && textures_[idx] != nullptr) {
        (void)SDL_RenderCopy(renderer, textures_[idx], nullptr, nullptr);
    }
}

} // namespace SdlAdapter
