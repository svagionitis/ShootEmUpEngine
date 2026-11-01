#include "SdlRenderer.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <cmath>
#include <utility>

#include <SDL.h>

#include "CanvasScaler.h"
#include "CrtPostProcessor.h"
#include "RenderCommandQueue.h"
#include "SpriteAtlas.h"
#include "Types.h"

namespace SdlAdapter {

namespace {

    constexpr float kTileDimension { 16.0f };

    [[nodiscard]] constexpr int toIntClamped(float val) noexcept
    {
        if (val < -32768.0f) {
            return -32768;
        }
        if (val > 32767.0f) {
            return 32767;
        }
        return static_cast<int>(val);
    }

} // namespace

SdlRenderer::~SdlRenderer() noexcept
{
    if (targetTexture_ != nullptr) {
        SDL_DestroyTexture(targetTexture_);
        targetTexture_ = nullptr;
    }
}

SdlRenderer::SdlRenderer(SdlRenderer&& other) noexcept
    : renderer_ { other.renderer_ }
    , targetTexture_ { other.targetTexture_ }
    , atlas_ { std::move(other.atlas_) }
    , crtProcessor_ { std::move(other.crtProcessor_) }
    , aspectMode_ { other.aspectMode_ }
    , activeQueue_ { other.activeQueue_ }
{
    other.renderer_ = nullptr;
    other.targetTexture_ = nullptr;
    other.activeQueue_ = nullptr;
}

SdlRenderer& SdlRenderer::operator=(SdlRenderer&& other) noexcept
{
    if (this != &other) {
        if (targetTexture_ != nullptr) {
            SDL_DestroyTexture(targetTexture_);
        }
        renderer_ = other.renderer_;
        targetTexture_ = other.targetTexture_;
        atlas_ = std::move(other.atlas_);
        crtProcessor_ = std::move(other.crtProcessor_);
        aspectMode_ = other.aspectMode_;
        activeQueue_ = other.activeQueue_;

        other.renderer_ = nullptr;
        other.targetTexture_ = nullptr;
        other.activeQueue_ = nullptr;
    }
    return *this;
}

ShooterCore::AspectRatioMode SdlRenderer::cycleAspectMode() noexcept
{
    const auto curVal { static_cast<std::uint8_t>(aspectMode_) };
    const std::uint8_t nextVal { static_cast<std::uint8_t>((curVal + 1U) % 3U) };
    aspectMode_ = static_cast<ShooterCore::AspectRatioMode>(nextVal);
    return aspectMode_;
}

void SdlRenderer::attachRenderer(SDL_Renderer* renderer) noexcept
{
    if (targetTexture_ != nullptr) {
        SDL_DestroyTexture(targetTexture_);
        targetTexture_ = nullptr;
    }

    renderer_ = renderer;
    if (renderer_ != nullptr) {
        (void)atlas_.init(renderer_);
        (void)crtProcessor_.init(renderer_);

        // Allocate 320x240 native offscreen render target
        targetTexture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 320, 240);
        if (targetTexture_ != nullptr) {
            (void)SDL_SetTextureBlendMode(targetTexture_, SDL_BLENDMODE_BLEND);
        }
    } else {
        atlas_.shutdown();
        crtProcessor_.shutdown();
    }
}

SDL_Renderer* SdlRenderer::getRenderer() const noexcept
{
    return renderer_;
}

const SpriteAtlas& SdlRenderer::getAtlas() const noexcept
{
    return atlas_;
}

void SdlRenderer::clearScreen() noexcept
{
    if (renderer_ == nullptr) {
        return;
    }
    // Deep authentic CRT sci-fi background (dark navy)
    (void)SDL_SetRenderDrawColor(renderer_, 8U, 12U, 24U, 255U);
    (void)SDL_RenderClear(renderer_);
}

void SdlRenderer::present() noexcept
{
    if (renderer_ != nullptr) {
        SDL_RenderPresent(renderer_);
    }
}

void SdlRenderer::renderFrame(const ShooterCore::RenderCommandQueue& queue) noexcept
{
    if (renderer_ == nullptr) {
        return;
    }

    if (targetTexture_ != nullptr) {
        // Direct drawing to 320x240 native offscreen target
        (void)SDL_SetRenderTarget(renderer_, targetTexture_);
        clearScreen();

        activeQueue_ = &queue;
        const std::size_t cmdCount { queue.count() };
        for (std::size_t i { 0U }; i < cmdCount; ++i) {
            drawCommand(queue.get(i));
        }
        activeQueue_ = nullptr;

        // Apply optical CRT filter (scanlines, aperture grille, vignette)
        crtProcessor_.render(renderer_);

        // Present to physical window with integer aspect ratio letterboxing
        int winW { 320 };
        int winH { 240 };
        (void)SDL_GetRendererOutputSize(renderer_, &winW, &winH);
        CanvasScaler::presentCanvas(renderer_, targetTexture_, winW, winH, aspectMode_);
        present();
    } else {
        // Fallback for mock/dummy test renderers where target textures are unsupported
        clearScreen();

        activeQueue_ = &queue;
        const std::size_t cmdCount { queue.count() };
        for (std::size_t i { 0U }; i < cmdCount; ++i) {
            drawCommand(queue.get(i));
        }
        activeQueue_ = nullptr;

        crtProcessor_.render(renderer_);
        present();
    }
}

