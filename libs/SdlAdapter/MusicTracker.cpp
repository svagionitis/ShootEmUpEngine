#include "MusicTracker.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <array>
#include <cmath>

#include "AdsrEnvelope.h"
#include "ChiptuneVoice.h"
#include "TrackBossBattle.h"
#include "TrackFireHawk.h"
#include "TrackMoonlightSonata.h"
#include "Types.h"

namespace SdlAdapter {

namespace {

    struct InstrumentPreset final {
        ChannelWaveform waveform { ChannelWaveform::Pulse };
        float dutyCycle { 0.50f };
        AdsrConfig adsr { };
        float sweepRate { 0.0f };
    };

    constexpr std::array<InstrumentPreset, 6> kInstruments { { // 0: Lead Pulse (25% duty, crisp arcade bite)
        { ChannelWaveform::Pulse, 0.25f, { 0.004f, 0.070f, 0.65f, 0.050f }, 0.0f },
        // 1: Harmony Pulse (50% square wave, smooth counterpoint)
        { ChannelWaveform::Pulse, 0.50f, { 0.008f, 0.090f, 0.50f, 0.070f }, 0.0f },
        // 2: Bass Triangle (Warm low-frequency fundamental)
        { ChannelWaveform::Triangle, 0.50f, { 0.005f, 0.110f, 0.70f, 0.040f }, 0.0f },
        // 3: Hi-hat Noise (Metallic short-period LFSR)
        { ChannelWaveform::NoiseMetallic, 0.50f, { 0.001f, 0.035f, 0.00f, 0.015f }, 0.0f },
        // 4: Snare Noise (White noise burst)
        { ChannelWaveform::NoiseWhite, 0.50f, { 0.002f, 0.090f, 0.00f, 0.030f }, 0.0f },
        // 5: Kick Drum (Punchy sub-bass drop)
        { ChannelWaveform::Triangle, 0.50f, { 0.002f, 0.080f, 0.00f, 0.020f }, -480.0f } } };

    // Standard pre-computed equal-temperament note frequencies for MIDI 1..96 (A4 = 69 = 440 Hz)
    float computeMidiHz(std::uint8_t note) noexcept
    {
        if (note == 0U) {
            return 0.0f;
        }
        const auto noteDiff { static_cast<float>(static_cast<int>(note) - 69) };
        return 440.0f * std::pow(2.0f, noteDiff / 12.0f);
    }

} // namespace

float MusicTracker::noteToFrequency(std::uint8_t note) noexcept
{
    if (note == 0U || note == kNoteCut) {
        return 0.0f;
    }
    return computeMidiHz(note);
}

void MusicTracker::loadSong(ShooterCore::MusicTrackId track) noexcept
{
    switch (track) {
    case ShooterCore::MusicTrackId::MainTheme:
        currentSong_ = TrackMoonlightSonata::createSong();
        break;

    case ShooterCore::MusicTrackId::Level1Cavern:
        currentSong_ = TrackFireHawk::createSong();
        break;

    case ShooterCore::MusicTrackId::BossBattle:
        currentSong_ = TrackBossBattle::createSong();
        break;

    case ShooterCore::MusicTrackId::GameOver:
    case ShooterCore::MusicTrackId::StageClear:
    case ShooterCore::MusicTrackId::None:
    default:
        currentSong_ = TrackerSong { };
        break;
    }
}

void MusicTracker::playTrack(ShooterCore::MusicTrackId track, bool loop) noexcept
{
    loadSong(track);
    currentTrackId_ = track;
    currentSong_.loop = loop;

    if (currentSong_.patternCount > 0U && currentSong_.patterns[0U] != nullptr) {
        isPlaying_ = true;
        patternIndex_ = 0U;
        rowIndex_ = 0U;
        currentTick_ = 0U;
        sampleAccumulator_ = 0.0f;
    } else {
        isPlaying_ = false;
    }
}

void MusicTracker::stop(std::array<ChiptuneVoice, 5>& voices) noexcept
{
    isPlaying_ = false;
    currentTrackId_ = ShooterCore::MusicTrackId::None;
    for (std::size_t c { 0U }; c < kVoiceCount; ++c) {
        voices[c].noteOff();
    }
}

void MusicTracker::advanceSample(std::array<ChiptuneVoice, 5>& voices, float sampleRate) noexcept
{
    if (!isPlaying_ || currentSong_.patternCount == 0U) {
        return;
    }

    const float safeRate { std::max(sampleRate, 1.0f) };
    const float denom { std::max(currentSong_.bpm * static_cast<float>(currentSong_.ticksPerRow), 1.0f) };
    samplesPerTick_ = (safeRate * 2.5f) / denom;

    sampleAccumulator_ += 1.0f;
    if (sampleAccumulator_ >= samplesPerTick_) {
        sampleAccumulator_ -= samplesPerTick_;

        if (currentTick_ == 0U) {
            triggerRow(voices);
        }

        ++currentTick_;
        if (currentTick_ >= currentSong_.ticksPerRow) {
            currentTick_ = 0U;
            ++rowIndex_;
            if (rowIndex_ >= TrackerPattern::kRowCount) {
                rowIndex_ = 0U;
                ++patternIndex_;
                if (patternIndex_ >= currentSong_.patternCount) {
                    if (currentSong_.loop) {
                        patternIndex_ = 0U;
                    } else {
                        stop(voices);
                    }
                }
            }
        }
    }
}

void MusicTracker::triggerRow(std::array<ChiptuneVoice, 5>& voices) noexcept
{
    if (patternIndex_ >= currentSong_.patternCount) {
        return;
    }

    const TrackerPattern* pattern { currentSong_.patterns[patternIndex_] };
    if (pattern == nullptr || rowIndex_ >= TrackerPattern::kRowCount) {
        return;
    }

    const TrackerRow& row { pattern->rows[rowIndex_] };

    for (std::size_t c { 0U }; c < kVoiceCount; ++c) {
        const TrackerCell& cell { row.channels[c] };
        if (cell.note == kNoteCut) {
            voices[c].noteOff();
        } else if (cell.note != kRest) {
            const float freq { noteToFrequency(cell.note) };
            const auto instIdx { static_cast<std::size_t>(std::min<std::uint8_t>(cell.instrument, 5U)) };
            const auto& inst { kInstruments[instIdx] };

            voices[c].setWaveform(inst.waveform);
            voices[c].setDutyCycle(inst.dutyCycle);
            voices[c].setSweepRate(inst.sweepRate);

            const float velocity { static_cast<float>(cell.volume) / 64.0f };
            // Scale velocity to master voice amplitude
            voices[c].noteOn(freq, velocity * 0.28f, inst.adsr);
        }
    }
}

} // namespace SdlAdapter
