#pragma once

#include <cstddef>
#include <cstdint>

#include <array>

#include "MusicTracker.h"

namespace SdlAdapter {

/// @brief Pre-baked tracker patterns for Fire Hawk (Thexder 2) Action Theme.
namespace TrackFireHawk {

    // Pattern 0: High-Energy Action Theme Main Riff (D minor)
    inline constexpr TrackerPattern kPattern0 { { { // Row 0..3: Opening punchy riff
        TrackerRow { { 62U, 0U, 60U }, { 50U, 1U, 45U }, { 38U, 2U, 64U }, { 60U, 5U, 60U } },
        TrackerRow { { 0U, 0U, 0U }, { 0U, 1U, 0U }, { 38U, 2U, 50U }, { 0U, 3U, 0U } },
        TrackerRow { { 62U, 0U, 58U }, { 0U, 1U, 0U }, { 50U, 2U, 55U }, { 60U, 3U, 35U } },
        TrackerRow { { 65U, 0U, 60U }, { 0U, 1U, 0U }, { 38U, 2U, 55U }, { 0U, 3U, 0U } },

        // Row 4..7: Driving ascent
        TrackerRow { { 67U, 0U, 60U }, { 55U, 1U, 48U }, { 38U, 2U, 64U }, { 60U, 4U, 60U } },
        TrackerRow { { 0U, 0U, 0U }, { 0U, 1U, 0U }, { 50U, 2U, 50U }, { 0U, 3U, 0U } },
        TrackerRow { { 69U, 0U, 62U }, { 57U, 1U, 50U }, { 38U, 2U, 55U }, { 60U, 3U, 35U } },
        TrackerRow { { 0U, 0U, 0U }, { 0U, 1U, 0U }, { 50U, 2U, 50U }, { 0U, 3U, 0U } },

        // Row 8..11: Syncopated leap to high C / D
        TrackerRow { { 72U, 0U, 64U }, { 60U, 1U, 52U }, { 41U, 2U, 64U }, { 60U, 5U, 60U } },
        TrackerRow { { 0U, 0U, 0U }, { 0U, 1U, 0U }, { 41U, 2U, 50U }, { 0U, 3U, 0U } },
        TrackerRow { { 74U, 0U, 64U }, { 62U, 1U, 55U }, { 53U, 2U, 55U }, { 60U, 3U, 35U } },
        TrackerRow { { 72U, 0U, 60U }, { 60U, 1U, 50U }, { 41U, 2U, 55U }, { 0U, 3U, 0U } },

        // Row 12..15: Descending turnaround
        TrackerRow { { 69U, 0U, 60U }, { 57U, 1U, 48U }, { 43U, 2U, 64U }, { 60U, 4U, 60U } },
        TrackerRow { { 67U, 0U, 58U }, { 55U, 1U, 46U }, { 43U, 2U, 50U }, { 0U, 3U, 0U } },
        TrackerRow { { 65U, 0U, 58U }, { 53U, 1U, 46U }, { 55U, 2U, 55U }, { 60U, 3U, 35U } },
        TrackerRow { { 62U, 0U, 62U }, { 50U, 1U, 50U }, { 38U, 2U, 60U }, { 0U, 3U, 0U } },

        // Row 16..19: Repeated motif with variation
        TrackerRow { { 62U, 0U, 60U }, { 50U, 1U, 45U }, { 38U, 2U, 64U }, { 60U, 5U, 60U } },
        TrackerRow { { 0U, 0U, 0U }, { 0U, 1U, 0U }, { 38U, 2U, 50U }, { 0U, 3U, 0U } },
        TrackerRow { { 65U, 0U, 58U }, { 0U, 1U, 0U }, { 50U, 2U, 55U }, { 60U, 3U, 35U } },
        TrackerRow { { 67U, 0U, 60U }, { 0U, 1U, 0U }, { 38U, 2U, 55U }, { 0U, 3U, 0U } },

        // Row 20..23: High soaring melody
        TrackerRow { { 74U, 0U, 64U }, { 62U, 1U, 55U }, { 38U, 2U, 64U }, { 60U, 4U, 60U } },
        TrackerRow { { 0U, 0U, 0U }, { 0U, 1U, 0U }, { 50U, 2U, 50U }, { 0U, 3U, 0U } },
        TrackerRow { { 72U, 0U, 62U }, { 60U, 1U, 52U }, { 38U, 2U, 55U }, { 60U, 3U, 35U } },
        TrackerRow { { 69U, 0U, 60U }, { 57U, 1U, 50U }, { 50U, 2U, 50U }, { 0U, 3U, 0U } },

        // Row 24..27: Staccato rhythmic stabs
        TrackerRow { { 77U, 0U, 64U }, { 65U, 1U, 58U }, { 46U, 2U, 64U }, { 60U, 5U, 60U } },
        TrackerRow { { 254U, 0U, 0U }, { 254U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },
        TrackerRow { { 76U, 0U, 62U }, { 64U, 1U, 55U }, { 45U, 2U, 64U }, { 60U, 4U, 60U } },
        TrackerRow { { 254U, 0U, 0U }, { 254U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },

        // Row 28..31: Cadence back to D
        TrackerRow { { 74U, 0U, 64U }, { 62U, 1U, 58U }, { 38U, 2U, 64U }, { 60U, 5U, 60U } },
        TrackerRow { { 0U, 0U, 0U }, { 0U, 1U, 0U }, { 38U, 2U, 50U }, { 0U, 3U, 0U } },
        TrackerRow { { 0U, 0U, 0U }, { 0U, 1U, 0U }, { 50U, 2U, 55U }, { 60U, 4U, 55U } },
        TrackerRow { { 254U, 0U, 0U }, { 254U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } } } } };

    inline TrackerSong createSong() noexcept
    {
        TrackerSong song;
        song.patterns[0U] = &kPattern0;
        song.patternCount = 1U;
        song.bpm = 140.0f;
        song.ticksPerRow = 6U;
        song.loop = true;
        return song;
    }

} // namespace TrackFireHawk

} // namespace SdlAdapter
