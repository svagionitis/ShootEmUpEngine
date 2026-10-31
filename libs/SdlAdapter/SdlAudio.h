#pragma once

#include <cstddef>
#include <cstdint>

#include <array>

#include "AudioCommandQueue.h"
#include "ChiptuneVoice.h"
#include "MusicTracker.h"
#include "SoundPresets.h"
#include "Types.h"

namespace SdlAdapter {

/// @class SdlAudio
/// @brief 5-channel retro chiptune synthesizer, music tracker, and priority sound effect mixer.
class SdlAudio final {
public:
    static constexpr std::uint32_t kSampleRate { 44100U };
    static constexpr std::size_t kVoiceCount { 5U };

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

    /// @brief Trigger a continuous looping sound effect (e.g. laser hum, shield drone).
    /// @param[in] effect Sound preset identifier.
    /// @param[in] volume Playback amplitude in range [0.0, 1.0].
    void playSoundLoop(ShooterCore::SoundEffectId effect, float volume = 1.0f) noexcept;

    /// @brief Stop a specific active sound effect.
    /// @param[in] effect Sound preset identifier to silence.
    void stopSound(ShooterCore::SoundEffectId effect) noexcept;

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

    /// @brief Query active music track.
    [[nodiscard]] constexpr ShooterCore::MusicTrackId currentMusic() const noexcept
    {
        return musicTracker_.currentTrack();
    }

private:
    static void audioCallback(void* userdata, std::uint8_t* stream, int len) noexcept;

    std::uint32_t deviceId_ { 0U };
    bool audioInitialized_ { false };

    std::array<ChiptuneVoice, kVoiceCount> voices_ { };
    std::array<SfxPriority, kVoiceCount> voicePriority_ { };
    std::array<bool, kVoiceCount> voiceIsSfx_ { };
    std::array<ShooterCore::SoundEffectId, kVoiceCount> voiceSfxId_ { };

    MusicTracker musicTracker_ { };
    float musicDuckingVolume_ { 1.0f };
};

} // namespace SdlAdapter
