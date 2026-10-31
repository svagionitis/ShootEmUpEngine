#pragma once

#include <cstdint>

#include "CanvasScaler.h"
#include "CrtPostProcessor.h"
#include "RenderCommandQueue.h"
#include "SpriteAtlas.h"
#include "Types.h"

struct SDL_Renderer;
struct SDL_Texture;

namespace SdlAdapter {

/// @class SdlRenderer
/// @brief Translates abstract RenderCommandQueue commands into hardware-accelerated SDL2 draw calls.
/// @details Features offscreen native 320x240 render targets, CRT optical post-processing, and integer aspect scaling.
class SdlRenderer final {
public:
    SdlRenderer() noexcept = default;
    ~SdlRenderer() noexcept;

    SdlRenderer(const SdlRenderer&) = delete;
    SdlRenderer& operator=(const SdlRenderer&) = delete;
    SdlRenderer(SdlRenderer&& other) noexcept;
    SdlRenderer& operator=(SdlRenderer&& other) noexcept;

    /// @brief Attach hardware SDL renderer and initialize sprite atlas and CRT processor.
    /// @param[in] renderer Valid hardware renderer pointer.
    void attachRenderer(SDL_Renderer* renderer) noexcept;

    /// @brief Retrieve attached hardware SDL renderer.
    /// @return Pointer to attached renderer or nullptr.
    [[nodiscard]] SDL_Renderer* getRenderer() const noexcept;

    /// @brief Retrieve underlying sprite atlas.
    /// @return Reference to sprite atlas.
    [[nodiscard]] const SpriteAtlas& getAtlas() const noexcept;

    /// @brief Consume and rasterize all commands in the command queue.
    /// @param[in] queue Frame render commands emitted by the engine core.
    void renderFrame(const ShooterCore::RenderCommandQueue& queue) noexcept;

    /// @brief Clear the screen with deep CRT backdrop color without presenting.
    void clearScreen() noexcept;

    /// @brief Present the rendered frame to the display.
    void present() noexcept;

    /// @brief Draw an individual render command without full frame clear/present.
    /// @param[in] cmd Single render command to draw.
    void drawCommand(const ShooterCore::RenderCommand& cmd) noexcept;

    /// @brief Set active aspect ratio scaling mode.
    /// @param[in] mode IntegerOnly, UniformFit, or Stretch.
    void setAspectRatioMode(ShooterCore::AspectRatioMode mode) noexcept
    {
        aspectMode_ = mode;
    }

    /// @brief Retrieve current aspect ratio scaling mode.
    /// @return Active AspectRatioMode.
    [[nodiscard]] constexpr ShooterCore::AspectRatioMode getAspectRatioMode() const noexcept
    {
        return aspectMode_;
    }

    /// @brief Cycle to next aspect ratio scaling mode.
    /// @return Newly selected AspectRatioMode.
    ShooterCore::AspectRatioMode cycleAspectMode() noexcept;

    /// @brief Set active optical CRT filter profile.
    /// @param[in] mode Disabled, Subtle, Arcade, or RetroCrt.
    void setCrtFilterMode(ShooterCore::CrtFilterMode mode) noexcept
    {
        crtProcessor_.setMode(mode);
    }

    /// @brief Retrieve active CRT filter profile.
    /// @return Current CrtFilterMode.
    [[nodiscard]] constexpr ShooterCore::CrtFilterMode getCrtFilterMode() const noexcept
    {
        return crtProcessor_.mode();
    }

    /// @brief Cycle to next CRT filter profile.
    /// @return Newly selected CrtFilterMode.
    ShooterCore::CrtFilterMode cycleCrtMode() noexcept
    {
        return crtProcessor_.cycleMode();
    }

    /// @brief Retrieve CRT post-processor reference.
    /// @return Reference to CrtPostProcessor.
    [[nodiscard]] CrtPostProcessor& getCrtProcessor() noexcept
    {
        return crtProcessor_;
    }

    /// @brief Retrieve offscreen native canvas target texture.
    /// @return Pointer to SDL_Texture or nullptr.
    [[nodiscard]] SDL_Texture* getTargetTexture() const noexcept
    {
        return targetTexture_;
    }

private:
    void drawTile(const ShooterCore::RenderCommand& cmd) noexcept;
    void drawSprite(const ShooterCore::RenderCommand& cmd) noexcept;
    void drawBeam(const ShooterCore::RenderCommand& cmd) noexcept;
    void drawBar(const ShooterCore::RenderCommand& cmd) noexcept;
    void drawLightning(const ShooterCore::RenderCommand& cmd) noexcept;
    void drawShieldBubble(const ShooterCore::RenderCommand& cmd) noexcept;
    void drawParticles(const ShooterCore::RenderCommand& cmd) noexcept;
    void drawShockwave(const ShooterCore::RenderCommand& cmd) noexcept;

    void drawRobotSprite(float x, float y, ShooterCore::FacingDirection facing, bool isAltWalk, bool isJump) noexcept;
    void drawJetSprite(float x, float y, ShooterCore::FacingDirection facing) noexcept;
    void drawTransformSprite(float x, float y, ShooterCore::FacingDirection facing) noexcept;
    void drawEnemyPatrol(float x, float y, ShooterCore::FacingDirection facing) noexcept;
    void drawEnemyFlyer(float x, float y) noexcept;
    void drawExplosion(float x, float y, float size) noexcept;

    SDL_Renderer* renderer_ { nullptr };
    SDL_Texture* targetTexture_ { nullptr };
    SpriteAtlas atlas_ { };
    CrtPostProcessor crtProcessor_ { };
    ShooterCore::AspectRatioMode aspectMode_ { ShooterCore::AspectRatioMode::IntegerOnly };
    const ShooterCore::RenderCommandQueue* activeQueue_ { nullptr };
};

} // namespace SdlAdapter