void SdlRenderer::drawCommand(const ShooterCore::RenderCommand& cmd) noexcept
{
    if (renderer_ == nullptr) {
        return;
    }

    switch (cmd.type) {
    case ShooterCore::RenderCommandType::DrawTile:
        drawTile(cmd);
        break;
    case ShooterCore::RenderCommandType::DrawSprite:
        drawSprite(cmd);
        break;
    case ShooterCore::RenderCommandType::DrawBeam:
        drawBeam(cmd);
        break;
    case ShooterCore::RenderCommandType::DrawBar:
        drawBar(cmd);
        break;
    case ShooterCore::RenderCommandType::DrawLightning:
        drawLightning(cmd);
        break;
    case ShooterCore::RenderCommandType::DrawShield:
        drawShieldBubble(cmd);
        break;
    case ShooterCore::RenderCommandType::DrawParticle:
        drawParticles(cmd);
        break;
    case ShooterCore::RenderCommandType::DrawShockwave:
        drawShockwave(cmd);
        break;
    case ShooterCore::RenderCommandType::None:
    default:
        break;
    }
}

void SdlRenderer::drawTile(const ShooterCore::RenderCommand& cmd) noexcept
{
    const int x { toIntClamped(cmd.pos.x) };
    const int y { toIntClamped(cmd.pos.y) };
    const int size { static_cast<int>(kTileDimension) };

    const SDL_Rect tileRect { x, y, size, size };

    if (atlas_.isLoaded()) {
        const SDL_Rect srcRect { atlas_.getTileSourceRect(cmd.tileType) };
        if (srcRect.w > 0 && srcRect.h > 0) {
            (void)SDL_RenderCopy(renderer_, atlas_.getTexture(), &srcRect, &tileRect);
            return;
        }
    }

    switch (cmd.tileType) {
    case ShooterCore::TileType::SolidWall: {
        // Metallic impenetrable labyrinth block: Slate steel fill with beveled highlight
        (void)SDL_SetRenderDrawColor(renderer_, 65U, 75U, 95U, 255U);
        (void)SDL_RenderFillRect(renderer_, &tileRect);

        // Border edge
        (void)SDL_SetRenderDrawColor(renderer_, 110U, 125U, 150U, 255U);
        (void)SDL_RenderDrawRect(renderer_, &tileRect);

        // Internal cross rivets
        (void)SDL_SetRenderDrawColor(renderer_, 45U, 55U, 75U, 255U);
        (void)SDL_RenderDrawLine(renderer_, x + 4, y + 4, x + size - 5, y + size - 5);
        (void)SDL_RenderDrawLine(renderer_, x + size - 5, y + 4, x + 4, y + size - 5);
        break;
    }

    case ShooterCore::TileType::DestructibleWall: {
        // Crumbled masonry wall: Earth amber/brown with mortar cracks
        (void)SDL_SetRenderDrawColor(renderer_, 135U, 90U, 50U, 255U);
        (void)SDL_RenderFillRect(renderer_, &tileRect);

        (void)SDL_SetRenderDrawColor(renderer_, 190U, 130U, 80U, 255U);
        (void)SDL_RenderDrawRect(renderer_, &tileRect);

        // Horizontal brick lines
        (void)SDL_SetRenderDrawColor(renderer_, 80U, 50U, 30U, 255U);
        (void)SDL_RenderDrawLine(renderer_, x, y + 8, x + size - 1, y + 8);
        (void)SDL_RenderDrawLine(renderer_, x + 8, y, x + 8, y + 7);
        (void)SDL_RenderDrawLine(renderer_, x + 4, y + 8, x + 4, y + size - 1);
        (void)SDL_RenderDrawLine(renderer_, x + 12, y + 8, x + 12, y + size - 1);
        break;
    }

    case ShooterCore::TileType::Hazard: {
        // Energy / Lava hazard block: High-voltage pulsating orange-red
        (void)SDL_SetRenderDrawColor(renderer_, 230U, 50U, 30U, 255U);
        (void)SDL_RenderFillRect(renderer_, &tileRect);

        // Warning diagonal hazard stripes
        (void)SDL_SetRenderDrawColor(renderer_, 255U, 220U, 40U, 255U);
        (void)SDL_RenderDrawLine(renderer_, x, y + 4, x + 4, y);
        (void)SDL_RenderDrawLine(renderer_, x, y + 12, x + 12, y);
        (void)SDL_RenderDrawLine(renderer_, x + 4, y + size - 1, x + size - 1, y + 4);
        break;
    }

    case ShooterCore::TileType::Empty:
    default:
        break;
    }
}

