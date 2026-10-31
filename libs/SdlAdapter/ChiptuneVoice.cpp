#include "ChiptuneVoice.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <cmath>

#include "AdsrEnvelope.h"

namespace SdlAdapter {

namespace {
    constexpr float kTwoPi { 6.28318530717958647692f };
} // namespace

void ChiptuneVoice::noteOn(float freq, float amp, const AdsrConfig& adsr) noexcept
{
    frequency_ = std::clamp(freq, 10.0f, 20000.0f);
    amplitude_ = std::clamp(amp, 0.0f, 1.0f);
    envelope_.setConfig(adsr);
    envelope_.noteOn();
}

void ChiptuneVoice::noteOff() noexcept
{
    envelope_.noteOff();
}

void ChiptuneVoice::reset() noexcept
{
    envelope_.reset();
    phase_ = 0.0f;
    modPhase_ = 0.0f;
    lfoPhase_ = 0.0f;
    noiseClockAcc_ = 0.0f;
    noiseSample_ = 0.0f;
}

bool ChiptuneVoice::isActive() const noexcept
{
    return envelope_.isActive();
}

void ChiptuneVoice::setWaveform(ChannelWaveform wf) noexcept
{
    waveform_ = wf;
}

void ChiptuneVoice::setFrequency(float freq) noexcept
{
    frequency_ = std::clamp(freq, 10.0f, 20000.0f);
}

void ChiptuneVoice::setSweepRate(float rate) noexcept
{
    sweepRate_ = rate;
}

void ChiptuneVoice::setDutyCycle(float duty) noexcept
{
    dutyCycle_ = std::clamp(duty, 0.05f, 0.95f);
}

void ChiptuneVoice::setVibrato(float rate, float depth) noexcept
{
    vibratoRate_ = std::max(rate, 0.0f);
    vibratoDepth_ = std::max(depth, 0.0f);
}

void ChiptuneVoice::setFmParams(float ratio, float modIndex) noexcept
{
    fmRatio_ = std::max(ratio, 0.1f);
    fmModIndex_ = std::max(modIndex, 0.0f);
}

void ChiptuneVoice::setAmplitude(float amp) noexcept
{
    amplitude_ = std::clamp(amp, 0.0f, 1.0f);
}

float ChiptuneVoice::processSample(float sampleRate) noexcept
{
    if (!envelope_.isActive()) {
        return 0.0f;
    }

    const float safeRate { std::max(sampleRate, 1.0f) };
    const float dt { 1.0f / safeRate };
    const float envLevel { envelope_.process(safeRate) };

    if (envLevel <= 0.0f) {
        return 0.0f;
    }

    // Advance frequency sweep
    if (sweepRate_ != 0.0f) {
        frequency_ += sweepRate_ * dt;
        if (frequency_ < 10.0f || frequency_ > 20000.0f) {
            envelope_.reset();
            return 0.0f;
        }
    }

    // Compute effective frequency with vibrato LFO
    float effectiveFreq { frequency_ };
    if (vibratoRate_ > 0.0f && vibratoDepth_ > 0.0f) {
        lfoPhase_ = std::fmod(lfoPhase_ + (vibratoRate_ * dt), 1.0f);
        const float lfo { std::sin(lfoPhase_ * kTwoPi) };
        effectiveFreq = std::clamp(frequency_ + (lfo * vibratoDepth_), 10.0f, 20000.0f);
    }

    // Advance fundamental phase accumulator
    const float phaseStep { effectiveFreq * dt };
    phase_ = std::fmod(phase_ + phaseStep, 1.0f);

    float rawSample { 0.0f };

    switch (waveform_) {
    case ChannelWaveform::Pulse:
        rawSample = (phase_ < dutyCycle_) ? 1.0f : -1.0f;
        break;

    case ChannelWaveform::Triangle: {
        const float t { std::fmod(2.0f * phase_ + 1.0f, 2.0f) };
        rawSample = 2.0f * std::abs(t - 1.0f) - 1.0f;
        break;
    }

    case ChannelWaveform::NoiseWhite: {
        noiseClockAcc_ += std::max(frequency_, 120.0f) * dt;
        while (noiseClockAcc_ >= 1.0f) {
            noiseClockAcc_ -= 1.0f;
            const auto bit { static_cast<std::uint16_t>(((lfsr_) ^ (lfsr_ >> 1U)) & 0x0001U) };
            lfsr_ = static_cast<std::uint16_t>((lfsr_ >> 1U) | (bit << 14U));
            noiseSample_ = (bit == 1U) ? 1.0f : -1.0f;
        }
        rawSample = noiseSample_;
        break;
    }

    case ChannelWaveform::NoiseMetallic: {
        noiseClockAcc_ += std::max(frequency_, 240.0f) * dt;
        while (noiseClockAcc_ >= 1.0f) {
            noiseClockAcc_ -= 1.0f;
            const auto bit { static_cast<std::uint16_t>(((lfsr_) ^ (lfsr_ >> 6U)) & 0x0001U) };
            lfsr_ = static_cast<std::uint16_t>((lfsr_ >> 1U) | (bit << 14U));
            noiseSample_ = (bit == 1U) ? 1.0f : -1.0f;
        }
        rawSample = noiseSample_;
        break;
    }

    case ChannelWaveform::FmOperator: {
        const float modStep { (effectiveFreq * fmRatio_) * dt };
        modPhase_ = std::fmod(modPhase_ + modStep, 1.0f);
        const float modOut { std::sin(modPhase_ * kTwoPi) };
        const float carPhase { phase_ * kTwoPi + (fmModIndex_ * modOut) };
        rawSample = std::sin(carPhase);
        break;
    }

    default:
        break;
    }

    return rawSample * amplitude_ * envLevel;
}

} // namespace SdlAdapter
