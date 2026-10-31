#include "GameState.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>

#include "EgaBugLoader.h"
#include "EngineInput.h"
#include "EntitySpawner.h"
#include "GameEngine.h"
#include "HudOverlay.h"
#include "Level1Map.h"
#include "ProceduralSprites.h"
#include "RenderCommandQueue.h"
#include "TileMap.h"
#include "Types.h"

namespace ThexderGame {

namespace {
    constexpr float kViewWidth { 320.0f };
    constexpr float kViewHeight { 240.0f };
    constexpr float kMapPixelW { static_cast<float>(Level1Map::kMapWidth) * Level1Map::kTileSize };
    constexpr float kMapPixelH { static_cast<float>(Level1Map::kMapHeight) * Level1Map::kTileSize };
    constexpr float kMaxCamX { kMapPixelW - kViewWidth };
    constexpr float kMaxCamY { kMapPixelH - kViewHeight };
} // namespace

GameState::GameState() noexcept
{
    init();
}

void GameState::init() noexcept
{
    camera_.pos = ShooterCore::WorldPos { 0.0f, 0.0f };
    camera_.width = kViewWidth;
    camera_.height = kViewHeight;

    resetGame();
    mode_ = GameStateMode::Title;
}

void GameState::resetGame() noexcept
{
    score_ = 0U;
    stateTimer_ = 0.0f;
    warningTimer_ = 0.0f;

    Level1Map::loadFromBinary(map_);
    engine_.initLevel(map_, Level1Map::getPlayerSpawn());
    spawner_.initLevel1Nests();

    // Register authentic alien nests from BUGDB01.BIN with fallback
    auto& nestMgr { engine_.getNestManager() };
    const std::size_t loadedNests { SdlAdapter::EgaBugLoader::loadBugDb("assets/thexder/BUGDB01.BIN", nestMgr) };
    if (loadedNests == 0U) {
        nestMgr.addNest(ShooterCore::WorldPos { 280.0f, 140.0f }, 80U, 4.0f);
        nestMgr.addNest(ShooterCore::WorldPos { 600.0f, 100.0f }, 80U, 3.5f);
        nestMgr.addNest(ShooterCore::WorldPos { 900.0f, 180.0f }, 80U, 3.0f);
        nestMgr.addNest(ShooterCore::WorldPos { 1200.0f, 120.0f }, 80U, 2.5f);
    }

    // Seed Tactical Ceiling Turrets and Wall Crawlers
    engine_.spawnEnemy(ShooterCore::EntityType::CeilingTurret, ShooterCore::WorldPos { 400.0f, 32.0f },
        ShooterCore::Hitbox { ShooterCore::WorldPos { 392.0f, 28.0f }, ShooterCore::WorldPos { 408.0f, 44.0f } }, 30U);
    engine_.spawnEnemy(ShooterCore::EntityType::CeilingTurret, ShooterCore::WorldPos { 800.0f, 32.0f },
        ShooterCore::Hitbox { ShooterCore::WorldPos { 792.0f, 28.0f }, ShooterCore::WorldPos { 808.0f, 44.0f } }, 30U);

    engine_.spawnEnemy(ShooterCore::EntityType::WallCrawler, ShooterCore::WorldPos { 350.0f, 192.0f },
        ShooterCore::Hitbox { ShooterCore::WorldPos { 342.0f, 184.0f }, ShooterCore::WorldPos { 358.0f, 200.0f } },
        20U);

    updateCamera(engine_.getPlayerPos());
}

void GameState::step(ShooterCore::DeltaTime dt, ShooterCore::EngineInput input) noexcept
{
    const float dtSec { dt.seconds() };
    warningTimer_ += dtSec;

    switch (mode_) {
    case GameStateMode::Title:
        if (input.isFire() || input.isJump()) {
            resetGame();
            mode_ = GameStateMode::Playing;
        }
        break;

    case GameStateMode::Playing: {
        engine_.step(dt, input);
        spawner_.updateSpawners(dt, engine_.getPlayerPos(), engine_.getEntityPool());

        // Laser damage interaction with generator nests
        const ShooterCore::BeamSegment beam { engine_.getWeaponSystem().getBeamSegment() };
        if (beam.active) {
            for (std::size_t i { 0U }; i < spawner_.nestCount(); ++i) {
                const SpawnerNode& nest { spawner_.getNest(i) };
                if (nest.destroyed) {
                    continue;
                }
                const float dx { beam.endpoint.x - nest.pos.x };
                const float dy { beam.endpoint.y - nest.pos.y };
                if ((dx * dx) + (dy * dy) <= 18.0f * 18.0f) {
                    const bool destroyed { spawner_.damageNest(i, ShooterCore::DamageValue { 2U }) };
                    if (destroyed) {
                        score_ += 1000U;
                    }
                }
            }
        }

        // Check level completion exit portal
        const ShooterCore::WorldPos ppos { engine_.getPlayerPos() };
        const ShooterCore::WorldPos eport { Level1Map::getExitPortal() };
        const float ex { ppos.x - eport.x };
        const float ey { ppos.y - eport.y };
        if (((ex * ex) + (ey * ey)) <= 28.0f * 28.0f) {
            mode_ = GameStateMode::LevelComplete;
            score_ += static_cast<std::uint32_t>(engine_.getPlayerEnergy().value()) * 100U;
            stateTimer_ = 0.0f;
        }

        // Check game over
        if (!engine_.isPlayerAlive() || engine_.getPlayerEnergy().isDepleted()) {
            if (!godMode_) {
                mode_ = GameStateMode::GameOver;
                stateTimer_ = 0.0f;
            }
        }

        if (score_ > hiScore_) {
            hiScore_ = score_;
        }

        updateCamera(engine_.getPlayerPos());
        break;
    }

    case GameStateMode::Paused:
        // Simulation frozen
        break;

    case GameStateMode::GameOver:
        stateTimer_ += dtSec;
        if (stateTimer_ >= 3.0f) {
            mode_ = GameStateMode::Title;
        }
        break;

    case GameStateMode::LevelComplete:
        stateTimer_ += dtSec;
        if (stateTimer_ >= 4.0f) {
            mode_ = GameStateMode::Title;
        }
        break;
    }
}

void GameState::updateCamera(ShooterCore::WorldPos playerPos) noexcept
{
    // Soft center deadzone: 64 px horizontal, 48 px vertical
    const float scrX { playerPos.x - camera_.pos.x };
    const float scrY { playerPos.y - camera_.pos.y };

    if (scrX < 128.0f) {
        camera_.pos.x = playerPos.x - 128.0f;
    } else if (scrX > 192.0f) {
        camera_.pos.x = playerPos.x - 192.0f;
    }

    if (scrY < 96.0f) {
        camera_.pos.y = playerPos.y - 96.0f;
    } else if (scrY > 144.0f) {
        camera_.pos.y = playerPos.y - 144.0f;
    }

    camera_.pos.x = std::clamp(camera_.pos.x, 0.0f, kMaxCamX);
    camera_.pos.y = std::clamp(camera_.pos.y, 0.0f, kMaxCamY);
}

void GameState::buildRenderQueue(ShooterCore::RenderCommandQueue& outQueue) noexcept
{
    outQueue.clear();

    if (mode_ == GameStateMode::Title) {
        ProceduralSprites::drawTitleLogo(outQueue, 50.0f, 30.0f);
        HudOverlay::drawText(outQueue, "THEXDER", 112.0f, 44.0f, 2U, ShooterCore::ColorRgba { 255U, 215U, 0U, 255U });
        HudOverlay::drawText(
            outQueue, "LEVEL 1 - CAVERN SYSTEM", 76.0f, 96.0f, 1U, ShooterCore::ColorRgba { 40U, 220U, 255U, 255U });

        HudOverlay::drawText(
            outQueue, "PILOT CONTROLS:", 60.0f, 120.0f, 1U, ShooterCore::ColorRgba { 255U, 215U, 0U, 255U });
        HudOverlay::drawText(
            outQueue, "ARROWS: MOVE / FLY JET", 70.0f, 134.0f, 1U, ShooterCore::ColorRgba { 230U, 235U, 255U, 255U });
        HudOverlay::drawText(outQueue, "SPACE:  JUMP / JET THRUST", 70.0f, 146.0f, 1U,
            ShooterCore::ColorRgba { 230U, 235U, 255U, 255U });
        HudOverlay::drawText(
            outQueue, "X:      AUTO-AIM LASER", 70.0f, 158.0f, 1U, ShooterCore::ColorRgba { 230U, 235U, 255U, 255U });
        HudOverlay::drawText(
            outQueue, "C:      SHIELD BARRIER", 70.0f, 170.0f, 1U, ShooterCore::ColorRgba { 230U, 235U, 255U, 255U });
        HudOverlay::drawText(outQueue, "TAB:    TRANSFORM ROBOT/JET", 70.0f, 182.0f, 1U,
            ShooterCore::ColorRgba { 230U, 235U, 255U, 255U });

        const auto blink { static_cast<std::uint32_t>(warningTimer_ * 2.0f) };
        if ((blink % 2U) == 0U) {
            HudOverlay::drawText(
                outQueue, "PRESS FIRE TO COMMENCE", 70.0f, 210.0f, 1U, ShooterCore::ColorRgba { 255U, 70U, 60U, 255U });
        }
        return;
    }

    // 1. World draw commands translated to screen coordinates with camera trauma shake
    const ShooterCore::WorldPos effectiveCam { screenShakeEnabled_ ? engine_.getCameraSystem().effectivePosition()
                                                                   : camera_.pos };

    const ShooterCore::RenderCommandQueue& wq { engine_.getRenderQueue() };
    const std::size_t wcmdCount { wq.count() };

    for (std::size_t i { 0U }; i < wcmdCount; ++i) {
        const ShooterCore::RenderCommand& cmd { wq.get(i) };

        if (cmd.type == ShooterCore::RenderCommandType::DrawTile) {
            const float sx { cmd.pos.x - effectiveCam.x };
            const float sy { cmd.pos.y - effectiveCam.y };
            if (sx >= -16.0f && sx <= kViewWidth && sy >= -16.0f && sy <= kViewHeight) {
                (void)outQueue.pushTile(cmd.tileType, ShooterCore::WorldPos { sx, sy }, cmd.layer);
            }
        } else if (cmd.type == ShooterCore::RenderCommandType::DrawSprite) {
            const float sx { cmd.pos.x - effectiveCam.x };
            const float sy { cmd.pos.y - effectiveCam.y };
            if (sx >= -48.0f && sx <= kViewWidth + 32.0f && sy >= -48.0f && sy <= kViewHeight + 32.0f) {
                (void)outQueue.pushSprite(
                    cmd.sprite, ShooterCore::WorldPos { sx, sy }, cmd.facing, cmd.layer, cmd.rotationDegrees);
            }
        } else if (cmd.type == ShooterCore::RenderCommandType::DrawBeam) {
            const float sx0 { cmd.pos.x - effectiveCam.x };
            const float sy0 { cmd.pos.y - effectiveCam.y };
            const float sx1 { cmd.endpoint.x - effectiveCam.x };
            const float sy1 { cmd.endpoint.y - effectiveCam.y };
            (void)outQueue.pushBeam(ShooterCore::WorldPos { sx0, sy0 }, ShooterCore::WorldPos { sx1, sy1 }, cmd.color);
        } else if (cmd.type == ShooterCore::RenderCommandType::DrawBar) {
            // Shield barrier has height > 10
            if (cmd.height > 10.0f) {
                const float sx { cmd.pos.x - effectiveCam.x };
                const float sy { cmd.pos.y - effectiveCam.y };
                (void)outQueue.pushBar(ShooterCore::WorldPos { sx, sy }, cmd.width, cmd.height, cmd.fillPct, cmd.color);
            }
        }
    }

    // 2. Generator nests
    for (std::size_t i { 0U }; i < spawner_.nestCount(); ++i) {
        const SpawnerNode& nest { spawner_.getNest(i) };
        const float sx { nest.pos.x - effectiveCam.x };
        const float sy { nest.pos.y - effectiveCam.y };
        if (sx >= -32.0f && sx <= kViewWidth && sy >= -32.0f && sy <= kViewHeight) {
            const ShooterCore::SpriteId nestSpr { nest.destroyed ? ShooterCore::SpriteId::NestCoreDestroyed
                                                                 : ShooterCore::SpriteId::NestCoreActive };
            (void)outQueue.pushSprite(
                nestSpr, ShooterCore::WorldPos { sx, sy }, ShooterCore::FacingDirection::Right, 2U);
        }
    }

    // 3. Extraction Gate / Exit Portal
    const ShooterCore::WorldPos eport { Level1Map::getExitPortal() };
    const float esx { eport.x - effectiveCam.x };
    const float esy { eport.y - effectiveCam.y };
    if (esx >= -32.0f && esx <= kViewWidth && esy >= -48.0f && esy <= kViewHeight) {
        outQueue.pushBar(ShooterCore::WorldPos { esx, esy - 16.0f }, 16.0f, 32.0f, 1.0f,
            ShooterCore::ColorRgba { 50U, 240U, 120U, 200U });
    }

    // 4. State overlays
    if (mode_ == GameStateMode::Paused) {
        outQueue.pushBar(
            ShooterCore::WorldPos { 0.0f, 0.0f }, 320.0f, 240.0f, 1.0f, ShooterCore::ColorRgba { 0U, 0U, 0U, 140U });
        HudOverlay::drawText(outQueue, "PAUSED", 136.0f, 114.0f, 1U, ShooterCore::ColorRgba { 255U, 215U, 0U, 255U });
    } else if (mode_ == GameStateMode::GameOver) {
        ProceduralSprites::drawGameOver(outQueue, 70.0f, 102.0f);
        HudOverlay::drawText(
            outQueue, "MISSION FAILED", 100.0f, 110.0f, 1U, ShooterCore::ColorRgba { 255U, 60U, 60U, 255U });
        HudOverlay::drawText(
            outQueue, "ENERGY DEPLETED", 94.0f, 122.0f, 1U, ShooterCore::ColorRgba { 255U, 200U, 200U, 255U });
    } else if (mode_ == GameStateMode::LevelComplete) {
        ProceduralSprites::drawVictory(outQueue, 60.0f, 100.0f);
        HudOverlay::drawText(
            outQueue, "LEVEL 1 CLEARED!", 96.0f, 110.0f, 1U, ShooterCore::ColorRgba { 255U, 215U, 0U, 255U });
        HudOverlay::drawText(
            outQueue, "EXTRACTION COMPLETE", 80.0f, 122.0f, 1U, ShooterCore::ColorRgba { 255U, 255U, 255U, 255U });
    }

    // 5. Screen-space HUD overlay
    const ShooterCore::ShieldGauge shieldGauge { static_cast<std::uint8_t>(
        engine_.getWeaponSystem().isShieldActive() ? 100U : 0U) };
    const auto& arsenal { engine_.getSecondaryArsenal() };
    const std::uint8_t secAmmo { (arsenal.activeWeapon() == ShooterCore::SecondaryWeaponType::HomingMissile)
            ? arsenal.missileAmmo().count()
            : arsenal.bombAmmo().count() };

    hud_.renderHud(outQueue, engine_.getPlayerEnergy(), shieldGauge, score_, hiScore_, engine_.getPlayerMode(),
        warningTimer_, arsenal.activeWeapon(), secAmmo);
}

} // namespace ThexderGame