void SdlRenderer::drawSprite(const ShooterCore::RenderCommand& cmd) noexcept
{
    const float x { cmd.pos.x };
    const float y { cmd.pos.y };

    if (atlas_.isLoaded()) {
        const SDL_Rect srcRect { atlas_.getSourceRect(cmd.sprite) };
        if (srcRect.w > 0 && srcRect.h > 0) {
            float drawX { x };
            float drawY { y };
            if (srcRect.w == 48 && atlas_.isEgaRobotLoaded()) {
                if (ShooterCore::isEgaMechaJet(cmd.sprite)) {
                    drawX -= 24.0f;
                    drawY -= 16.0f;
                } else if (ShooterCore::isEgaMechaRobot(cmd.sprite)) {
                    drawX -= 24.0f;
                    drawY -= 32.0f;
                }
            } else {
                drawX = x - (static_cast<float>(srcRect.w) * 0.5f);
                drawY = y - static_cast<float>(srcRect.h);
            }

            const SDL_Rect dstRect { toIntClamped(drawX), toIntClamped(drawY), srcRect.w, srcRect.h };
            SDL_RendererFlip flip { SDL_FLIP_NONE };
            if (cmd.facing == ShooterCore::FacingDirection::Left) {
                flip = SDL_FLIP_HORIZONTAL;
            }

            (void)SDL_RenderCopyEx(renderer_, atlas_.getTexture(), &srcRect, &dstRect,
                static_cast<double>(cmd.rotationDegrees), nullptr, flip);
            return;
        }
    }

    switch (cmd.sprite) {
    case ShooterCore::SpriteId::PlayerRobotWalk1:
        drawRobotSprite(x, y, cmd.facing, false, false);
        break;
    case ShooterCore::SpriteId::PlayerRobotWalk2:
        drawRobotSprite(x, y, cmd.facing, true, false);
        break;
    case ShooterCore::SpriteId::PlayerRobotJump:
        drawRobotSprite(x, y, cmd.facing, false, true);
        break;
    case ShooterCore::SpriteId::PlayerJet:
        drawJetSprite(x, y, cmd.facing);
        break;
    case ShooterCore::SpriteId::PlayerTransforming:
        drawTransformSprite(x, y, cmd.facing);
        break;
    case ShooterCore::SpriteId::EnemyPatrol:
        drawEnemyPatrol(x, y, cmd.facing);
        break;
    case ShooterCore::SpriteId::EnemyFlyer:
        drawEnemyFlyer(x, y);
        break;
    case ShooterCore::SpriteId::Explosion:
        drawExplosion(x, y, cmd.width > 0.0f ? cmd.width : 24.0f);
        break;
    case ShooterCore::SpriteId::SparkParticle: {
        (void)SDL_SetRenderDrawColor(renderer_, 255U, 240U, 80U, 255U);
        const int px { toIntClamped(x) };
        const int py { toIntClamped(y) };
        (void)SDL_RenderDrawPoint(renderer_, px, py);
        (void)SDL_RenderDrawPoint(renderer_, px + 1, py);
        (void)SDL_RenderDrawPoint(renderer_, px, py + 1);
        (void)SDL_RenderDrawPoint(renderer_, px + 1, py + 1);
        break;
    }
    case ShooterCore::SpriteId::MissileInFlight: {
        (void)SDL_SetRenderDrawColor(renderer_, 255U, 180U, 40U, 255U);
        const SDL_Rect mRect { toIntClamped(x) - 4, toIntClamped(y) - 2, 8, 4 };
        (void)SDL_RenderFillRect(renderer_, &mRect);
        break;
    }
    case ShooterCore::SpriteId::CanisterMissile:
    case ShooterCore::SpriteId::CanisterBomb:
    case ShooterCore::SpriteId::CanisterEnergy:
    case ShooterCore::SpriteId::CanisterShield: {
        if (cmd.sprite == ShooterCore::SpriteId::CanisterMissile) {
            (void)SDL_SetRenderDrawColor(renderer_, 80U, 220U, 80U, 255U);
        } else if (cmd.sprite == ShooterCore::SpriteId::CanisterBomb) {
            (void)SDL_SetRenderDrawColor(renderer_, 255U, 140U, 20U, 255U);
        } else if (cmd.sprite == ShooterCore::SpriteId::CanisterEnergy) {
            (void)SDL_SetRenderDrawColor(renderer_, 60U, 200U, 255U, 255U);
        } else {
            (void)SDL_SetRenderDrawColor(renderer_, 220U, 80U, 255U, 255U);
        }
        const SDL_Rect cRect { toIntClamped(x) - 6, toIntClamped(y) - 6, 12, 12 };
        (void)SDL_RenderFillRect(renderer_, &cRect);
        break;
    }
    case ShooterCore::SpriteId::EnemyInterceptor: {
        (void)SDL_SetRenderDrawColor(renderer_, 240U, 50U, 60U, 255U);
        const SDL_Rect iRect { toIntClamped(x) - 8, toIntClamped(y) - 6, 16, 12 };
        (void)SDL_RenderFillRect(renderer_, &iRect);
        break;
    }
    case ShooterCore::SpriteId::EnemyCrawler1:
    case ShooterCore::SpriteId::EnemyCrawler2: {
        (void)SDL_SetRenderDrawColor(renderer_, 180U, 140U, 40U, 255U);
        const SDL_Rect crRect { toIntClamped(x) - 6, toIntClamped(y) - 6, 12, 12 };
        (void)SDL_RenderFillRect(renderer_, &crRect);
        break;
    }
    case ShooterCore::SpriteId::EnemyTurretCeiling: {
        (void)SDL_SetRenderDrawColor(renderer_, 160U, 40U, 180U, 255U);
        const SDL_Rect tRect { toIntClamped(x) - 8, toIntClamped(y) - 4, 16, 10 };
        (void)SDL_RenderFillRect(renderer_, &tRect);
        break;
    }
    case ShooterCore::SpriteId::EnemyBulletHostile: {
        (void)SDL_SetRenderDrawColor(renderer_, 255U, 60U, 60U, 255U);
        const SDL_Rect bRect { toIntClamped(x) - 2, toIntClamped(y) - 2, 4, 4 };
        (void)SDL_RenderFillRect(renderer_, &bRect);
        break;
    }
    case ShooterCore::SpriteId::NestCoreActive: {
        (void)SDL_SetRenderDrawColor(renderer_, 200U, 40U, 80U, 255U);
        const SDL_Rect nRect { toIntClamped(x) - 14, toIntClamped(y) - 14, 28, 28 };
        (void)SDL_RenderFillRect(renderer_, &nRect);
        break;
    }
    case ShooterCore::SpriteId::None:
    default:
        break;
    }
}

