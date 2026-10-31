#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include <array>

#include "ChiptuneVoice.h"
#include "MusicTracker.h"
#include "Types.h"

namespace SdlAdapter::Test {

void runMusicTrackerTests() noexcept
{
    // Test 1: Note frequency mapping
    {
        TEST_ASSERT(MusicTracker::noteToFrequency(0U) == 0.0f, "Rest note must return 0.0 Hz");
        TEST_ASSERT(MusicTracker::noteToFrequency(MusicTracker::kNoteCut) == 0.0f, "Cut note must return 0.0 Hz");

        const float a4 { MusicTracker::noteToFrequency(69U) };
        TEST_ASSERT_NEAR(a4, 440.0f, 0.1f, "MIDI note 69 (A4) must be 440.0 Hz");

        const float c4 { MusicTracker::noteToFrequency(60U) };
        TEST_ASSERT_NEAR(c4, 261.63f, 0.5f, "MIDI note 60 (C4) must be ~261.63 Hz");
    }

    // Test 2: Song loading and state initialization
    {
        MusicTracker tracker;
        TEST_ASSERT(!tracker.isPlaying(), "Fresh tracker must not be playing");

        tracker.playTrack(ShooterCore::MusicTrackId::MainTheme, true);
        TEST_ASSERT(tracker.isPlaying(), "Tracker must be playing after playTrack");
        TEST_ASSERT(tracker.currentTrack() == ShooterCore::MusicTrackId::MainTheme, "Active track must be MainTheme");
        TEST_ASSERT(tracker.patternIndex() == 0U, "Initial pattern index must be 0");
        TEST_ASSERT(tracker.rowIndex() == 0U, "Initial row index must be 0");
    }

    // Test 3: Sample advancement and row progression
    {
        MusicTracker tracker;
        std::array<ChiptuneVoice, 5> voices { };

        tracker.playTrack(ShooterCore::MusicTrackId::MainTheme, true);

        // Advance 1 full tick (~735 samples at 44.1kHz, 165 BPM)
        for (std::size_t i { 0U }; i < 1000U; ++i) {
            tracker.advanceSample(voices, 44100.0f);
        }

        // Voice 0 (Pulse Lead) should now be active playing Moonlight Sonata note
        TEST_ASSERT(voices[0].isActive(), "Voice 0 must be active after triggering row 0");

        // Advance through multiple rows
        for (std::size_t i { 0U }; i < 44100U; ++i) {
            tracker.advanceSample(voices, 44100.0f);
        }
        TEST_ASSERT(tracker.rowIndex() > 0U, "Row index must advance as samples accumulate");

        // Test stopping
        tracker.stop(voices);
        TEST_ASSERT(!tracker.isPlaying(), "Tracker must report not playing after stop");
    }

    // Test 4: Fire Hawk track playback
    {
        MusicTracker tracker;
        std::array<ChiptuneVoice, 5> voices { };

        tracker.playTrack(ShooterCore::MusicTrackId::Level1Cavern, true);
        TEST_ASSERT(tracker.isPlaying(), "Tracker must play Fire Hawk track");

        for (std::size_t i { 0U }; i < 1500U; ++i) {
            tracker.advanceSample(voices, 44100.0f);
        }
        TEST_ASSERT(voices[0].isActive(), "Voice 0 must be active playing Fire Hawk theme");

        tracker.stop(voices);
    }

    // Test 5: Boss Battle track playback
    {
        MusicTracker tracker;
        std::array<ChiptuneVoice, 5> voices { };

        tracker.playTrack(ShooterCore::MusicTrackId::BossBattle, true);
        TEST_ASSERT(tracker.isPlaying(), "Tracker must play Boss Battle track");

        for (std::size_t i { 0U }; i < 1500U; ++i) {
            tracker.advanceSample(voices, 44100.0f);
        }
        TEST_ASSERT(voices[0].isActive(), "Voice 0 must be active playing Boss theme");

        tracker.stop(voices);
    }
}

} // namespace SdlAdapter::Test
