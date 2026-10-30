#pragma once

#include <cstddef>
#include <cstdint>

#include <array>

#include "AudioCommandQueue.h"
#include "Types.h"

namespace SdlAdapter {

/// @enum ChannelWaveform
/// @brief Waveform generator type for an audio voice.
enum class ChannelWaveform : std::uint8_t { Pulse = 0U, Triangle = 1U, Noise = 2U };

/// @struct VoiceState
/// @brief Synthesis state for an individual procedural sound voice.
struct VoiceState final {
    bool active { false };
    ChannelWaveform waveform { ChannelWaveform::Pulse };
    float frequency { 440.0f };
    float targetFreq { 440.0f };
    float sweepRate { 0.0f };
    float phase { 0.0f };
    float dutyCycle { 0.5f };
    float amplitude { 0.0f };
    float decayRate { 0.0f };
    std::uint16_t lfsr { 0x7FFFU };
};

/// @struct MusicNote
/// @brief Representation of a procedural melodic note.
struct MusicNote final {
    float frequency { 0.0f };
    std::uint16_t durationTicks { 0U };
};

/// @class SdlAudio
/// @brief RAII audio device player and procedural 4-channel chiptune synthesizer.
class SdlAudio final {
public:
    static constexpr std::uint32_t kSampleRate { 44100U };
    static constexpr std::size_t kVoiceCount { 4U };

    SdlAudio() noexcept;
    ~SdlAudio() noexcept;

    SdlAudio(const SdlAudio&) = delete;
    SdlAudio& operator=(const SdlAudio&) = delete;

    SdlAudio(SdlAudio&& other) noexcept;
    SdlAudio& operator=(SdlAudio&& other) noexcept;

    /// @brief Initialize SDL audio device and begin synthesis playback.
    /// @return true if device opened successfully; false otherwise.
    [[nodiscard]] bool init() noexcept;

    /// @brief Shutdown audio device and stop playback thread.
    void shutdown() noexcept;

    /// @brief Ingest and dispatch frame audio commands emitted by ShooterCore.
    /// @param[in] queue Frame audio command queue.
    void processQueue(const ShooterCore::AudioCommandQueue& queue) noexcept;

    /// @brief Trigger a procedural sound effect preset.
    /// @param[in] effect Sound preset identifier.
    /// @param[in] volume Playback amplitude in range [0.0, 1.0].
    void playSound(ShooterCore::SoundEffectId effect, float volume = 1.0f) noexcept;

    /// @brief Trigger or change background music track.
    /// @param[in] track Track identifier.
    /// @param[in] loop Whether track repeats upon finishing.
    void playMusic(ShooterCore::MusicTrackId track, bool loop = true) noexcept;

    /// @brief Stop background music playback.
    void stopMusic() noexcept;

    /// @brief Synthesize samples directly into output buffer (invoked by audio callback or offline tests).
    /// @param[out] stream Destination 16-bit signed PCM buffer.
    /// @param[in] lengthInBytes Byte length of stream.
    void generatePcm(std::int16_t* stream, std::size_t lengthInBytes) noexcept;

    /// @brief Check whether audio device is open and active.
    [[nodiscard]] bool isValid() const noexcept;

private:
    static void audioCallback(void* userdata, std::uint8_t* stream, int len) noexcept;

    void updateMusicSequencer() noexcept;

    std::uint32_t deviceId_ { 0U };
    bool audioInitialized_ { false };
    std::array<VoiceState, kVoiceCount> voices_ { };

    ShooterCore::MusicTrackId currentMusic_ { ShooterCore::MusicTrackId::None };
    bool musicLoop_ { true };
    std::size_t musicStep_ { 0U };
    std::uint32_t musicTickCounter_ { 0U };
};

} // namespace SdlAdapter