void SdlRenderer::drawRobotSprite(
    float x, float y, ShooterCore::FacingDirection facing, bool isAltWalk, bool isJump) noexcept
{
    const int rx { toIntClamped(x) };
    const int ry { toIntClamped(y) };
    const bool isLeft { facing == ShooterCore::FacingDirection::Left };

    // Torso armor plate (16 x 14 px)
    const SDL_Rect torsoRect { rx + 2, ry + 8, 12, 14 };
    (void)SDL_SetRenderDrawColor(renderer_, 40U, 120U, 200U, 255U);
    (void)SDL_RenderFillRect(renderer_, &torsoRect);

    // Head / Helm (10 x 8 px)
    const SDL_Rect headRect { rx + 3, ry, 10, 8 };
    (void)SDL_SetRenderDrawColor(renderer_, 70U, 140U, 220U, 255U);
    (void)SDL_RenderFillRect(renderer_, &headRect);

    // Visor eye slit (Cyan glow)
    (void)SDL_SetRenderDrawColor(renderer_, 60U, 240U, 255U, 255U);
    const int eyeX { isLeft ? rx + 3 : rx + 8 };
    const SDL_Rect visorRect { eyeX, ry + 3, 5, 2 };
    (void)SDL_RenderFillRect(renderer_, &visorRect);

    // Chest power core
    (void)SDL_SetRenderDrawColor(renderer_, 255U, 80U, 60U, 255U);
    (void)SDL_RenderDrawPoint(renderer_, rx + 7, ry + 13);
    (void)SDL_RenderDrawPoint(renderer_, rx + 8, ry + 13);

    // Legs / Thrusters
    (void)SDL_SetRenderDrawColor(renderer_, 30U, 70U, 130U, 255U);
    if (isJump) {
        // Tucked jumping thrust legs with exhaust glow
        const SDL_Rect leftLeg { rx + 2, ry + 22, 5, 6 };
        const SDL_Rect rightLeg { rx + 9, ry + 22, 5, 6 };
        (void)SDL_RenderFillRect(renderer_, &leftLeg);
        (void)SDL_RenderFillRect(renderer_, &rightLeg);

        // Thruster sparks
        (void)SDL_SetRenderDrawColor(renderer_, 255U, 200U, 50U, 255U);
        (void)SDL_RenderDrawLine(renderer_, rx + 4, ry + 28, rx + 4, ry + 31);
        (void)SDL_RenderDrawLine(renderer_, rx + 11, ry + 28, rx + 11, ry + 31);
    } else if (isAltWalk) {
        // Stride frame 2
        const SDL_Rect leftLeg { rx + 1, ry + 22, 5, 10 };
        const SDL_Rect rightLeg { rx + 10, ry + 22, 5, 10 };
        (void)SDL_RenderFillRect(renderer_, &leftLeg);
        (void)SDL_RenderFillRect(renderer_, &rightLeg);
    } else {
        // Stride frame 1
        const SDL_Rect leftLeg { rx + 3, ry + 22, 5, 10 };
        const SDL_Rect rightLeg { rx + 8, ry + 22, 5, 10 };
        (void)SDL_RenderFillRect(renderer_, &leftLeg);
        (void)SDL_RenderFillRect(renderer_, &rightLeg);
    }
}

