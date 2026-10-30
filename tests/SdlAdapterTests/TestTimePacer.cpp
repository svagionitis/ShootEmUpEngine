#include "TestHarness.h"

#include <cstdint>

#include "TimePacer.h"

namespace SdlAdapter::Test {

void runTimePacerTests() noexcept
{
    // Test 1: Timestep constants
    {
        TimePacer pacer;
        TEST_ASSERT_NEAR(pacer.fixedDelta(), 1.0f / 60.0f, 0.0001f, "fixedDelta must be 1/60th second");
        TEST_ASSERT_NEAR(pacer.rawDelta(), 0.0f, 0.0001f, "Initial rawDelta should be 0");
    }

    // Test 2: Accumulator accumulation below tick threshold
    {
        TimePacer pacer;
        // Step half a tick (8.33 ms)
        const std::uint32_t ticks1 = pacer.stepSimulated(0.008333f);
        TEST_ASSERT_EQ(ticks1, 0U, "8.33ms should produce 0 ticks");
        TEST_ASSERT_NEAR(pacer.subframeAlpha(), 0.5f, 0.05f, "subframeAlpha should be ~0.5 after half tick");

        // Step another half tick (9.0 ms)
        const std::uint32_t ticks2 = pacer.stepSimulated(0.009000f);
        TEST_ASSERT_EQ(ticks2, 1U, "Accruing across threshold must produce 1 tick");
    }

    // Test 3: Multiple ticks in single frame
    {
        TimePacer pacer;
        // Step 2 full ticks (33.34 ms)
        const std::uint32_t ticks = pacer.stepSimulated(0.03334f);
        TEST_ASSERT_EQ(ticks, 2U, "33.34ms should produce 2 ticks");
    }

    // Test 4: Spiral of death clamping
    {
        TimePacer pacer;
        // Massive lag spike: 1.0 second hitch
        const std::uint32_t ticks = pacer.stepSimulated(1.0f);
        // Clamped to 0.25s maximum -> 0.25 / (1/60) = 15 ticks
        TEST_ASSERT_EQ(ticks, 15U, "1.0s hitch must be clamped to 15 ticks (0.25s)");
    }

    // Test 5: Negative delta handling
    {
        TimePacer pacer;
        const std::uint32_t ticks = pacer.stepSimulated(-0.5f);
        TEST_ASSERT_EQ(ticks, 0U, "Negative delta must produce 0 ticks");
    }
}

} // namespace SdlAdapter::Test
