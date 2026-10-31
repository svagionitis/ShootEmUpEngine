#include "SdlAudio.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>

#include <SDL.h>
#include <glog/logging.h>

#include "AudioCommandQueue.h"
#include "ChiptuneVoice.h"
#include "MusicTracker.h"
#include "SoundPresets.h"
#include "Types.h"

namespace SdlAdapter {

SdlAudio::SdlAudio() noexcept
{
    // Voice 0: Pulse 1 (Lead SFX & Melody)
    voices_[0].setWaveform(ChannelWaveform::Pulse);
    voices_[0].setDutyCycle(0.25f);

    // Voice 1: Pulse 2 (Shield / Alarms / Harmonies)
    voices_[1].setWaveform(ChannelWaveform::Pulse);
    voices_[1].setDutyCycle(0.50f);

    // Voice 2: Triangle (Basslines & Sub-Bass)
    voices_[2].setWaveform(ChannelWaveform::Triangle);

    // Voice 3: LFSR Noise (Percussion & Explosions)
    voices_[3].setWaveform(ChannelWaveform::NoiseWhite);

    // Voice 4: 2-Operator FM (Metallic Bells & Boss Lasers)
    voices_[4].setWaveform(ChannelWaveform::FmOperator);
    voices_[4].setFmParams(3.5f, 2.0f);
}

SdlAudio::~SdlAudio() noexcept
{
    shutdown();
}

SdlAudio::SdlAudio(SdlAudio&& other) noexcept
    : deviceId_ { other.deviceId_ }
    , audioInitialized_ { other.audioInitialized_ }
    , voices_ { other.voices_ }
    , voicePriority_ { other.voicePriority_ }
    , voiceIsSfx_ { other.voiceIsSfx_ }
    , voiceSfxId_ { other.voiceSfxId_ }
    , musicTracker_ { other.musicTracker_ }
    , musicDuckingVolume_ { other.musicDuckingVolume_ }
{
    other.deviceId_ = 0U;
    other.audioInitialized_ = false;
}

SdlAudio& SdlAudio::operator=(SdlAudio&& other) noexcept
{
    if (this != &other) {
        shutdown();
        deviceId_ = other.deviceId_;
        audioInitialized_ = other.audioInitialized_;
        voices_ = other.voices_;
        voicePriority_ = other.voicePriority_;
        voiceIsSfx_ = other.voiceIsSfx_;
        voiceSfxId_ = other.voiceSfxId_;
        musicTracker_ = other.musicTracker_;
        musicDuckingVolume_ = other.musicDuckingVolume_;

        other.deviceId_ = 0U;
        other.audioInitialized_ = false;
    }
    return *this;
}

bool SdlAudio::init() noexcept
{
    shutdown();

    LOG(INFO) << "SdlAudio: Initializing 5-channel retro chiptune audio subsystem (44.1 kHz stereo)...";
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        LOG(ERROR) << "SdlAudio: Failed to initialize audio: " << SDL_GetError();
        return false;
    }
    audioInitialized_ = true;

    SDL_AudioSpec wanted { };
    wanted.freq = static_cast<int>(kSampleRate);
    wanted.format = AUDIO_S16SYS;
    wanted.channels = 2U;
    wanted.samples = 1024U;
    wanted.callback = &SdlAudio::audioCallback;
    wanted.userdata = this;

    SDL_AudioSpec obtained { };
    deviceId_ = SDL_OpenAudioDevice(nullptr, 0, &wanted, &obtained, 0);
    if (deviceId_ == 0U) {
        LOG(ERROR) << "SdlAudio: Failed to open audio device: " << SDL_GetError();
        shutdown();
        return false;
    }

    LOG(INFO) << "SdlAudio: Audio device opened successfully (ID: " << deviceId_ << ").";
    SDL_PauseAudioDevice(deviceId_, 0);
    return true;
}

void SdlAudio::shutdown() noexcept
{
    if (deviceId_ != 0U) {
        SDL_CloseAudioDevice(deviceId_);
        deviceId_ = 0U;
    }

    if (audioInitialized_) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        audioInitialized_ = false;
        LOG(INFO) << "SdlAudio: Audio subsystem released.";
    }
}

