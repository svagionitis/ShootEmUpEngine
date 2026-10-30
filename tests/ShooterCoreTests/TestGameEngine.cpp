#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include "AudioCommandQueue.h"
#include "EngineInput.h"
#include "EntityPool.h"
#include "GameEngine.h"
#include "Hitbox.h"
#include "RenderCommandQueue.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore::Test {

void runGameEngineTests() noexcept
{
    // Test 1: Robot walking, gravity and floor landing
    {
        TileMap map { 10U, 10U };
        // Place solid floor at row 5
        for (std::size_t col { 0U }; col < 10U; ++col) {
            map.setTile(TileCoord { static_cast<std::uint32_t>(col), 5U }, TileType::SolidWall);
        }

        GameEngine engine { };
        // Spawn at (32.0, 16.0) above floor
        engine.initLevel(map, WorldPos { 32.0f, 16.0f });

        TEST_ASSERT(engine.getPlayerMode() == TransformMode::Robot, "Initial mode must be Robot");
        TEST_ASSERT(engine.isPlayerAlive(), "Player must be alive");
        TEST_ASSERT(engine.getPlayerEnergy().value() == 100U, "Player energy must be 100");

        // Step simulation with gravity
        const DeltaTime dt { 0.1f };
        for (int i { 0 }; i < 10; ++i) {
            engine.step(dt, EngineInput { });
        }

        // Player should land on floor at row 5 (y = 5 * 16 = 80px).
        // Robot height = 32px, anchor bottom-center: max.y should contact floor at 80px.
        const Hitbox playerBox { engine.getPlayerBox() };
        TEST_ASSERT(playerBox.max().y <= 80.0f + 0.1f, "Robot must land on or above solid floor");

        // Walk right
        const float posXBefore { engine.getPlayerPos().x };
        EngineInput walkRight { };
        walkRight.setButton(EngineInput::kRight);
        engine.step(DeltaTime { 0.1f }, walkRight);

        TEST_ASSERT(engine.getPlayerPos().x > posXBefore, "Player must move right when kRight pressed");
        TEST_ASSERT(engine.getPlayerFacing() == FacingDirection::Right, "Facing direction must be Right");
    }

    // Test 2: Robot jump
    {
        TileMap map { 10U, 10U };
        for (std::size_t col { 0U }; col < 10U; ++col) {
            map.setTile(TileCoord { static_cast<std::uint32_t>(col), 5U }, TileType::SolidWall);
        }

        GameEngine engine { };
        engine.initLevel(map, WorldPos { 32.0f, 16.0f });

        // Let player land on ground
        for (int i { 0 }; i < 10; ++i) {
            engine.step(DeltaTime { 0.1f }, EngineInput { });
        }

        // Trigger Jump
        EngineInput jumpInput { };
        jumpInput.setButton(EngineInput::kJump);
        engine.step(DeltaTime { 0.016f }, jumpInput);

        TEST_ASSERT(engine.getPlayerVel().dy < 0.0f, "Jump impulse must give negative vertical velocity");
    }

    // Test 3: Jet flight movement and tunnel transformation restriction
    {
        TileMap map { 20U, 10U };
        // Create 1-tile tunnel between row 2 and row 4
        // Ceiling at row 2, Floor at row 4. Free row is row 3.
        for (std::size_t col { 0U }; col < 20U; ++col) {
            map.setTile(TileCoord { static_cast<std::uint32_t>(col), 2U }, TileType::SolidWall);
            map.setTile(TileCoord { static_cast<std::uint32_t>(col), 4U }, TileType::SolidWall);
        }

        GameEngine engine { };
        // Spawn inside open area
        engine.initLevel(map, WorldPos { 32.0f, 80.0f });

        // Transform to Jet
        EngineInput transInput { };
        transInput.setButton(EngineInput::kTransform);
        engine.step(DeltaTime { 0.016f }, transInput);

        // Step through transform delay (100ms)
        for (int i { 0 }; i < 10; ++i) {
            engine.step(DeltaTime { 0.02f }, EngineInput { });
        }

        TEST_ASSERT(engine.getPlayerMode() == TransformMode::Jet, "Player should be in Jet mode");

        // Fly inside the tunnel at row 3 (y = 56px)
        engine.initLevel(map, WorldPos { 64.0f, 56.0f });
        // Force jet mode
        engine.step(DeltaTime { 0.016f }, transInput);
        for (int i { 0 }; i < 10; ++i) {
            engine.step(DeltaTime { 0.02f }, EngineInput { });
        }
        TEST_ASSERT(engine.getPlayerMode() == TransformMode::Jet, "Must be in Jet mode in tunnel");

        // Attempt transform to Robot inside tunnel
        engine.step(DeltaTime { 0.016f }, transInput);
        TEST_ASSERT(engine.getPlayerMode() == TransformMode::Jet, "Transform in 1-tile tunnel must be rejected");
    }

    // Test 4: Laser auto-aim, damage delivery and energy drain
    {
        TileMap map { 20U, 20U };
        GameEngine engine { };
        engine.initLevel(map, WorldPos { 32.0f, 32.0f });

        // Spawn hostile enemy in front of player
        const Hitbox enemyBox { WorldPos { 80.0f, 24.0f }, WorldPos { 96.0f, 40.0f } };
        const EntityId enemyId { engine.spawnEnemy(EntityType::PatrolWalker, WorldPos { 80.0f, 24.0f }, enemyBox, 2U) };
        TEST_ASSERT(enemyId.isValid(), "Enemy spawn must succeed");

        // Fire laser
        EngineInput fireInput { };
        fireInput.setButton(EngineInput::kFire);

        // Advance simulation with laser firing
        engine.step(DeltaTime { 0.1f }, fireInput);

        TEST_ASSERT(engine.getWeaponSystem().isLaserActive(), "Laser must be active during fire input");
        TEST_ASSERT(engine.getPlayerEnergy().value() < 100U, "Firing laser must consume energy");

        // Check render queue emitted beam
        const RenderCommandQueue& rqueue { engine.getRenderQueue() };
        bool foundBeam { false };
        for (std::size_t i { 0U }; i < rqueue.count(); ++i) {
            if (rqueue.get(i).type == RenderCommandType::DrawBeam) {
                foundBeam = true;
                break;
            }
        }
        TEST_ASSERT(foundBeam, "Render queue must contain DrawBeam command");

        // Check audio queue emitted laser SFX
        const AudioCommandQueue& aqueue { engine.getAudioQueue() };
        bool foundLaserSfx { false };
        for (std::size_t i { 0U }; i < aqueue.count(); ++i) {
            if (aqueue.get(i).soundId == SoundEffectId::LaserFire) {
                foundLaserSfx = true;
                break;
            }
        }
        TEST_ASSERT(foundLaserSfx, "Audio queue must contain LaserFire sound command");
    }

    // Test 5: Shield barrier defense
    {
        TileMap map { 20U, 20U };
        GameEngine engine { };
        engine.initLevel(map, WorldPos { 32.0f, 32.0f });

        // Deploy shield
        EngineInput shieldInput { };
        shieldInput.setButton(EngineInput::kShield);
        engine.step(DeltaTime { 0.05f }, shieldInput);

        TEST_ASSERT(engine.getWeaponSystem().isShieldActive(), "Shield must be active when kShield is pressed");

        // Check render queue emitted shield representation and HUD
        const RenderCommandQueue& rqueue { engine.getRenderQueue() };
        bool foundBar { false };
        for (std::size_t i { 0U }; i < rqueue.count(); ++i) {
            if (rqueue.get(i).type == RenderCommandType::DrawBar) {
                foundBar = true;
                break;
            }
        }
        TEST_ASSERT(foundBar, "Render queue must emit HUD/shield DrawBar commands");
    }
}

} // namespace ShooterCore::Test
