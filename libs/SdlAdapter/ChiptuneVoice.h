#pragma once

#include <cstddef>
#include <cstdint>

#include "AdsrEnvelope.h"

namespace SdlAdapter {

/// @enum ChannelWaveform
/// @brief Waveform generator type for a retro chiptune voice.
enum class ChannelWaveform : std::uint8_t {
    Pulse = 0U,
    Triangle = 1U,
    NoiseWhite = 2U,
    NoiseMetallic = 3U,
    FmOperator = 4U
};

/// @struct VoiceParams
/// @brief Initialization parameters for a chiptune synthesis voice.
struct VoiceParams final {
    ChannelWaveform waveform { ChannelWaveform::Pulse };
    float frequency { 440.0f };
    float sweepRate { 0.0f };
    float dutyCycle { 0.50f };
    float vibratoRate { 0.0f };
    float vibratoDepth { 0.0f };
    float fmRatio { 2.0f };
    float fmModIndex { 1.5f };
    float amplitude { 0.5f };
    AdsrConfig adsr { };
};

/// @class ChiptuneVoice
/// @brief Single polyphonic synthesis voice supporting Pulse PWM, Triangle, Dual-Mode LFSR Noise, and 2-Op FM.
class ChiptuneVoice final {
public:
    constexpr ChiptuneVoice() noexcept = default;

    /// @brief Trigger voice note-on with specific pitch, amplitude, and envelope.
    /// @param[in] freq Fundamental pitch frequency in Hertz.
    /// @param[in] amp Base volume scale [0.0, 1.0].
    /// @param[in] adsr Envelope configuration.
    void noteOn(float freq, float amp, const AdsrConfig& adsr) noexcept;

    /// @brief Trigger voice note-off (enters envelope release phase).
    void noteOff() noexcept;

    /// @brief Immediately silence and reset voice state.
    void reset() noexcept;

    /// @brief Synthesize a single floating-point audio sample in range [-1.0, 1.0].
    /// @param[in] sampleRate Output sampling frequency in Hertz.
    /// @return Audio sample amplitude.
    [[nodiscard]] float processSample(float sampleRate) noexcept;

    /// @brief Check whether voice is actively generating audio.
    /// @return true if envelope is active.
    [[nodiscard]] bool isActive() const noexcept;

    /// @brief Set active waveform generator type.
    /// @param[in] wf Desired ChannelWaveform.
    void setWaveform(ChannelWaveform wf) noexcept;

    /// @brief Set fundamental frequency in Hertz.
    /// @param[in] freq Frequency value.
    void setFrequency(float freq) noexcept;

    /// @brief Set linear pitch sweep rate in Hertz per second.
    /// @param[in] rate Sweep rate (positive or negative).
    void setSweepRate(float rate) noexcept;

    /// @brief Set pulse duty cycle for Pulse waveform.
    /// @param[in] duty Duty cycle ratio in range [0.05, 0.95].
    void setDutyCycle(float duty) noexcept;

    /// @brief Configure low-frequency pitch vibrato.
    /// @param[in] rate LFO frequency in Hertz.
    /// @param[in] depth Peak pitch deviation in Hertz.
    void setVibrato(float rate, float depth) noexcept;

    /// @brief Configure 2-operator FM synthesis parameters.
    /// @param[in] ratio Modulator to carrier frequency ratio.
    /// @param[in] modIndex Spectral brightness modulation depth.
    void setFmParams(float ratio, float modIndex) noexcept;

    /// @brief Set base voice amplitude scale.
    /// @param[in] amp Volume scale [0.0, 1.0].
    void setAmplitude(float amp) noexcept;

    /// @brief Access envelope generator.
    /// @return Reference to internal AdsrEnvelope.
    [[nodiscard]] AdsrEnvelope& envelope() noexcept
    {
        return envelope_;
    }

    /// @brief Const access to envelope generator.
    /// @return Const reference to internal AdsrEnvelope.
    [[nodiscard]] const AdsrEnvelope& envelope() const noexcept
    {
        return envelope_;
    }

    /// @brief Query active fundamental frequency.
    /// @return Current frequency in Hertz.
    [[nodiscard]] constexpr float frequency() const noexcept
    {
        return frequency_;
    }

private:
    ChannelWaveform waveform_ { ChannelWaveform::Pulse };
    float frequency_ { 440.0f };
    float sweepRate_ { 0.0f };
    float dutyCycle_ { 0.50f };
    float vibratoRate_ { 0.0f };
    float vibratoDepth_ { 0.0f };
    float fmRatio_ { 2.0f };
    float fmModIndex_ { 1.5f };
    float amplitude_ { 0.50f };

    float phase_ { 0.0f };
    float modPhase_ { 0.0f };
    float lfoPhase_ { 0.0f };
    std::uint16_t lfsr_ { 0x7FFFU };
    float noiseSample_ { 0.0f };
    float noiseClockAcc_ { 0.0f };

    AdsrEnvelope envelope_ { };
};

} // namespace SdlAdapter
