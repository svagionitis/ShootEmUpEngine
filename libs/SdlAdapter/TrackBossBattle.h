#pragma once

#include <cstddef>
#include <cstdint>

#include <array>

#include "MusicTracker.h"

namespace SdlAdapter {

/// @brief Pre-baked tracker patterns for Dreadnought Guardian Stage Boss Battle.
namespace TrackBossBattle {

    // Pattern 0: Fast Chromatic Boss Confrontation (A minor / Diminished)
    inline constexpr TrackerPattern kPattern0 { { { // Row 0..3: Heavy chromatic alarm pulse
        TrackerRow { { 69U, 0U, 64U }, { 57U, 1U, 55U }, { 45U, 2U, 64U }, { 60U, 5U, 64U } },
        TrackerRow { { 70U, 0U, 62U }, { 58U, 1U, 52U }, { 45U, 2U, 55U }, { 60U, 3U, 40U } },
        TrackerRow { { 69U, 0U, 64U }, { 57U, 1U, 55U }, { 45U, 2U, 60U }, { 60U, 4U, 60U } },
        TrackerRow { { 68U, 0U, 60U }, { 56U, 1U, 50U }, { 45U, 2U, 55U }, { 60U, 3U, 40U } },

        // Row 4..7: Diminished leap
        TrackerRow { { 69U, 0U, 64U }, { 57U, 1U, 55U }, { 45U, 2U, 64U }, { 60U, 5U, 64U } },
        TrackerRow { { 72U, 0U, 62U }, { 60U, 1U, 52U }, { 45U, 2U, 55U }, { 60U, 3U, 40U } },
        TrackerRow { { 75U, 0U, 64U }, { 63U, 1U, 55U }, { 45U, 2U, 60U }, { 60U, 4U, 60U } },
        TrackerRow { { 74U, 0U, 60U }, { 62U, 1U, 50U }, { 45U, 2U, 55U }, { 60U, 3U, 40U } },

        // Row 8..11: Staccato alarm trill
        TrackerRow { { 75U, 0U, 64U }, { 63U, 1U, 58U }, { 51U, 2U, 64U }, { 60U, 5U, 64U } },
        TrackerRow { { 74U, 0U, 62U }, { 62U, 1U, 55U }, { 51U, 2U, 55U }, { 60U, 4U, 55U } },
        TrackerRow { { 75U, 0U, 64U }, { 63U, 1U, 58U }, { 51U, 2U, 60U }, { 60U, 4U, 60U } },
        TrackerRow { { 77U, 0U, 64U }, { 65U, 1U, 60U }, { 51U, 2U, 55U }, { 60U, 3U, 40U } },

        // Row 12..15: Cascading panic fall
        TrackerRow { { 81U, 0U, 64U }, { 69U, 1U, 60U }, { 45U, 2U, 64U }, { 60U, 5U, 64U } },
        TrackerRow { { 80U, 0U, 62U }, { 68U, 1U, 55U }, { 45U, 2U, 55U }, { 60U, 3U, 40U } },
        TrackerRow { { 78U, 0U, 60U }, { 66U, 1U, 52U }, { 45U, 2U, 60U }, { 60U, 4U, 60U } },
        TrackerRow { { 77U, 0U, 58U }, { 65U, 1U, 50U }, { 45U, 2U, 55U }, { 60U, 3U, 40U } },

        // Row 16..19: Repeated relentless march
        TrackerRow { { 69U, 0U, 64U }, { 57U, 1U, 55U }, { 45U, 2U, 64U }, { 60U, 5U, 64U } },
        TrackerRow { { 70U, 0U, 62U }, { 58U, 1U, 52U }, { 45U, 2U, 55U }, { 60U, 3U, 40U } },
        TrackerRow { { 69U, 0U, 64U }, { 57U, 1U, 55U }, { 45U, 2U, 60U }, { 60U, 4U, 60U } },
        TrackerRow { { 68U, 0U, 60U }, { 56U, 1U, 50U }, { 45U, 2U, 55U }, { 60U, 3U, 40U } },

        // Row 20..23: Rapid syncopated octave sweeps
        TrackerRow { { 81U, 0U, 64U }, { 69U, 1U, 60U }, { 45U, 2U, 64U }, { 60U, 5U, 64U } },
        TrackerRow { { 0U, 0U, 0U }, { 0U, 1U, 0U }, { 45U, 2U, 55U }, { 60U, 3U, 40U } },
        TrackerRow { { 80U, 0U, 62U }, { 68U, 1U, 58U }, { 45U, 2U, 60U }, { 60U, 4U, 60U } },
        TrackerRow { { 0U, 0U, 0U }, { 0U, 1U, 0U }, { 45U, 2U, 55U }, { 60U, 3U, 40U } },

        // Row 24..27: Staccato machinegun blasts
        TrackerRow { { 81U, 0U, 64U }, { 69U, 1U, 60U }, { 45U, 2U, 64U }, { 60U, 5U, 64U } },
        TrackerRow { { 254U, 0U, 0U }, { 254U, 1U, 0U }, { 0U, 2U, 0U }, { 60U, 4U, 60U } },
        TrackerRow { { 81U, 0U, 64U }, { 69U, 1U, 60U }, { 45U, 2U, 64U }, { 60U, 5U, 64U } },
        TrackerRow { { 254U, 0U, 0U }, { 254U, 1U, 0U }, { 0U, 2U, 0U }, { 60U, 4U, 60U } },

        // Row 28..31: Power cadence into loop
        TrackerRow { { 84U, 0U, 64U }, { 72U, 1U, 62U }, { 45U, 2U, 64U }, { 60U, 5U, 64U } },
        TrackerRow { { 83U, 0U, 62U }, { 71U, 1U, 60U }, { 45U, 2U, 55U }, { 60U, 4U, 55U } },
        TrackerRow { { 81U, 0U, 64U }, { 69U, 1U, 60U }, { 45U, 2U, 60U }, { 60U, 5U, 60U } },
        TrackerRow { { 254U, 0U, 0U }, { 254U, 1U, 0U }, { 0U, 2U, 0U }, { 60U, 4U, 60U } } } } };

    inline TrackerSong createSong() noexcept
    {
        TrackerSong song;
        song.patterns[0U] = &kPattern0;
        song.patternCount = 1U;
        song.bpm = 172.0f;
        song.ticksPerRow = 6U;
        song.loop = true;
        return song;
    }

} // namespace TrackBossBattle

} // namespace SdlAdapter
