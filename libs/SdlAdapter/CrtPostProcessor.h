#pragma once

#include <array>
#include <cstdint>

#include "Types.h"

struct SDL_Renderer;
struct SDL_Texture;

namespace SdlAdapter {

/// @class CrtPostProcessor
/// @brief Multi-tier optical CRT post-processing simulator delivering scanlines, aperture grille, and vignette.
/// @details Pre-bakes hardware overlay textures at native resolution, rendering with zero heap allocations.
class CrtPostProcessor final {
public:
    static constexpr std::int32_t kWidth { 320 };
    static constexpr std::int32_t kHeight { 240 };

    CrtPostProcessor() noexcept = default;
    ~CrtPostProcessor() noexcept;

    CrtPostProcessor(const CrtPostProcessor&) = delete;
    CrtPostProcessor& operator=(const CrtPostProcessor&) = delete;
    CrtPostProcessor(CrtPostProcessor&& other) noexcept;
    CrtPostProcessor& operator=(CrtPostProcessor&& other) noexcept;

    /// @brief Initialize procedural CRT overlay textures for all visual profiles.
    /// @param[in] renderer Valid hardware SDL renderer.
    /// @return true if initialized successfully; false otherwise.
    [[nodiscard]] bool init(SDL_Renderer* renderer) noexcept;

    /// @brief Release all CRT texture resources.
    void shutdown() noexcept;

    /// @brief Set active CRT filter mode.
    /// @param[in] mode Desired profile (Disabled, Subtle, Arcade, RetroCrt).
    void setMode(ShooterCore::CrtFilterMode mode) noexcept
    {
        mode_ = mode;
    }

    /// @brief Cycle to the next CRT filter mode.
    /// @return Newly selected CrtFilterMode.
    ShooterCore::CrtFilterMode cycleMode() noexcept;

    /// @brief Retrieve active CRT filter mode.
    /// @return Current mode enum.
    [[nodiscard]] constexpr ShooterCore::CrtFilterMode mode() const noexcept
    {
        return mode_;
    }

    /// @brief Apply CRT post-processing overlay to currently active render target.
    /// @param[in] renderer Hardware SDL renderer.
    void render(SDL_Renderer* renderer) noexcept;

private:
    void generateOverlay(
        SDL_Renderer* renderer, ShooterCore::CrtFilterMode mode, float scanlineAlpha, float vigFactor) noexcept;

    ShooterCore::CrtFilterMode mode_ { ShooterCore::CrtFilterMode::Arcade };
    std::array<SDL_Texture*, 4> textures_ { nullptr, nullptr, nullptr, nullptr };
};

} // namespace SdlAdapter
