#include "TestHarness.h"

#include <cstdint>

#include "CrtPostProcessor.h"
#include "Types.h"

namespace SdlAdapter::Test {

void runCrtPostProcessorTests() noexcept
{
    LOG(INFO) << "Executing CrtPostProcessor State & Profile Tests...";

    CrtPostProcessor crt { };

    // 1. Initial Profile
    TEST_ASSERT_EQ(static_cast<std::uint8_t>(crt.mode()), static_cast<std::uint8_t>(ShooterCore::CrtFilterMode::Arcade),
        "Default CRT mode must be Arcade");

    // 2. Mode Cycling (Arcade -> RetroCrt -> Disabled -> Subtle -> Arcade)
    TEST_ASSERT_EQ(static_cast<std::uint8_t>(crt.cycleMode()),
        static_cast<std::uint8_t>(ShooterCore::CrtFilterMode::RetroCrt), "Cycle from Arcade to RetroCrt");

    TEST_ASSERT_EQ(static_cast<std::uint8_t>(crt.cycleMode()),
        static_cast<std::uint8_t>(ShooterCore::CrtFilterMode::Disabled), "Cycle from RetroCrt to Disabled");

    TEST_ASSERT_EQ(static_cast<std::uint8_t>(crt.cycleMode()),
        static_cast<std::uint8_t>(ShooterCore::CrtFilterMode::Subtle), "Cycle from Disabled to Subtle");

    TEST_ASSERT_EQ(static_cast<std::uint8_t>(crt.cycleMode()),
        static_cast<std::uint8_t>(ShooterCore::CrtFilterMode::Arcade), "Cycle from Subtle to Arcade");

    // 3. Direct Mode Setter
    crt.setMode(ShooterCore::CrtFilterMode::Subtle);
    TEST_ASSERT_EQ(static_cast<std::uint8_t>(crt.mode()), static_cast<std::uint8_t>(ShooterCore::CrtFilterMode::Subtle),
        "Direct setMode Subtle");

    // 4. Null Renderer Hardening
    TEST_ASSERT(!crt.init(nullptr), "Init with nullptr renderer must return false safely");
    crt.render(nullptr); // Must not crash
}

} // namespace SdlAdapter::Test
