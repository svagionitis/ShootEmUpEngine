#pragma once

#include <cstdint>

#include "RenderCommandQueue.h"
#include "Types.h"

struct SDL_Renderer;

namespace SdlAdapter {

/// @class SdlRenderer
/// @brief Translates abstract RenderCommandQueue commands into hardware-accelerated SDL2 draw calls.
class SdlRenderer final {
public:
    SdlRenderer() noexcept = default;
    ~SdlRenderer() noexcept = default;

    SdlRenderer(const SdlRenderer&) = delete;
    SdlRenderer& operator=(const SdlRenderer&) = delete;
    SdlRenderer(SdlRenderer&&) noexcept = default;
    SdlRenderer& operator=(SdlRenderer&&) noexcept = default;

    /// @brief Attach hardware SDL renderer.
    /// @param[in] renderer Valid hardware renderer pointer.
    void attachRenderer(SDL_Renderer* renderer) noexcept;

    /// @brief Retrieve attached hardware SDL renderer.
    /// @return Pointer to attached renderer or nullptr.
    [[nodiscard]] SDL_Renderer* getRenderer() const noexcept;

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

private:
    void drawTile(const ShooterCore::RenderCommand& cmd) noexcept;
    void drawSprite(const ShooterCore::RenderCommand& cmd) noexcept;
    void drawBeam(const ShooterCore::RenderCommand& cmd) noexcept;
    void drawBar(const ShooterCore::RenderCommand& cmd) noexcept;

    void drawRobotSprite(float x, float y, ShooterCore::FacingDirection facing, bool isAltWalk, bool isJump) noexcept;
    void drawJetSprite(float x, float y, ShooterCore::FacingDirection facing) noexcept;
    void drawTransformSprite(float x, float y, ShooterCore::FacingDirection facing) noexcept;
    void drawEnemyPatrol(float x, float y, ShooterCore::FacingDirection facing) noexcept;
    void drawEnemyFlyer(float x, float y) noexcept;
    void drawExplosion(float x, float y, float size) noexcept;

    SDL_Renderer* renderer_ { nullptr };
};

} // namespace SdlAdapter
