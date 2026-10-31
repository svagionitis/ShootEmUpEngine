#pragma once

#include <cstddef>
#include <cstdint>

namespace SdlAdapter {

/// @enum EnvelopeState
/// @brief 4-phase ADSR envelope operational state.
enum class EnvelopeState : std::uint8_t { Idle = 0U, Attack = 1U, Decay = 2U, Sustain = 3U, Release = 4U };

/// @struct AdsrConfig
/// @brief Timing and level parameters for an ADSR envelope.
struct AdsrConfig final {
    float attackSec { 0.010f }; ///< Duration in seconds to ramp to peak amplitude (1.0).
    float decaySec { 0.050f }; ///< Duration in seconds to decay from peak to sustain.
    float sustainLevel { 0.70f }; ///< Held amplitude ratio in range [0.0, 1.0].
    float releaseSec { 0.080f }; ///< Duration in seconds to fade to silence upon note release.
};

/// @class AdsrEnvelope
/// @brief Real-time 4-phase ADSR envelope generator with anti-click retriggering.
class AdsrEnvelope final {
public:
    constexpr AdsrEnvelope() noexcept = default;

    /// @brief Construct envelope with specific configuration parameters.
    /// @param[in] config Initial timing and level parameters.
    explicit constexpr AdsrEnvelope(const AdsrConfig& config) noexcept
        : config_ { config }
    {
    }

    /// @brief Configure envelope attack, decay, sustain, and release parameters.
    /// @param[in] config Target parameters.
    void setConfig(const AdsrConfig& config) noexcept;

    /// @brief Trigger note start (transitions to Attack, preserving current level to prevent clicks).
    void noteOn() noexcept;

    /// @brief Trigger note release (transitions to Release phase).
    void noteOff() noexcept;

    /// @brief Force immediate reset to Idle with zero amplitude.
    void reset() noexcept;

    /// @brief Advance envelope by one audio sample period.
    /// @param[in] sampleRate Output sampling frequency in Hertz.
    /// @return Current amplitude scale in range [0.0, 1.0].
    [[nodiscard]] float process(float sampleRate) noexcept;

    /// @brief Current operational phase of the envelope.
    /// @return Active EnvelopeState enum.
    [[nodiscard]] constexpr EnvelopeState state() const noexcept
    {
        return state_;
    }

    /// @brief Current instantaneous amplitude scale.
    /// @return Amplitude multiplier in range [0.0, 1.0].
    [[nodiscard]] constexpr float level() const noexcept
    {
        return level_;
    }

    /// @brief Check whether envelope is actively outputting sound.
    /// @return true if state is not Idle.
    [[nodiscard]] constexpr bool isActive() const noexcept
    {
        return state_ != EnvelopeState::Idle;
    }

private:
    AdsrConfig config_ { };
    EnvelopeState state_ { EnvelopeState::Idle };
    float level_ { 0.0f };
    float releaseStartLevel_ { 1.0f };
};

} // namespace SdlAdapter
