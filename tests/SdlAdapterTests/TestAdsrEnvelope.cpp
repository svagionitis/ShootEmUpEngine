#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include "AdsrEnvelope.h"

namespace SdlAdapter::Test {

void runAdsrEnvelopeTests() noexcept
{
    // Test 1: Initial idle state
    {
        AdsrEnvelope env;
        TEST_ASSERT(env.state() == EnvelopeState::Idle, "Initial state must be Idle");
        TEST_ASSERT(env.level() == 0.0f, "Initial level must be 0.0f");
        TEST_ASSERT(!env.isActive(), "Fresh envelope must not be active");

        const float sample { env.process(44100.0f) };
        TEST_ASSERT(sample == 0.0f, "Idle envelope must output 0.0f");
    }

    // Test 2: Full ADSR lifecycle
    {
        AdsrConfig cfg;
        cfg.attackSec = 0.010f; // 10ms (441 samples at 44.1kHz)
        cfg.decaySec = 0.020f; // 20ms (882 samples)
        cfg.sustainLevel = 0.60f; // 60% sustain
        cfg.releaseSec = 0.030f; // 30ms (1323 samples)

        AdsrEnvelope env { cfg };
        env.noteOn();
        TEST_ASSERT(env.state() == EnvelopeState::Attack, "After noteOn, state must be Attack");
        TEST_ASSERT(env.isActive(), "Envelope must be active after noteOn");

        // Process attack phase
        for (std::size_t i { 0U }; i < 441U; ++i) {
            static_cast<void>(env.process(44100.0f));
        }
        TEST_ASSERT(env.state() == EnvelopeState::Decay, "State must transition to Decay after attack");
        TEST_ASSERT_NEAR(env.level(), 1.0f, 0.01f, "Level at end of attack must be near 1.0");

        // Process decay phase
        for (std::size_t i { 0U }; i < 882U; ++i) {
            static_cast<void>(env.process(44100.0f));
        }
        TEST_ASSERT(env.state() == EnvelopeState::Sustain, "State must transition to Sustain after decay");
        TEST_ASSERT_NEAR(env.level(), 0.60f, 0.01f, "Level during sustain must match sustainLevel");

        // Sustain holds indefinitely
        for (std::size_t i { 0U }; i < 1000U; ++i) {
            static_cast<void>(env.process(44100.0f));
        }
        TEST_ASSERT(env.state() == EnvelopeState::Sustain, "State must remain Sustain while note is held");
        TEST_ASSERT_NEAR(env.level(), 0.60f, 0.01f, "Sustain level must stay stable");

        // Trigger noteOff
        env.noteOff();
        TEST_ASSERT(env.state() == EnvelopeState::Release, "State must transition to Release after noteOff");

        // Process release phase
        for (std::size_t i { 0U }; i < 1350U; ++i) {
            static_cast<void>(env.process(44100.0f));
        }
        TEST_ASSERT(env.state() == EnvelopeState::Idle, "State must return to Idle after release");
        TEST_ASSERT(env.level() == 0.0f, "Level after release must be 0.0f");
        TEST_ASSERT(!env.isActive(), "Envelope must be inactive after release completes");
    }

    // Test 3: Anti-click retriggering
    {
        AdsrConfig cfg;
        cfg.attackSec = 0.050f;
        cfg.decaySec = 0.050f;
        cfg.sustainLevel = 0.50f;
        cfg.releaseSec = 0.050f;

        AdsrEnvelope env { cfg };
        env.noteOn();
        // Advance into sustain
        for (std::size_t i { 0U }; i < 5000U; ++i) {
            static_cast<void>(env.process(44100.0f));
        }

        env.noteOff();
        // Advance halfway through release
        for (std::size_t i { 0U }; i < 1000U; ++i) {
            static_cast<void>(env.process(44100.0f));
        }
        const float midReleaseLevel { env.level() };
        TEST_ASSERT(midReleaseLevel > 0.1f && midReleaseLevel < 0.5f, "Should be mid-release");

        // Retrigger noteOn mid-release
        env.noteOn();
        TEST_ASSERT(env.state() == EnvelopeState::Attack, "Retrigger must set state to Attack");
        const float retriggerStartLevel { env.level() };
        TEST_ASSERT_NEAR(
            retriggerStartLevel, midReleaseLevel, 0.001f, "Retrigger must preserve current level to eliminate click");
    }
}

} // namespace SdlAdapter::Test