void SdlRenderer::drawJetSprite(float x, float y, ShooterCore::FacingDirection facing) noexcept
{
    const int jx { toIntClamped(x) };
    const int jy { toIntClamped(y) };
    const bool isLeft { facing == ShooterCore::FacingDirection::Left };

    // Sleek aerodynamic delta wing fuselage (32 x 16 px)
    // Main wing body
    const SDL_Rect wingRect { jx + 4, jy + 4, 24, 8 };
    (void)SDL_SetRenderDrawColor(renderer_, 40U, 120U, 200U, 255U);
    (void)SDL_RenderFillRect(renderer_, &wingRect);

    // Cockpit canopy (Cyan glass)
    (void)SDL_SetRenderDrawColor(renderer_, 80U, 230U, 255U, 255U);
    const int cockpitX { isLeft ? jx + 6 : jx + 18 };
    const SDL_Rect cockpitRect { cockpitX, jy + 3, 8, 3 };
    (void)SDL_RenderFillRect(renderer_, &cockpitRect);

    // Needle nose cone
    (void)SDL_SetRenderDrawColor(renderer_, 70U, 140U, 220U, 255U);
    if (isLeft) {
        (void)SDL_RenderDrawLine(renderer_, jx + 4, jy + 4, jx, jy + 8);
        (void)SDL_RenderDrawLine(renderer_, jx + 4, jy + 11, jx, jy + 8);
        // Rear thrust exhaust flame
        (void)SDL_SetRenderDrawColor(renderer_, 255U, 180U, 40U, 255U);
        (void)SDL_RenderDrawLine(renderer_, jx + 28, jy + 7, jx + 31, jy + 8);
        (void)SDL_RenderDrawLine(renderer_, jx + 28, jy + 9, jx + 31, jy + 8);
    } else {
        (void)SDL_RenderDrawLine(renderer_, jx + 27, jy + 4, jx + 31, jy + 8);
        (void)SDL_RenderDrawLine(renderer_, jx + 27, jy + 11, jx + 31, jy + 8);
        // Rear thrust exhaust flame
        (void)SDL_SetRenderDrawColor(renderer_, 255U, 180U, 40U, 255U);
        (void)SDL_RenderDrawLine(renderer_, jx + 3, jy + 7, jx, jy + 8);
        (void)SDL_RenderDrawLine(renderer_, jx + 3, jy + 9, jx, jy + 8);
    }
}

void SdlRenderer::drawTransformSprite(float x, float y, ShooterCore::FacingDirection facing) noexcept
{
    const int tx { toIntClamped(x) };
    const int ty { toIntClamped(y) };

    // Hybrid folding chassis with energy discharge
    const SDL_Rect hybridRect { tx + 2, ty + 4, 20, 18 };
    (void)SDL_SetRenderDrawColor(renderer_, 90U, 150U, 230U, 255U);
    (void)SDL_RenderFillRect(renderer_, &hybridRect);

    (void)SDL_SetRenderDrawColor(renderer_, 255U, 255U, 100U, 255U);
    (void)SDL_RenderDrawLine(renderer_, tx + 4, ty + 6, tx + 18, ty + 20);
    (void)SDL_RenderDrawLine(renderer_, tx + 18, ty + 6, tx + 4, ty + 20);

    (void)facing;
}

void SdlRenderer::drawEnemyPatrol(float x, float y, ShooterCore::FacingDirection facing) noexcept
{
    const int ex { toIntClamped(x) };
    const int ey { toIntClamped(y) };

    // Hostile red walker mech
    const SDL_Rect bodyRect { ex + 2, ey + 2, 12, 12 };
    (void)SDL_SetRenderDrawColor(renderer_, 200U, 40U, 50U, 255U);
    (void)SDL_RenderFillRect(renderer_, &bodyRect);

    // Sinister optic eye
    (void)SDL_SetRenderDrawColor(renderer_, 255U, 230U, 60U, 255U);
    const int eyeX { (facing == ShooterCore::FacingDirection::Left) ? ex + 3 : ex + 10 };
    (void)SDL_RenderDrawPoint(renderer_, eyeX, ey + 6);
    (void)SDL_RenderDrawPoint(renderer_, eyeX + 1, ey + 6);

    // Mechanical walking struts
    (void)SDL_SetRenderDrawColor(renderer_, 110U, 20U, 30U, 255U);
    (void)SDL_RenderDrawLine(renderer_, ex + 4, ey + 14, ex + 2, ey + 16);
    (void)SDL_RenderDrawLine(renderer_, ex + 11, ey + 14, ex + 13, ey + 16);
}

