#include "TestHarness.h"

#include <cstdint>

#include <SDL.h>

#include "EngineInput.h"
#include "SdlInput.h"

namespace SdlAdapter::Test {

namespace {

    void pushKeyEvent(std::uint32_t type, SDL_Keycode keycode) noexcept
    {
        SDL_Event evt { };
        evt.type = type;
        evt.key.keysym.sym = keycode;
        evt.key.repeat = 0U;
        (void)SDL_PushEvent(&evt);
    }

} // namespace

void runSdlInputTests() noexcept
{
    (void)SDL_InitSubSystem(SDL_INIT_EVENTS);

    // Test 1: Neutral initial state
    {
        SdlInput input;
        TEST_ASSERT_EQ(input.currentInput().raw(), ShooterCore::EngineInput::kNone, "Initial input must be neutral");
        TEST_ASSERT_FALSE(input.quitRequested(), "Initial quitRequested must be false");
    }

    // Test 2: Directional and Action key mappings
    {
        SdlInput input;

        // Push Up (W) and Jump (Space)
        pushKeyEvent(SDL_KEYDOWN, SDLK_w);
        pushKeyEvent(SDL_KEYDOWN, SDLK_SPACE);
        (void)input.pollEvents();

        TEST_ASSERT_TRUE(input.currentInput().isUp(), "W key should set Up bit");
        TEST_ASSERT_TRUE(input.currentInput().isJump(), "Space key should set Jump bit");
        TEST_ASSERT_TRUE(input.isJustPressed(ShooterCore::EngineInput::kUp), "Up should be just pressed");
        TEST_ASSERT_TRUE(input.isJustPressed(ShooterCore::EngineInput::kJump), "Jump should be just pressed");
        TEST_ASSERT_FALSE(input.currentInput().isDown(), "Down bit must not be set");

        // Next frame: keys held down (no new events)
        (void)input.pollEvents();
        TEST_ASSERT_TRUE(input.currentInput().isUp(), "Up should remain held");
        TEST_ASSERT_TRUE(input.currentInput().isJump(), "Jump should remain held");
        TEST_ASSERT_FALSE(input.isJustPressed(ShooterCore::EngineInput::kUp), "Up is held, not just pressed");

        // Release Jump
        pushKeyEvent(SDL_KEYUP, SDLK_SPACE);
        (void)input.pollEvents();
        TEST_ASSERT_TRUE(input.currentInput().isUp(), "Up should still be held");
        TEST_ASSERT_FALSE(input.currentInput().isJump(), "Jump should be released");
        TEST_ASSERT_TRUE(input.isJustReleased(ShooterCore::EngineInput::kJump), "Jump should be just released");

        // Clean up remaining keys
        pushKeyEvent(SDL_KEYUP, SDLK_w);
        (void)input.pollEvents();
        TEST_ASSERT_EQ(input.currentInput().raw(), ShooterCore::EngineInput::kNone, "Input should return to neutral");
    }

    // Test 3: Tactical Action Keys (Fire, Shield, Transform)
    {
        SdlInput input;

        pushKeyEvent(SDL_KEYDOWN, SDLK_x);
        pushKeyEvent(SDL_KEYDOWN, SDLK_c);
        pushKeyEvent(SDL_KEYDOWN, SDLK_TAB);
        (void)input.pollEvents();

        TEST_ASSERT_TRUE(input.currentInput().isFire(), "X should set Fire bit");
        TEST_ASSERT_TRUE(input.currentInput().isShield(), "C should set Shield bit");
        TEST_ASSERT_TRUE(input.currentInput().isTransform(), "Tab should set Transform bit");

        // Reset
        input.reset();
        TEST_ASSERT_EQ(input.currentInput().raw(), ShooterCore::EngineInput::kNone, "Reset must clear all bits");
    }

    // Test 4: Quit event handling
    {
        SdlInput input;
        SDL_Event quitEvt { };
        quitEvt.type = SDL_QUIT;
        (void)SDL_PushEvent(&quitEvt);

        const bool keepRunning = input.pollEvents();
        TEST_ASSERT_FALSE(keepRunning, "pollEvents should return false on SDL_QUIT");
        TEST_ASSERT_TRUE(input.quitRequested(), "quitRequested should be true on SDL_QUIT");
    }

    (void)SDL_QuitSubSystem(SDL_INIT_EVENTS);
}

} // namespace SdlAdapter::Test
