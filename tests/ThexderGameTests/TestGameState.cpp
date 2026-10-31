#include "TestHarness.h"

#include <cstdint>

#include "EngineInput.h"
#include "GameState.h"
#include "RenderCommandQueue.h"
#include "Types.h"

namespace ThexderGame::Test {

static void testStateTransitions() noexcept
{
    GameState game { };

    // 1. Initial State
    TEST_ASSERT_EQ(static_cast<std::uint8_t>(game.getMode()), static_cast<std::uint8_t>(GameStateMode::Title),
        "Initial game state must be Title");

    // 2. Transition from Title to Playing on Fire input
    game.step(ShooterCore::DeltaTime { 0.016f }, ShooterCore::EngineInput { ShooterCore::EngineInput::kFire });
    TEST_ASSERT_EQ(static_cast<std::uint8_t>(game.getMode()), static_cast<std::uint8_t>(GameStateMode::Playing),
        "Pressing Fire should transition from Title to Playing");

    // 3. Camera bounds checking
    const ShooterCore::CameraViewport cam { game.getCamera() };
    TEST_ASSERT(cam.pos.x >= 0.0f, "Camera X must not be negative");
    TEST_ASSERT(cam.pos.y >= 0.0f, "Camera Y must not be negative");
    TEST_ASSERT(cam.pos.x <= 2240.0f, "Camera X must not exceed map horizontal bounds");
    TEST_ASSERT(cam.pos.y <= 272.0f, "Camera Y must not exceed map vertical bounds");

    // 4. Render Queue Generation in Playing State
    ShooterCore::RenderCommandQueue queue { };
    game.buildRenderQueue(queue);

    TEST_ASSERT(queue.count() > 0U, "Playing state must emit non-empty render queue");
    TEST_ASSERT(queue.count() < ShooterCore::RenderCommandQueue::kCapacity, "Render commands must fit in capacity");
}

static void testGameOverTransition() noexcept
{
    GameState playState { };
    playState.setMode(GameStateMode::Playing);
    playState.setGodMode(false);

    // Drain player energy completely
    playState.setPlayerEnergy(ShooterCore::EnergyPoints { 0U });

    playState.step(ShooterCore::DeltaTime { 0.016f }, ShooterCore::EngineInput { });
    TEST_ASSERT_EQ(static_cast<std::uint8_t>(playState.getMode()), static_cast<std::uint8_t>(GameStateMode::GameOver),
        "Depleting energy must trigger GameOver state");
}

static void testGodModeProtection() noexcept
{
    GameState godState { };
    godState.setMode(GameStateMode::Playing);
    godState.setGodMode(true);

    godState.setPlayerEnergy(ShooterCore::EnergyPoints { 0U });

    godState.step(ShooterCore::DeltaTime { 0.016f }, ShooterCore::EngineInput { });
    TEST_ASSERT_EQ(static_cast<std::uint8_t>(godState.getMode()), static_cast<std::uint8_t>(GameStateMode::Playing),
        "God mode must prevent GameOver transition");
}

void runGameStateTests()
{
    LOG(INFO) << "Executing GameState FSM & Camera Tracking Tests...";
    testStateTransitions();
    testGameOverTransition();
    testGodModeProtection();
}

} // namespace ThexderGame::Test