void SdlRenderer::drawEnemyFlyer(float x, float y) noexcept
{
    const int fx { toIntClamped(x) };
    const int fy { toIntClamped(y) };

    // Diamond shaped airborne scout
    (void)SDL_SetRenderDrawColor(renderer_, 160U, 50U, 200U, 255U);
    (void)SDL_RenderDrawLine(renderer_, fx + 8, fy, fx + 15, fy + 8);
    (void)SDL_RenderDrawLine(renderer_, fx + 15, fy + 8, fx + 8, fy + 15);
    (void)SDL_RenderDrawLine(renderer_, fx + 8, fy + 15, fx, fy + 8);
    (void)SDL_RenderDrawLine(renderer_, fx, fy + 8, fx + 8, fy);

    // Inner scanning nucleus
    (void)SDL_SetRenderDrawColor(renderer_, 60U, 255U, 220U, 255U);
    const SDL_Rect coreRect { fx + 6, fy + 6, 4, 4 };
    (void)SDL_RenderFillRect(renderer_, &coreRect);
}

void SdlRenderer::drawExplosion(float x, float y, float size) noexcept
{
    const int cx { toIntClamped(x) };
    const int cy { toIntClamped(y) };
    const int radius { static_cast<int>(size * 0.5f) };

    // Expanding concentric shockwave bursts
    (void)SDL_SetRenderDrawColor(renderer_, 255U, 200U, 50U, 255U);
    (void)SDL_RenderDrawLine(renderer_, cx - radius, cy, cx + radius, cy);
    (void)SDL_RenderDrawLine(renderer_, cx, cy - radius, cx, cy + radius);

    (void)SDL_SetRenderDrawColor(renderer_, 255U, 80U, 20U, 255U);
    const int rHalf { radius / 2 };
    (void)SDL_RenderDrawLine(renderer_, cx - rHalf, cy - rHalf, cx + rHalf, cy + rHalf);
    (void)SDL_RenderDrawLine(renderer_, cx - rHalf, cy + rHalf, cx + rHalf, cy - rHalf);
}

void SdlRenderer::drawBeam(const ShooterCore::RenderCommand& cmd) noexcept
{
    const int x0 { toIntClamped(cmd.pos.x) };
    const int y0 { toIntClamped(cmd.pos.y) };
    const int x1 { toIntClamped(cmd.endpoint.x) };
    const int y1 { toIntClamped(cmd.endpoint.y) };

    (void)SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

    // Pass 1: Outer glowing halo (alpha blended)
    const std::uint8_t haloAlpha { static_cast<std::uint8_t>(cmd.color.a / 2U) };
    (void)SDL_SetRenderDrawColor(renderer_, cmd.color.r, cmd.color.g, cmd.color.b, haloAlpha);
    (void)SDL_RenderDrawLine(renderer_, x0 - 1, y0, x1 - 1, y1);
    (void)SDL_RenderDrawLine(renderer_, x0 + 1, y0, x1 + 1, y1);
    (void)SDL_RenderDrawLine(renderer_, x0, y0 - 1, x1, y1 - 1);
    (void)SDL_RenderDrawLine(renderer_, x0, y0 + 1, x1, y1 + 1);

    // Pass 2: High intensity pure white core
    (void)SDL_SetRenderDrawColor(renderer_, 255U, 255U, 255U, 255U);
    (void)SDL_RenderDrawLine(renderer_, x0, y0, x1, y1);

    // Pass 3: Target impact diamond reticle
    (void)SDL_SetRenderDrawColor(renderer_, 255U, 240U, 80U, 255U);
    (void)SDL_RenderDrawLine(renderer_, x1 - 3, y1, x1, y1 - 3);
    (void)SDL_RenderDrawLine(renderer_, x1, y1 - 3, x1 + 3, y1);
    (void)SDL_RenderDrawLine(renderer_, x1 + 3, y1, x1, y1 + 3);
    (void)SDL_RenderDrawLine(renderer_, x1, y1 + 3, x1 - 3, y1);

    (void)SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
}

void SdlRenderer::drawBar(const ShooterCore::RenderCommand& cmd) noexcept
{
    const int bx { toIntClamped(cmd.pos.x) };
    const int by { toIntClamped(cmd.pos.y) };
    const int bw { toIntClamped(cmd.width) };
    const int bh { toIntClamped(cmd.height) };

    if (bw <= 0 || bh <= 0) {
        return;
    }

    const SDL_Rect outerRect { bx, by, bw, bh };

    // Dark inset background
    (void)SDL_SetRenderDrawColor(renderer_, 20U, 25U, 35U, 255U);
    (void)SDL_RenderFillRect(renderer_, &outerRect);

    // Filled foreground portion
    const float fillClamped { std::min(std::max(cmd.fillPct, 0.0f), 1.0f) };
    const int fillWidth { static_cast<int>(static_cast<float>(bw - 2) * fillClamped) };

    if (fillWidth > 0) {
        const SDL_Rect fillRect { bx + 1, by + 1, fillWidth, bh - 2 };
        (void)SDL_SetRenderDrawColor(renderer_, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a);
        (void)SDL_RenderFillRect(renderer_, &fillRect);
    }

    // Border outline
    (void)SDL_SetRenderDrawColor(renderer_, 140U, 150U, 175U, 255U);
    (void)SDL_RenderDrawRect(renderer_, &outerRect);
}

