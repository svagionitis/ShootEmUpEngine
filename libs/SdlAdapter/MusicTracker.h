#pragma once

#include <cstddef>
#include <cstdint>

#include <array>

#include "ChiptuneVoice.h"
#include "Types.h"

namespace SdlAdapter {

/// @struct TrackerCell
/// @brief A single musical note event within a tracker pattern.
struct TrackerCell final {
    std::uint8_t note { 0U }; ///< MIDI note number (1..96), 0 = rest/hold, 254 = note off/cut.
    std::uint8_t instrument { 0U }; ///< Instrument preset ID.
    std::uint8_t volume { 64U }; ///< Note velocity in range [0, 64].

    constexpr TrackerCell() noexcept = default;
    constexpr TrackerCell(std::uint8_t n, std::uint8_t i, std::uint8_t v) noexcept
        : note { n }
        , instrument { i }
        , volume { v }
    {
    }
};

/// @struct TrackerRow
/// @brief 4-channel simultaneous musical step.
struct TrackerRow final {
    std::array<TrackerCell, 4> channels { };

    constexpr TrackerRow() noexcept = default;
    constexpr TrackerRow(TrackerCell c0, TrackerCell c1, TrackerCell c2, TrackerCell c3) noexcept
        : channels { { c0, c1, c2, c3 } }
    {
    }
};

/// @struct TrackerPattern
/// @brief A 32-row musical measure block.
struct TrackerPattern final {
    static constexpr std::size_t kRowCount { 32U };
    std::array<TrackerRow, kRowCount> rows { };
};

/// @struct TrackerSong
/// @brief Complete structured composition comprising an ordered sequence of pattern pointers.
struct TrackerSong final {
    static constexpr std::size_t kMaxPatterns { 16U };
    std::array<const TrackerPattern*, kMaxPatterns> patterns { };
    std::size_t patternCount { 0U };
    float bpm { 150.0f };
    std::uint8_t ticksPerRow { 6U };
    bool loop { true };
};

/// @class MusicTracker
/// @brief Deterministic 4-track chiptune music sequencer driven by sample-accurate clock division.
class MusicTracker final {
public:
    static constexpr std::uint8_t kRest { 0U };
    static constexpr std::uint8_t kNoteCut { 254U };
    static constexpr std::size_t kVoiceCount { 4U };

    constexpr MusicTracker() noexcept = default;

    /// @brief Load and start playback of a designated music track.
    /// @param[in] track Song identifier.
    /// @param[in] loop Whether the song repeats upon reaching the end.
    void playTrack(ShooterCore::MusicTrackId track, bool loop = true) noexcept;

    /// @brief Stop current track playback and silence music voices.
    /// @param[in,out] voices Synthesizer voices to release.
    void stop(std::array<ChiptuneVoice, 5>& voices) noexcept;

    /// @brief Advance tracker by one audio sample period, triggering notes when tick thresholds expire.
    /// @param[in,out] voices Synthesizer voices to update.
    /// @param[in] sampleRate Output sampling frequency in Hertz.
    void advanceSample(std::array<ChiptuneVoice, 5>& voices, float sampleRate) noexcept;

    /// @brief Query active music track identifier.
    /// @return Active MusicTrackId.
    [[nodiscard]] constexpr ShooterCore::MusicTrackId currentTrack() const noexcept
    {
        return currentTrackId_;
    }

    /// @brief Check whether tracker is currently playing music.
    /// @return true if an active song is sequencing.
    [[nodiscard]] constexpr bool isPlaying() const noexcept
    {
        return isPlaying_;
    }

    /// @brief Query current pattern index in song sequence.
    [[nodiscard]] constexpr std::size_t patternIndex() const noexcept
    {
        return patternIndex_;
    }

    /// @brief Query current row index within active pattern.
    [[nodiscard]] constexpr std::size_t rowIndex() const noexcept
    {
        return rowIndex_;
    }

    /// @brief Convert 1-based MIDI note number to fundamental frequency in Hertz.
    /// @param[in] note MIDI note index (1..96).
    /// @return Frequency in Hertz, or 0.0f if note is 0.
    [[nodiscard]] static float noteToFrequency(std::uint8_t note) noexcept;

private:
    void triggerRow(std::array<ChiptuneVoice, 5>& voices) noexcept;
    void loadSong(ShooterCore::MusicTrackId track) noexcept;

    TrackerSong currentSong_ { };
    ShooterCore::MusicTrackId currentTrackId_ { ShooterCore::MusicTrackId::None };
    bool isPlaying_ { false };

    std::size_t patternIndex_ { 0U };
    std::size_t rowIndex_ { 0U };
    std::uint8_t currentTick_ { 0U };

    float sampleAccumulator_ { 0.0f };
    float samplesPerTick_ { 735.0f };
};

} // namespace SdlAdapter