bool SdlAudio::isValid() const noexcept
{
    return deviceId_ != 0U;
}

void SdlAudio::audioCallback(void* userdata, std::uint8_t* stream, int len) noexcept
{
    if (userdata == nullptr || stream == nullptr || len <= 0) {
        return;
    }

    auto* self { static_cast<SdlAudio*>(userdata) };
    self->generatePcm(reinterpret_cast<std::int16_t*>(stream), static_cast<std::size_t>(len));
}

void SdlAudio::processQueue(const ShooterCore::AudioCommandQueue& queue) noexcept
{
    const std::size_t count { queue.count() };
    for (std::size_t i { 0U }; i < count; ++i) {
        const auto& cmd { queue.get(i) };
        switch (cmd.type) {
        case ShooterCore::AudioCommandType::PlaySound:
            if (cmd.loop) {
                playSoundLoop(cmd.soundId, cmd.volume);
            } else {
                playSound(cmd.soundId, cmd.volume);
            }
            break;
        case ShooterCore::AudioCommandType::StopSound:
            stopSound(cmd.soundId);
            break;
        case ShooterCore::AudioCommandType::PlayMusic:
            playMusic(cmd.musicId, cmd.loop);
            break;
        case ShooterCore::AudioCommandType::StopMusic:
            stopMusic();
            break;
        case ShooterCore::AudioCommandType::None:
        default:
            break;
        }
    }
}

void SdlAudio::playSound(ShooterCore::SoundEffectId effect, float volume) noexcept
{
    if (effect == ShooterCore::SoundEffectId::None) {
        return;
    }

    if (deviceId_ != 0U) {
        SDL_LockAudioDevice(deviceId_);
    }

    const SfxPreset preset { getSfxPreset(effect) };
    const std::size_t targetVoice { preset.preferredVoice % kVoiceCount };

    // Priority arbitration
    if (voiceIsSfx_[targetVoice] && preset.priority < voicePriority_[targetVoice]) {
        if (deviceId_ != 0U) {
            SDL_UnlockAudioDevice(deviceId_);
        }
        return;
    }

    voiceIsSfx_[targetVoice] = true;
    voicePriority_[targetVoice] = preset.priority;
    voiceSfxId_[targetVoice] = effect;

    auto& v { voices_[targetVoice] };
    v.setWaveform(preset.waveform);
    v.setDutyCycle(preset.dutyCycle);
    v.setSweepRate(preset.sweepRate);
    v.setVibrato(preset.vibratoRate, preset.vibratoDepth);
    v.setFmParams(preset.fmRatio, preset.fmModIndex);

    const float clampedVol { std::clamp(volume, 0.0f, 1.0f) * preset.volume };
    v.noteOn(preset.frequency, clampedVol, preset.adsr);

    if (preset.priority >= SfxPriority::Weapon) {
        musicDuckingVolume_ = 0.25f;
    }

    if (deviceId_ != 0U) {
        SDL_UnlockAudioDevice(deviceId_);
    }
}

void SdlAudio::playSoundLoop(ShooterCore::SoundEffectId effect, float volume) noexcept
{
    if (effect == ShooterCore::SoundEffectId::None) {
        return;
    }

    if (deviceId_ != 0U) {
        SDL_LockAudioDevice(deviceId_);
    }

    const SfxPreset preset { getSfxPreset(effect) };
    const std::size_t targetVoice { preset.preferredVoice % kVoiceCount };

    voiceIsSfx_[targetVoice] = true;
    voicePriority_[targetVoice] = preset.priority;
    voiceSfxId_[targetVoice] = effect;

    auto& v { voices_[targetVoice] };
    v.setWaveform(preset.waveform);
    v.setDutyCycle(preset.dutyCycle);
    v.setSweepRate(preset.sweepRate);
    v.setVibrato(preset.vibratoRate, preset.vibratoDepth);
    v.setFmParams(preset.fmRatio, preset.fmModIndex);

    const float clampedVol { std::clamp(volume, 0.0f, 1.0f) * preset.volume };
    v.noteOn(preset.frequency, clampedVol, preset.adsr);

    if (preset.priority >= SfxPriority::Weapon) {
        musicDuckingVolume_ = 0.35f;
    }

    if (deviceId_ != 0U) {
        SDL_UnlockAudioDevice(deviceId_);
    }
}