void SdlRenderer::drawLightning(const ShooterCore::RenderCommand& cmd) noexcept
{
    const ShooterCore::LightningArc& arc { activeQueue_ ? activeQueue_->getLightning()
                                                        : ShooterCore::LightningArc { } };
    if (arc.nodeCount < 2U) {
        return;
    }

    (void)SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_ADD);

    // Pass 1: Outer glowing corona (additive, wide 5px)
    const ShooterCore::ColorRgba& glow { cmd.glowColor };
    (void)SDL_SetRenderDrawColor(renderer_, glow.r, glow.g, glow.b, glow.a);

    for (std::size_t i { 0U }; i < (arc.nodeCount - 1U); ++i) {
        const int x0 { toIntClamped(arc.nodes[i].x) };
        const int y0 { toIntClamped(arc.nodes[i].y) };
        const int x1 { toIntClamped(arc.nodes[i + 1U].x) };
        const int y1 { toIntClamped(arc.nodes[i + 1U].y) };

        (void)SDL_RenderDrawLine(renderer_, x0 - 2, y0, x1 - 2, y1);
        (void)SDL_RenderDrawLine(renderer_, x0 + 2, y0, x1 + 2, y1);
        (void)SDL_RenderDrawLine(renderer_, x0, y0 - 2, x1, y1 - 2);
        (void)SDL_RenderDrawLine(renderer_, x0, y0 + 2, x1, y1 + 2);
        (void)SDL_RenderDrawLine(renderer_, x0 - 1, y0 - 1, x1 - 1, y1 - 1);
        (void)SDL_RenderDrawLine(renderer_, x0 + 1, y0 + 1, x1 + 1, y1 + 1);
    }

    // Pass 2: High-intensity plasma core (additive, 3px)
    (void)SDL_SetRenderDrawColor(renderer_, 120U, 220U, 255U, 220U);
    for (std::size_t i { 0U }; i < (arc.nodeCount - 1U); ++i) {
        const int x0 { toIntClamped(arc.nodes[i].x) };
        const int y0 { toIntClamped(arc.nodes[i].y) };
        const int x1 { toIntClamped(arc.nodes[i + 1U].x) };
        const int y1 { toIntClamped(arc.nodes[i + 1U].y) };

        (void)SDL_RenderDrawLine(renderer_, x0 - 1, y0, x1 - 1, y1);
        (void)SDL_RenderDrawLine(renderer_, x0 + 1, y0, x1 + 1, y1);
        (void)SDL_RenderDrawLine(renderer_, x0, y0 - 1, x1, y1 - 1);
        (void)SDL_RenderDrawLine(renderer_, x0, y0 + 1, x1, y1 + 1);
    }

    // Pass 3: Pure white central filament spine (1px)
    (void)SDL_SetRenderDrawColor(renderer_, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a);
    for (std::size_t i { 0U }; i < (arc.nodeCount - 1U); ++i) {
        const int x0 { toIntClamped(arc.nodes[i].x) };
        const int y0 { toIntClamped(arc.nodes[i].y) };
        const int x1 { toIntClamped(arc.nodes[i + 1U].x) };
        const int y1 { toIntClamped(arc.nodes[i + 1U].y) };

        (void)SDL_RenderDrawLine(renderer_, x0, y0, x1, y1);
    }

    // Pass 4: Impact reticle if locked to target
    if (arc.lockedToTarget) {
        const int tx { toIntClamped(arc.nodes[arc.nodeCount - 1U].x) };
        const int ty { toIntClamped(arc.nodes[arc.nodeCount - 1U].y) };

        (void)SDL_SetRenderDrawColor(renderer_, 255U, 240U, 90U, 255U);
        (void)SDL_RenderDrawLine(renderer_, tx - 4, ty, tx + 4, ty);
        (void)SDL_RenderDrawLine(renderer_, tx, ty - 4, tx, ty + 4);
        (void)SDL_RenderDrawLine(renderer_, tx - 3, ty, tx, ty - 3);
        (void)SDL_RenderDrawLine(renderer_, tx, ty - 3, tx + 3, ty);
        (void)SDL_RenderDrawLine(renderer_, tx + 3, ty, tx, ty + 3);
        (void)SDL_RenderDrawLine(renderer_, tx, ty + 3, tx - 3, ty);
    }

    (void)SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
}

