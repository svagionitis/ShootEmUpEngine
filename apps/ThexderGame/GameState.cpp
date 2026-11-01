#include "GameState.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>

#include "EngineInput.h"
#include "EntitySpawner.h"
#include "GameEngine.h"
#include "Level1Map.h"
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

    // Clear map completely of any background/obstacles
    for (std::uint32_t r { 0U }; r < map_.height(); ++r) {
        for (std::uint32_t c { 0U }; c < map_.width(); ++c) {
            map_.setTile(ShooterCore::TileCoord { c, r }, ShooterCore::TileType::Empty);
        }
    }

    // Set a solid floor row across the map so the robot can stand, walk, and jump without falling into void
    constexpr std::uint32_t kFloorRow { 10U };
    for (std::uint32_t c { 0U }; c < map_.width(); ++c) {
        map_.setTile(ShooterCore::TileCoord { c, kFloorRow }, ShooterCore::TileType::SolidWall);
    }

    // Position player centered in the screen on the floor
    const ShooterCore::WorldPos spawnPos { 152.0f, static_cast<float>(kFloorRow) * 16.0f };
    exitPortal_ = ShooterCore::WorldPos { 2000.0f, 2000.0f };

    engine_.initLevel(map_, spawnPos);

    updateCamera(engine_.getPlayerPos());
}

void GameState::step(ShooterCore::DeltaTime dt, ShooterCore::EngineInput input) noexcept
{
    const float dtSec { dt.seconds() };
    warningTimer_ += dtSec;

    switch (mode_) {
    case GameStateMode::Title:
        resetGame();
        mode_ = GameStateMode::Playing;
        [[fallthrough]];

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
        const float ex { ppos.x - exitPortal_.x };
        const float ey { ppos.y - exitPortal_.y };
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

    // Isolated robot diagnostic mode: remove intro and all background, leave only the robot
    const ShooterCore::WorldPos effectiveCam { screenShakeEnabled_ ? engine_.getCameraSystem().effectivePosition()
                                                                   : camera_.pos };

    const ShooterCore::RenderCommandQueue& wq { engine_.getRenderQueue() };
    const std::size_t wcmdCount { wq.count() };

    for (std::size_t i { 0U }; i < wcmdCount; ++i) {
        const ShooterCore::RenderCommand& cmd { wq.get(i) };

        // Only emit player robot craft sprites (layers 9..10), beam, and shield
        if (cmd.type == ShooterCore::RenderCommandType::DrawSprite) {
            if (cmd.layer >= 9U && cmd.layer <= 10U) {
                const float sx { cmd.pos.x - effectiveCam.x };
                const float sy { cmd.pos.y - effectiveCam.y };
                if (sx >= -48.0f && sx <= kViewWidth + 32.0f && sy >= -48.0f && sy <= kViewHeight + 32.0f) {
                    (void)outQueue.pushSprite(
                        cmd.sprite, ShooterCore::WorldPos { sx, sy }, cmd.facing, cmd.layer, cmd.rotationDegrees);
                }
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
}

} // namespace ThexderGame
