#include "TestHarness.h"

#include <cstdint>

#include "EngineInput.h"

namespace ShooterCore::Test {

void runEngineInputTests() noexcept
{
    // Test 1: Defaults
    {
        constexpr EngineInput input { };
        TEST_ASSERT(input.raw() == EngineInput::kNone, "Default EngineInput must have raw value 0");
        TEST_ASSERT(!input.isUp(), "Default input Up must be false");
        TEST_ASSERT(!input.isDown(), "Default input Down must be false");
        TEST_ASSERT(!input.isLeft(), "Default input Left must be false");
        TEST_ASSERT(!input.isRight(), "Default input Right must be false");
        TEST_ASSERT(!input.isJump(), "Default input Jump must be false");
        TEST_ASSERT(!input.isTransform(), "Default input Transform must be false");
        TEST_ASSERT(!input.isFire(), "Default input Fire must be false");
        TEST_ASSERT(!input.isShield(), "Default input Shield must be false");
        TEST_ASSERT(!input.isFireSecondary(), "Default input FireSecondary must be false");
        TEST_ASSERT(!input.isCycleWeapon(), "Default input CycleWeapon must be false");
    }

    // Test 2: Button setting and clearing
    {
        EngineInput input { };
        input.setButton(EngineInput::kUp);
        TEST_ASSERT(input.isUp(), "Up must be true after setButton");
        TEST_ASSERT(input.raw() == EngineInput::kUp, "raw() must match kUp");

        input.setButton(EngineInput::kFireSecondary);
        TEST_ASSERT(input.isUp(), "Up must remain true");
        TEST_ASSERT(input.isFireSecondary(), "FireSecondary must be true");

        input.setButton(EngineInput::kCycleWeapon);
        TEST_ASSERT(input.isCycleWeapon(), "CycleWeapon must be true");

        input.clearButton(EngineInput::kUp);
        TEST_ASSERT(!input.isUp(), "Up must be false after clearButton");
        TEST_ASSERT(input.isFireSecondary(), "FireSecondary must remain true");
        TEST_ASSERT(input.isCycleWeapon(), "CycleWeapon must remain true");

        input.clear();
        TEST_ASSERT(input.raw() == EngineInput::kNone, "clear() must reset mask to 0");
        TEST_ASSERT(!input.isFireSecondary(), "FireSecondary must be false after clear");
        TEST_ASSERT(!input.isCycleWeapon(), "CycleWeapon must be false after clear");
    }

    // Test 3: Multiple simultaneous chords
    {
        constexpr std::uint16_t chord { static_cast<std::uint16_t>(
            EngineInput::kRight | EngineInput::kJump | EngineInput::kShield) };
        constexpr EngineInput input { chord };

        TEST_ASSERT(input.isRight(), "Right must be detected in chord");
        TEST_ASSERT(input.isJump(), "Jump must be detected in chord");
        TEST_ASSERT(input.isShield(), "Shield must be detected in chord");
        TEST_ASSERT(!input.isLeft(), "Left must be false in chord");
        TEST_ASSERT(!input.isFire(), "Fire must be false in chord");
        TEST_ASSERT(!input.isTransform(), "Transform must be false in chord");

        constexpr EngineInput matching { chord };
        TEST_ASSERT(input == matching, "Equal inputs must compare equal");
    }
}

} // namespace ShooterCore::Test