void SdlRenderer::drawShieldBubble(const ShooterCore::RenderCommand& cmd) noexcept
{
    const ShooterCore::ShieldFxState& shieldFx { activeQueue_ ? activeQueue_->getShieldFx()
                                                              : ShooterCore::ShieldFxState { } };
    const float cx { cmd.pos.x + (cmd.width * 0.5f) };
    const float cy { cmd.pos.y + (cmd.height * 0.5f) };
    const float baseRadius { (std::max(cmd.width, cmd.height) * 0.55f) + shieldFx.breathRadius };

    (void)SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

    constexpr std::size_t kSides { 12U };
    constexpr float kStepAngle { 6.2831853f / static_cast<float>(kSides) };

    int vx[kSides] { };
    int vy[kSides] { };

    for (std::size_t i { 0U }; i < kSides; ++i) {
        const auto angle { static_cast<float>(i) * kStepAngle };
        vx[i] = toIntClamped(cx + (baseRadius * std::cos(angle)));
        vy[i] = toIntClamped(cy + (baseRadius * std::sin(angle)));
    }

    // Outer contour
    if (shieldFx.isLowEnergy) {
        (void)SDL_SetRenderDrawColor(renderer_, 255U, 60U, 40U, 220U);
    } else {
        (void)SDL_SetRenderDrawColor(renderer_, 40U, 190U, 255U, 200U);
    }

    for (std::size_t i { 0U }; i < kSides; ++i) {
        const std::size_t next { (i + 1U) % kSides };
        (void)SDL_RenderDrawLine(renderer_, vx[i], vy[i], vx[next], vy[next]);
    }

    // Internal translucent cross-facets
    if (shieldFx.isLowEnergy) {
        (void)SDL_SetRenderDrawColor(renderer_, 200U, 40U, 30U, 60U);
    } else {
        (void)SDL_SetRenderDrawColor(renderer_, 30U, 120U, 220U, 70U);
    }

    for (std::size_t i { 0U }; i < (kSides / 2U); ++i) {
        const std::size_t opp { i + (kSides / 2U) };
        (void)SDL_RenderDrawLine(renderer_, vx[i], vy[i], vx[opp], vy[opp]);
    }

    // Dynamic deflection ripple ring
    if (shieldFx.hitIntensity > 0.05f) {
        const auto rippleAlpha { static_cast<std::uint8_t>(255.0f * shieldFx.hitIntensity) };
        (void)SDL_SetRenderDrawColor(renderer_, 255U, 255U, 255U, rippleAlpha);

        const float rippleRad { baseRadius + (6.0f * (1.0f - shieldFx.hitIntensity)) };
        for (std::size_t i { 0U }; i < kSides; ++i) {
            const auto angle0 { static_cast<float>(i) * kStepAngle };
            const auto angle1 { static_cast<float>((i + 1U) % kSides) * kStepAngle };

            const int rx0 { toIntClamped(cx + (rippleRad * std::cos(angle0))) };
            const int ry0 { toIntClamped(cy + (rippleRad * std::sin(angle0))) };
            const int rx1 { toIntClamped(cx + (rippleRad * std::cos(angle1))) };
            const int ry1 { toIntClamped(cy + (rippleRad * std::sin(angle1))) };

            (void)SDL_RenderDrawLine(renderer_, rx0, ry0, rx1, ry1);
        }
    }

    (void)SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
}

void SdlRenderer::drawParticles(const ShooterCore::RenderCommand& cmd) noexcept
{
    const int px { toIntClamped(cmd.pos.x) };
    const int py { toIntClamped(cmd.pos.y) };
    const int size { std::max(1, toIntClamped(cmd.width)) };

    (void)SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    (void)SDL_SetRenderDrawColor(renderer_, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a);

    if (size <= 1) {
        (void)SDL_RenderDrawPoint(renderer_, px, py);
    } else {
        const SDL_Rect r { px, py, size, size };
        (void)SDL_RenderFillRect(renderer_, &r);
    }

    (void)SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
}

void SdlRenderer::drawShockwave(const ShooterCore::RenderCommand& cmd) noexcept
{
    if (renderer_ == nullptr) {
        return;
    }

    const int cx { toIntClamped(cmd.pos.x) };
    const int cy { toIntClamped(cmd.pos.y) };
    const float radius { cmd.width * 0.5f };

    (void)SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_ADD);
    (void)SDL_SetRenderDrawColor(renderer_, cmd.color.r, cmd.color.g, cmd.color.b, cmd.color.a);

    constexpr std::size_t kSegments { 16U };
    constexpr float kStep { 6.2831853f / static_cast<float>(kSegments) };

    for (std::size_t s { 0U }; s < kSegments; ++s) {
        const float a1 { static_cast<float>(s) * kStep };
        const float a2 { static_cast<float>(s + 1U) * kStep };
        const int x1 { cx + toIntClamped(std::cos(a1) * radius) };
        const int y1 { cy + toIntClamped(std::sin(a1) * radius) };
        const int x2 { cx + toIntClamped(std::cos(a2) * radius) };
        const int y2 { cy + toIntClamped(std::sin(a2) * radius) };
        (void)SDL_RenderDrawLine(renderer_, x1, y1, x2, y2);
    }

    (void)SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
}

} // namespace SdlAdapter
