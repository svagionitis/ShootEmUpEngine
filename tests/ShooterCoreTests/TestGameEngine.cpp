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

static void testWalkingAndLanding() noexcept
{
    TileMap map { 10U, 10U };
    for (std::size_t col { 0U }; col < 10U; ++col) {
        map.setTile(TileCoord { static_cast<std::uint32_t>(col), 5U }, TileType::SolidWall);
    }

    GameEngine engine { };
    engine.initLevel(map, WorldPos { 32.0f, 16.0f });

    TEST_ASSERT(engine.getPlayerMode() == TransformMode::Robot, "Initial mode must be Robot");
    TEST_ASSERT(engine.isPlayerAlive(), "Player must be alive");
    TEST_ASSERT(engine.getPlayerEnergy().value() == 100U, "Player energy must be 100");

    const DeltaTime dt { 0.1f };
    for (int i { 0 }; i < 10; ++i) {
        engine.step(dt, EngineInput { });
    }

    const Hitbox playerBox { engine.getPlayerBox() };
    TEST_ASSERT(playerBox.max().y <= 80.0f + 0.1f, "Robot must land on or above solid floor");

    const float posXBefore { engine.getPlayerPos().x };
    EngineInput walkRight { };
    walkRight.setButton(EngineInput::kRight);
    engine.step(DeltaTime { 0.1f }, walkRight);

    TEST_ASSERT(engine.getPlayerPos().x > posXBefore, "Player must move right when kRight pressed");
    TEST_ASSERT(engine.getPlayerFacing() == FacingDirection::Right, "Facing direction must be Right");
}

static void testJump() noexcept
{
    TileMap map { 10U, 10U };
    for (std::size_t col { 0U }; col < 10U; ++col) {
        map.setTile(TileCoord { static_cast<std::uint32_t>(col), 5U }, TileType::SolidWall);
    }

    GameEngine engine { };
    engine.initLevel(map, WorldPos { 32.0f, 16.0f });

    for (int i { 0 }; i < 10; ++i) {
        engine.step(DeltaTime { 0.1f }, EngineInput { });
    }

    EngineInput jumpInput { };
    jumpInput.setButton(EngineInput::kJump);
    engine.step(DeltaTime { 0.016f }, jumpInput);

    TEST_ASSERT(engine.getPlayerVel().dy < 0.0f, "Jump impulse must give negative vertical velocity");
}

static void testJetFlightAndTunnel() noexcept
{
    TileMap map { 20U, 10U };
    for (std::size_t col { 0U }; col < 20U; ++col) {
        map.setTile(TileCoord { static_cast<std::uint32_t>(col), 2U }, TileType::SolidWall);
        map.setTile(TileCoord { static_cast<std::uint32_t>(col), 4U }, TileType::SolidWall);
    }

    GameEngine engine { };
    engine.initLevel(map, WorldPos { 32.0f, 80.0f });

    EngineInput transInput { };
    transInput.setButton(EngineInput::kTransform);
    engine.step(DeltaTime { 0.016f }, transInput);

    for (int i { 0 }; i < 10; ++i) {
        engine.step(DeltaTime { 0.02f }, EngineInput { });
    }

    TEST_ASSERT(engine.getPlayerMode() == TransformMode::Jet, "Player should be in Jet mode");

    engine.initLevel(map, WorldPos { 64.0f, 56.0f });
    engine.step(DeltaTime { 0.016f }, transInput);
    for (int i { 0 }; i < 10; ++i) {
        engine.step(DeltaTime { 0.02f }, EngineInput { });
    }
    TEST_ASSERT(engine.getPlayerMode() == TransformMode::Jet, "Must be in Jet mode in tunnel");

    engine.step(DeltaTime { 0.016f }, transInput);
    TEST_ASSERT(engine.getPlayerMode() == TransformMode::Jet, "Transform in 1-tile tunnel must be rejected");
}

static void testLaserDamageAndParticles() noexcept
{
    TileMap map { 20U, 20U };
    GameEngine engine { };
    engine.initLevel(map, WorldPos { 32.0f, 32.0f });

    const Hitbox enemyBox { WorldPos { 80.0f, 24.0f }, WorldPos { 96.0f, 40.0f } };
    const EntityId enemyId { engine.spawnEnemy(EntityType::PatrolWalker, WorldPos { 80.0f, 24.0f }, enemyBox, 2U) };
    TEST_ASSERT(enemyId.isValid(), "Enemy spawn must succeed");

    EngineInput fireInput { };
    fireInput.setButton(EngineInput::kFire);

    engine.step(DeltaTime { 0.1f }, fireInput);

    TEST_ASSERT(engine.getWeaponSystem().isLaserActive(), "Laser must be active during fire input");
    TEST_ASSERT(engine.getPlayerEnergy().value() < 100U, "Firing laser must consume energy");

    // Phase 10: ParticlePool must contain impact sparks
    TEST_ASSERT(engine.getParticlePool().activeCount() > 0U, "Laser impact must emit particles into pool");

    const RenderCommandQueue& rqueue { engine.getRenderQueue() };
    bool foundLightning { false };
    bool foundParticles { false };
    for (std::size_t i { 0U }; i < rqueue.count(); ++i) {
        if (rqueue.get(i).type == RenderCommandType::DrawLightning) {
            foundLightning = true;
        } else if (rqueue.get(i).type == RenderCommandType::DrawParticle) {
            foundParticles = true;
        }
    }
    TEST_ASSERT(foundLightning, "Render queue must contain DrawLightning command");
    TEST_ASSERT(foundParticles, "Render queue must contain DrawParticle command");

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

static void testShieldBarrierDefense() noexcept
{
    TileMap map { 20U, 20U };
    GameEngine engine { };
    engine.initLevel(map, WorldPos { 32.0f, 32.0f });

    EngineInput shieldInput { };
    shieldInput.setButton(EngineInput::kShield);
    engine.step(DeltaTime { 0.05f }, shieldInput);

    TEST_ASSERT(engine.getWeaponSystem().isShieldActive(), "Shield must be active when kShield is pressed");

    const RenderCommandQueue& rqueue { engine.getRenderQueue() };
    bool foundShield { false };
    for (std::size_t i { 0U }; i < rqueue.count(); ++i) {
        if (rqueue.get(i).type == RenderCommandType::DrawShield) {
            foundShield = true;
            break;
        }
    }
    TEST_ASSERT(foundShield, "Render queue must emit DrawShield command");
}

void runGameEngineTests() noexcept
{
    testWalkingAndLanding();
    testJump();
    testJetFlightAndTunnel();
    testLaserDamageAndParticles();
    testShieldBarrierDefense();
}

} // namespace ShooterCore::Test
