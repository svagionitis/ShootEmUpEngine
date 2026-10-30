#pragma once

#include <cstddef>
#include <cstdint>

#include <array>

#include "AudioCommandQueue.h"
#include "CollisionSolver.h"
#include "EngineInput.h"
#include "EntityPool.h"
#include "Hitbox.h"
#include "RenderCommandQueue.h"
#include "TileMap.h"
#include "TransformSystem.h"
#include "Types.h"
#include "WeaponSystem.h"

namespace ShooterCore {

/// @class GameEngine
/// @brief Master simulation engine coordinating physics, combat, transform, and command emission.
/// @details Maintains zero dynamic heap allocations on the simulation path (CERT ERR55-CPP).
class GameEngine final {
public:
    static constexpr float kTileSize { 16.0f };
    static constexpr float kGravity { 980.0f };
    static constexpr float kJumpSpeed { -320.0f };
    static constexpr float kWalkSpeed { 100.0f };
    static constexpr float kJetThrust { 400.0f };
    static constexpr float kJetMaxSpeed { 220.0f };
    static constexpr float kGroundFriction { 12.0f };
    static constexpr float kJetDrag { 3.5f };

    GameEngine() noexcept;

    /// @brief Initialize new level simulation.
    /// @param[in] map Discrete tile grid environment.
    /// @param[in] playerSpawn World coordinates for initial player position.
    void initLevel(const TileMap& map, WorldPos playerSpawn) noexcept;

    /// @brief Master simulation update step.
    /// @param[in] dt Simulation delta time.
    /// @param[in] input Normalized input bitmask.
    void step(DeltaTime dt, EngineInput input) noexcept;

    // State Queries
    [[nodiscard]] WorldPos getPlayerPos() const noexcept
    {
        return playerPos_;
    }

    [[nodiscard]] Velocity getPlayerVel() const noexcept
    {
        return playerVel_;
    }

    [[nodiscard]] Hitbox getPlayerBox() const noexcept
    {
        return playerBox_;
    }

    [[nodiscard]] TransformMode getPlayerMode() const noexcept
    {
        return transformSys_.mode();
    }

    [[nodiscard]] EnergyPoints getPlayerEnergy() const noexcept
    {
        return energy_;
    }

    [[nodiscard]] FacingDirection getPlayerFacing() const noexcept
    {
        return playerFacing_;
    }

    [[nodiscard]] bool isPlayerAlive() const noexcept
    {
        return isAlive_;
    }

    [[nodiscard]] CameraViewport getViewport() const noexcept
    {
        return camera_;
    }

    /// @brief Calculate player craft bounding box for specified position and current mode.
    /// @param[in] pos World position of bottom-center anchor.
    /// @return Axis-aligned bounding box.
    [[nodiscard]] Hitbox computeHitbox(WorldPos pos) const noexcept;

    // Direct Access to Decoupled Output Queues
    [[nodiscard]] const RenderCommandQueue& getRenderQueue() const noexcept
    {
        return renderQueue_;
    }

    [[nodiscard]] const AudioCommandQueue& getAudioQueue() const noexcept
    {
        return audioQueue_;
    }

    // Subsystem Queries for Testing
    [[nodiscard]] const TileMap& getTileMap() const noexcept
    {
        return map_;
    }

    [[nodiscard]] TileMap& getTileMap() noexcept
    {
        return map_;
    }

    [[nodiscard]] const EntityPool& getEntityPool() const noexcept
    {
        return entities_;
    }

    [[nodiscard]] EntityPool& getEntityPool() noexcept
    {
        return entities_;
    }

    [[nodiscard]] const WeaponSystem& getWeaponSystem() const noexcept
    {
        return weaponSys_;
    }

    [[nodiscard]] const TransformSystem& getTransformSystem() const noexcept
    {
        return transformSys_;
    }

    /// @brief Spawn an enemy entity into the simulation.
    /// @param[in] type Entity classification category.
    /// @param[in] pos Continuous world coordinates.
    /// @param[in] bounds Hitbox extents.
    /// @param[in] hp Initial hit points.
    /// @return Generational EntityId handle.
    EntityId spawnEnemy(EntityType type, WorldPos pos, Hitbox bounds, std::uint16_t hp) noexcept;

private:
    void updateInput(DeltaTime dt, EngineInput input) noexcept;
    void updatePhysics(DeltaTime dt, EngineInput input) noexcept;
    void updateWeapons(DeltaTime dt, EngineInput input) noexcept;
    void updateEntities(DeltaTime dt) noexcept;
    void updateCamera() noexcept;
    void emitCommands() noexcept;

    TileMap map_ { 1U, 1U };
    EntityPool entities_ { };
    TransformSystem transformSys_ { };
    WeaponSystem weaponSys_ { };

    RenderCommandQueue renderQueue_ { };
    AudioCommandQueue audioQueue_ { };

    WorldPos playerPos_ { 0.0f, 0.0f };
    Velocity playerVel_ { 0.0f, 0.0f };
    Hitbox playerBox_ { };
    FacingDirection playerFacing_ { FacingDirection::Right };
    EnergyPoints energy_ { EnergyPoints::kDefaultMax };
    CameraViewport camera_ { };
    bool onGround_ { false };
    bool isAlive_ { true };
};

} // namespace ShooterCore