void SdlAudio::stopSound(ShooterCore::SoundEffectId effect) noexcept
{
    if (effect == ShooterCore::SoundEffectId::None) {
        return;
    }

    if (deviceId_ != 0U) {
        SDL_LockAudioDevice(deviceId_);
    }

    for (std::size_t i { 0U }; i < kVoiceCount; ++i) {
        if (voiceIsSfx_[i] && voiceSfxId_[i] == effect) {
            voices_[i].noteOff();
            voiceIsSfx_[i] = false;
            voicePriority_[i] = SfxPriority::Ambient;
            voiceSfxId_[i] = ShooterCore::SoundEffectId::None;
        }
    }

    if (deviceId_ != 0U) {
        SDL_UnlockAudioDevice(deviceId_);
    }
}

void SdlAudio::playMusic(ShooterCore::MusicTrackId track, bool loop) noexcept
{
    if (deviceId_ != 0U) {
        SDL_LockAudioDevice(deviceId_);
    }

    musicTracker_.playTrack(track, loop);

    if (deviceId_ != 0U) {
        SDL_UnlockAudioDevice(deviceId_);
    }
}

void SdlAudio::stopMusic() noexcept
{
    if (deviceId_ != 0U) {
        SDL_LockAudioDevice(deviceId_);
    }

    musicTracker_.stop(voices_);

    if (deviceId_ != 0U) {
        SDL_UnlockAudioDevice(deviceId_);
    }
}

void SdlAudio::generatePcm(std::int16_t* stream, std::size_t lengthInBytes) noexcept
{
    if (stream == nullptr || lengthInBytes == 0U) {
        return;
    }

    const std::size_t totalSamples { lengthInBytes / sizeof(std::int16_t) };
    const std::size_t frameCount { totalSamples / 2U }; // Stereo 2 channels
    const auto safeRate { static_cast<float>(kSampleRate) };

    for (std::size_t f { 0U }; f < frameCount; ++f) {
        musicTracker_.advanceSample(voices_, safeRate);

        // Smooth ducking volume recovery (~60 ms ramp)
        if (musicDuckingVolume_ < 1.0f) {
            musicDuckingVolume_ = std::min(musicDuckingVolume_ + 0.00035f, 1.0f);
        }

        // Release expired SFX voice locks
        for (std::size_t vi { 0U }; vi < kVoiceCount; ++vi) {
            if (voiceIsSfx_[vi] && !voices_[vi].isActive()) {
                voiceIsSfx_[vi] = false;
                voicePriority_[vi] = SfxPriority::Ambient;
                voiceSfxId_[vi] = ShooterCore::SoundEffectId::None;
            }
        }

        float mix { 0.0f };

        for (std::size_t vi { 0U }; vi < kVoiceCount; ++vi) {
            auto& v { voices_[vi] };
            if (!v.isActive()) {
                continue;
            }

            float sample { v.processSample(safeRate) };
            if (!voiceIsSfx_[vi]) {
                sample *= musicDuckingVolume_;
            }

            mix += sample;
        }

        // Fast smooth polynomial soft saturation limiter: y = x / (1 + 0.35 * |x|)
        const float x { mix };
        const float softClipped { x / (1.0f + (0.35f * std::abs(x))) };
        const float clampedMix { std::clamp(softClipped, -1.0f, 1.0f) };
        const auto pcmVal { static_cast<std::int16_t>(clampedMix * 32767.0f) };

        stream[f * 2U] = pcmVal; // Left
        stream[f * 2U + 1U] = pcmVal; // Right
    }
}

} // namespace SdlAdapter
