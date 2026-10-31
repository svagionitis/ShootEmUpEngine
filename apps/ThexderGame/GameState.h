#pragma once

#include <cstddef>
#include <cstdint>

#include "EngineInput.h"
#include "EntitySpawner.h"
#include "GameEngine.h"
#include "HudOverlay.h"
#include "RenderCommandQueue.h"
#include "TileMap.h"
#include "Types.h"

namespace ThexderGame {

/// @enum GameStateMode
/// @brief Master lifecycle state of the game application.
enum class GameStateMode : std::uint8_t { Title = 0U, Playing = 1U, Paused = 2U, GameOver = 3U, LevelComplete = 4U };

/// @class GameState
/// @brief Master state controller coordinating engine, spawners, HUD, and viewport tracking.
/// @details Maintains zero dynamic heap allocations on the simulation and render path (CERT ERR55-CPP).
class GameState final {
public:
    GameState() noexcept;

    /// @brief Initialize new game session with Level 1 environment.
    void init() noexcept;

    /// @brief Reset game to beginning of Level 1.
    void resetGame() noexcept;

    /// @brief Advance simulation step based on current state mode.
    /// @param[in] dt Timestep duration.
    /// @param[in] input Player input bitmask.
    void step(ShooterCore::DeltaTime dt, ShooterCore::EngineInput input) noexcept;

    /// @brief Populate render queue with translated screen-space draw commands and HUD overlay.
    /// @param[out] outQueue Destination command queue to dispatch to SdlRenderer.
    void buildRenderQueue(ShooterCore::RenderCommandQueue& outQueue) noexcept;

    /// @brief Query active game state mode.
    [[nodiscard]] constexpr GameStateMode getMode() const noexcept
    {
        return mode_;
    }

    /// @brief Set active game state mode.
    constexpr void setMode(GameStateMode m) noexcept
    {
        mode_ = m;
    }

    /// @brief Query player accumulated score.
    [[nodiscard]] constexpr std::uint32_t getScore() const noexcept
    {
        return score_;
    }

    /// @brief Query session high score.
    [[nodiscard]] constexpr std::uint32_t getHiScore() const noexcept
    {
        return hiScore_;
    }

    /// @brief Set player energy level directly (for testing and pickups).
    /// @param[in] pts Energy points value.
    void setPlayerEnergy(ShooterCore::EnergyPoints pts) noexcept
    {
        engine_.setPlayerEnergy(pts);
    }

    /// @brief Enable or disable god mode invulnerability.
    constexpr void setGodMode(bool enable) noexcept
    {
        godMode_ = enable;
    }

    /// @brief Query whether god mode is active.
    [[nodiscard]] constexpr bool isGodMode() const noexcept
    {
        return godMode_;
    }

    /// @brief Enable or disable screen shake effect.
    constexpr void setScreenShakeEnabled(bool enable) noexcept
    {
        screenShakeEnabled_ = enable;
    }

    /// @brief Toggle screen shake effect state.
    constexpr void toggleScreenShake() noexcept
    {
        screenShakeEnabled_ = !screenShakeEnabled_;
    }

    /// @brief Query whether screen shake is enabled.
    [[nodiscard]] constexpr bool isScreenShakeEnabled() const noexcept
    {
        return screenShakeEnabled_;
    }

    /// @brief Query viewport camera.
    [[nodiscard]] constexpr ShooterCore::CameraViewport getCamera() const noexcept
    {
        return camera_;
    }

    /// @brief Retrieve underlying core simulation engine.
    [[nodiscard]] const ShooterCore::GameEngine& getEngine() const noexcept
    {
        return engine_;
    }

    /// @brief Retrieve underlying entity spawner.
    [[nodiscard]] const EntitySpawner& getSpawner() const noexcept
    {
        return spawner_;
    }

private:
    void updateCamera(ShooterCore::WorldPos playerPos) noexcept;

    ShooterCore::GameEngine engine_ { };
    ShooterCore::TileMap map_ { 160U, 32U };
    EntitySpawner spawner_ { };
    HudOverlay hud_ { };
    ShooterCore::CameraViewport camera_ { };

    GameStateMode mode_ { GameStateMode::Title };
    std::uint32_t score_ { 0U };
    std::uint32_t hiScore_ { 50000U };
    float stateTimer_ { 0.0f };
    float warningTimer_ { 0.0f };
    bool godMode_ { false };
    bool screenShakeEnabled_ { true };
    bool pausePressedPrev_ { false };
};

} // namespace ThexderGame
