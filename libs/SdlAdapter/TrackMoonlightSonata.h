#pragma once

#include <cstddef>
#include <cstdint>

#include <array>

#include "MusicTracker.h"

namespace SdlAdapter {

/// @brief Pre-baked tracker patterns for Beethoven's Moonlight Sonata (3rd Movement - Presto agitato).
namespace TrackMoonlightSonata {

    // Pattern 0: Main Theme Exposition Part 1 (C# minor arpeggiation)
    inline constexpr TrackerPattern kPattern0 { { { // Row 0..3: C#m ascending arpeggio
        TrackerRow { { 56U, 0U, 55U }, { 0U, 1U, 0U }, { 37U, 2U, 60U }, { 60U, 3U, 45U } },
        TrackerRow { { 61U, 0U, 55U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },
        TrackerRow { { 64U, 0U, 55U }, { 0U, 1U, 0U }, { 49U, 2U, 55U }, { 60U, 3U, 35U } },
        TrackerRow { { 68U, 0U, 55U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },

        // Row 4..7: Higher octave ascent
        TrackerRow { { 73U, 0U, 58U }, { 0U, 1U, 0U }, { 37U, 2U, 60U }, { 60U, 4U, 55U } },
        TrackerRow { { 76U, 0U, 58U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },
        TrackerRow { { 80U, 0U, 58U }, { 0U, 1U, 0U }, { 49U, 2U, 55U }, { 60U, 3U, 35U } },
        TrackerRow { { 85U, 0U, 60U }, { 73U, 1U, 55U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },

        // Row 8..11: Staccato sforzando chords
        TrackerRow { { 85U, 0U, 62U }, { 76U, 1U, 58U }, { 37U, 2U, 64U }, { 60U, 4U, 60U } },
        TrackerRow { { 254U, 0U, 0U }, { 254U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },
        TrackerRow { { 85U, 0U, 62U }, { 76U, 1U, 58U }, { 37U, 2U, 64U }, { 60U, 3U, 40U } },
        TrackerRow { { 254U, 0U, 0U }, { 254U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },

        // Row 12..15: G#7 Dominant resolution descent
        TrackerRow { { 80U, 0U, 58U }, { 68U, 1U, 50U }, { 44U, 2U, 60U }, { 60U, 4U, 55U } },
        TrackerRow { { 76U, 0U, 55U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },
        TrackerRow { { 71U, 0U, 55U }, { 0U, 1U, 0U }, { 56U, 2U, 55U }, { 60U, 3U, 35U } },
        TrackerRow { { 68U, 0U, 55U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },

        // Row 16..19: G#7 Arpeggio rise
        TrackerRow { { 56U, 0U, 55U }, { 0U, 1U, 0U }, { 44U, 2U, 60U }, { 60U, 3U, 45U } },
        TrackerRow { { 59U, 0U, 55U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },
        TrackerRow { { 63U, 0U, 55U }, { 0U, 1U, 0U }, { 56U, 2U, 55U }, { 60U, 3U, 35U } },
        TrackerRow { { 68U, 0U, 55U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },

        // Row 20..23: Higher G# ascent
        TrackerRow { { 71U, 0U, 58U }, { 0U, 1U, 0U }, { 44U, 2U, 60U }, { 60U, 4U, 55U } },
        TrackerRow { { 75U, 0U, 58U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },
        TrackerRow { { 80U, 0U, 58U }, { 0U, 1U, 0U }, { 56U, 2U, 55U }, { 60U, 3U, 35U } },
        TrackerRow { { 83U, 0U, 60U }, { 71U, 1U, 55U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },

        // Row 24..27: Staccato cadence
        TrackerRow { { 83U, 0U, 62U }, { 75U, 1U, 58U }, { 44U, 2U, 64U }, { 60U, 4U, 60U } },
        TrackerRow { { 254U, 0U, 0U }, { 254U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },
        TrackerRow { { 83U, 0U, 62U }, { 75U, 1U, 58U }, { 44U, 2U, 64U }, { 60U, 3U, 40U } },
        TrackerRow { { 254U, 0U, 0U }, { 254U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },

        // Row 28..31: Turnaround back to C#
        TrackerRow { { 80U, 0U, 58U }, { 68U, 1U, 50U }, { 37U, 2U, 60U }, { 60U, 4U, 55U } },
        TrackerRow { { 76U, 0U, 55U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },
        TrackerRow { { 73U, 0U, 55U }, { 0U, 1U, 0U }, { 49U, 2U, 55U }, { 60U, 3U, 35U } },
        TrackerRow { { 68U, 0U, 55U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } } } } };

    // Pattern 1: Melodic Development Part 2
    inline constexpr TrackerPattern kPattern1 { { { // Row 0..7: Fast cascade downwards
        TrackerRow { { 85U, 0U, 60U }, { 61U, 1U, 45U }, { 37U, 2U, 60U }, { 60U, 4U, 55U } },
        TrackerRow { { 83U, 0U, 58U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },
        TrackerRow { { 81U, 0U, 58U }, { 0U, 1U, 0U }, { 49U, 2U, 55U }, { 60U, 3U, 35U } },
        TrackerRow { { 80U, 0U, 56U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },
        TrackerRow { { 78U, 0U, 56U }, { 64U, 1U, 45U }, { 42U, 2U, 60U }, { 60U, 4U, 55U } },
        TrackerRow { { 76U, 0U, 54U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },
        TrackerRow { { 75U, 0U, 54U }, { 0U, 1U, 0U }, { 54U, 2U, 55U }, { 60U, 3U, 35U } },
        TrackerRow { { 73U, 0U, 52U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },

        // Row 8..15: Rapid climbing runs
        TrackerRow { { 68U, 0U, 55U }, { 68U, 1U, 50U }, { 44U, 2U, 60U }, { 60U, 4U, 55U } },
        TrackerRow { { 70U, 0U, 55U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },
        TrackerRow { { 71U, 0U, 56U }, { 0U, 1U, 0U }, { 56U, 2U, 55U }, { 60U, 3U, 35U } },
        TrackerRow { { 73U, 0U, 56U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },
        TrackerRow { { 75U, 0U, 58U }, { 71U, 1U, 50U }, { 44U, 2U, 60U }, { 60U, 4U, 55U } },
        TrackerRow { { 76U, 0U, 58U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },
        TrackerRow { { 78U, 0U, 60U }, { 0U, 1U, 0U }, { 56U, 2U, 55U }, { 60U, 3U, 35U } },
        TrackerRow { { 80U, 0U, 62U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },

        // Row 16..23: Climactic trill and chord resolution
        TrackerRow { { 80U, 0U, 64U }, { 73U, 1U, 60U }, { 37U, 2U, 64U }, { 60U, 4U, 60U } },
        TrackerRow { { 78U, 0U, 60U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },
        TrackerRow { { 80U, 0U, 64U }, { 73U, 1U, 60U }, { 49U, 2U, 58U }, { 60U, 3U, 40U } },
        TrackerRow { { 78U, 0U, 60U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },
        TrackerRow { { 80U, 0U, 64U }, { 73U, 1U, 60U }, { 37U, 2U, 64U }, { 60U, 4U, 60U } },
        TrackerRow { { 254U, 0U, 0U }, { 254U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },
        TrackerRow { { 80U, 0U, 64U }, { 73U, 1U, 60U }, { 49U, 2U, 58U }, { 60U, 3U, 40U } },
        TrackerRow { { 254U, 0U, 0U }, { 254U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },

        // Row 24..31: Return cadence to C# minor
        TrackerRow { { 73U, 0U, 60U }, { 64U, 1U, 52U }, { 37U, 2U, 62U }, { 60U, 4U, 55U } },
        TrackerRow { { 68U, 0U, 55U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },
        TrackerRow { { 64U, 0U, 55U }, { 0U, 1U, 0U }, { 49U, 2U, 55U }, { 60U, 3U, 35U } },
        TrackerRow { { 61U, 0U, 55U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },
        TrackerRow { { 56U, 0U, 55U }, { 0U, 1U, 0U }, { 37U, 2U, 60U }, { 60U, 4U, 55U } },
        TrackerRow { { 61U, 0U, 55U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } },
        TrackerRow { { 64U, 0U, 58U }, { 0U, 1U, 0U }, { 49U, 2U, 55U }, { 60U, 3U, 35U } },
        TrackerRow { { 68U, 0U, 60U }, { 0U, 1U, 0U }, { 0U, 2U, 0U }, { 0U, 3U, 0U } } } } };

    inline TrackerSong createSong() noexcept
    {
        TrackerSong song;
        song.patterns[0U] = &kPattern0;
        song.patterns[1U] = &kPattern1;
        song.patternCount = 2U;
        song.bpm = 165.0f;
        song.ticksPerRow = 6U;
        song.loop = true;
        return song;
    }

} // namespace TrackMoonlightSonata

} // namespace SdlAdapter
