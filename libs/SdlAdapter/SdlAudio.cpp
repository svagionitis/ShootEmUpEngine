#include "SdlAudio.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>

#include <SDL.h>

#include "AudioCommandQueue.h"
#include "Types.h"

namespace SdlAdapter {

namespace {

    // Moonlight Sonata / Thexder main melody snippet (Notes in Hz and durations in 60Hz ticks)
    constexpr std::array<MusicNote, 16> kThexderTheme { {
        { 220.00f, 12U }, // A3
        { 261.63f, 12U }, // C4
        { 329.63f, 12U }, // E4
        { 440.00f, 24U }, // A4
        { 415.30f, 12U }, // G#4
        { 440.00f, 12U }, // A4
        { 493.88f, 24U }, // B4
        { 523.25f, 36U }, // C5
        { 493.88f, 12U }, // B4
        { 440.00f, 12U }, // A4
        { 392.00f, 24U }, // G4
        { 349.23f, 12U }, // F4
        { 329.63f, 24U }, // E4
        { 293.66f, 12U }, // D4
        { 261.63f, 24U }, // C4
        { 220.00f, 36U } // A3
    } };

} // namespace

SdlAudio::SdlAudio() noexcept
{
    // Voice 0: Pulse 1 (Lead SFX & Melody)
    voices_[0].waveform = ChannelWaveform::Pulse;
    voices_[0].dutyCycle = 0.25f;

    // Voice 1: Pulse 2 (Shield / Alarms / Harmonies)
    voices_[1].waveform = ChannelWaveform::Pulse;
    voices_[1].dutyCycle = 0.50f;

    // Voice 2: Triangle (Basslines & Sub)
    voices_[2].waveform = ChannelWaveform::Triangle;

    // Voice 3: Noise (LFSR Percussion & Explosions)
    voices_[3].waveform = ChannelWaveform::Noise;
}

SdlAudio::~SdlAudio() noexcept
{
    shutdown();
}

SdlAudio::SdlAudio(SdlAudio&& other) noexcept
    : deviceId_ { other.deviceId_ }
    , audioInitialized_ { other.audioInitialized_ }
    , voices_ { other.voices_ }
    , currentMusic_ { other.currentMusic_ }
    , musicLoop_ { other.musicLoop_ }
    , musicStep_ { other.musicStep_ }
    , musicTickCounter_ { other.musicTickCounter_ }
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
        currentMusic_ = other.currentMusic_;
        musicLoop_ = other.musicLoop_;
        musicStep_ = other.musicStep_;
        musicTickCounter_ = other.musicTickCounter_;

        other.deviceId_ = 0U;
        other.audioInitialized_ = false;
    }
    return *this;
}

bool SdlAudio::init() noexcept
{
    shutdown();

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
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
        shutdown();
        return false;
    }

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
            playSound(cmd.soundId, cmd.volume);
            break;
        case ShooterCore::AudioCommandType::PlayMusic:
            playMusic(cmd.musicId, cmd.loop);
            break;
        case ShooterCore::AudioCommandType::StopMusic:
            stopMusic();
            break;
        case ShooterCore::AudioCommandType::StopSound:
        case ShooterCore::AudioCommandType::None:
        default:
            break;
        }
    }
}

void SdlAudio::playSound(ShooterCore::SoundEffectId effect, float volume) noexcept
{
    if (deviceId_ != 0U) {
        SDL_LockAudioDevice(deviceId_);
    }

    const float volClamped { std::min(std::max(volume, 0.0f), 1.0f) * 0.35f };

    switch (effect) {
    case ShooterCore::SoundEffectId::LaserFire: {
        // High-pitched rapid downward pitch sweep (Pulse 1, 25% duty)
        auto& v { voices_[0] };
        v.active = true;
        v.waveform = ChannelWaveform::Pulse;
        v.dutyCycle = 0.25f;
        v.frequency = 960.0f;
        v.sweepRate = -3600.0f; // Drops to 240 Hz in ~0.2s
        v.amplitude = volClamped;
        v.decayRate = volClamped / 0.16f; // 160 ms decay
        v.phase = 0.0f;
        break;
    }

    case ShooterCore::SoundEffectId::LaserHit: {
        // Short burst of noise + high ping
        auto& vn { voices_[3] };
        vn.active = true;
        vn.waveform = ChannelWaveform::Noise;
        vn.amplitude = volClamped * 0.8f;
        vn.decayRate = (volClamped * 0.8f) / 0.07f; // 70 ms
        break;
    }

    case ShooterCore::SoundEffectId::ShieldDeploy: {
        // Rising frequency shimmer (Pulse 2)
        auto& v { voices_[1] };
        v.active = true;
        v.waveform = ChannelWaveform::Pulse;
        v.dutyCycle = 0.50f;
        v.frequency = 320.0f;
        v.sweepRate = 2400.0f;
        v.amplitude = volClamped;
        v.decayRate = volClamped / 0.22f; // 220 ms
        v.phase = 0.0f;
        break;
    }

    case ShooterCore::SoundEffectId::ShieldDeflect: {
        // Resonant deflection chime
        auto& v { voices_[1] };
        v.active = true;
        v.waveform = ChannelWaveform::Pulse;
        v.dutyCycle = 0.125f;
        v.frequency = 1100.0f;
        v.sweepRate = -1500.0f;
        v.amplitude = volClamped * 1.2f;
        v.decayRate = (volClamped * 1.2f) / 0.12f;
        v.phase = 0.0f;
        break;
    }

    case ShooterCore::SoundEffectId::TransformJet: {
        // Ascending electronic arpeggio (Triangle sub)
        auto& v { voices_[2] };
        v.active = true;
        v.waveform = ChannelWaveform::Triangle;
        v.frequency = 180.0f;
        v.sweepRate = 1800.0f;
        v.amplitude = volClamped * 1.4f;
        v.decayRate = (volClamped * 1.4f) / 0.20f;
        v.phase = 0.0f;
        break;
    }

    case ShooterCore::SoundEffectId::TransformRobot: {
        // Descending electronic mechanical lock (Triangle sub)
        auto& v { voices_[2] };
        v.active = true;
        v.waveform = ChannelWaveform::Triangle;
        v.frequency = 540.0f;
        v.sweepRate = -1800.0f;
        v.amplitude = volClamped * 1.4f;
        v.decayRate = (volClamped * 1.4f) / 0.20f;
        v.phase = 0.0f;
        break;
    }

    case ShooterCore::SoundEffectId::PlayerDamage: {
        // Gritty low crunch
        auto& vn { voices_[3] };
        vn.active = true;
        vn.waveform = ChannelWaveform::Noise;
        vn.amplitude = volClamped * 1.3f;
        vn.decayRate = (volClamped * 1.3f) / 0.35f;

        auto& vp { voices_[0] };
        vp.active = true;
        vp.waveform = ChannelWaveform::Pulse;
        vp.dutyCycle = 0.5f;
        vp.frequency = 140.0f;
        vp.sweepRate = -200.0f;
        vp.amplitude = volClamped;
        vp.decayRate = volClamped / 0.25f;
        break;
    }

    case ShooterCore::SoundEffectId::EnemyExplode: {
        // Heavy low-pass decaying explosion rumble
        auto& vn { voices_[3] };
        vn.active = true;
        vn.waveform = ChannelWaveform::Noise;
        vn.amplitude = volClamped * 1.5f;
        vn.decayRate = (volClamped * 1.5f) / 0.45f; // 450 ms
        break;
    }

    case ShooterCore::SoundEffectId::BlockCrumble: {
        // Staccato crackle
        auto& vn { voices_[3] };
        vn.active = true;
        vn.waveform = ChannelWaveform::Noise;
        vn.amplitude = volClamped * 0.9f;
        vn.decayRate = (volClamped * 0.9f) / 0.14f;
        break;
    }

    case ShooterCore::SoundEffectId::EnergyWarning: {
        // High alert pulse beep
        auto& vp { voices_[1] };
        vp.active = true;
        vp.waveform = ChannelWaveform::Pulse;
        vp.dutyCycle = 0.50f;
        vp.frequency = 880.0f;
        vp.sweepRate = 0.0f;
        vp.amplitude = volClamped * 0.7f;
        vp.decayRate = (volClamped * 0.7f) / 0.10f;
        break;
    }

    case ShooterCore::SoundEffectId::None:
    default:
        break;
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

    currentMusic_ = track;
    musicLoop_ = loop;
    musicStep_ = 0U;
    musicTickCounter_ = 0U;

    if (deviceId_ != 0U) {
        SDL_UnlockAudioDevice(deviceId_);
    }
}

void SdlAudio::stopMusic() noexcept
{
    if (deviceId_ != 0U) {
        SDL_LockAudioDevice(deviceId_);
    }

    currentMusic_ = ShooterCore::MusicTrackId::None;
    musicStep_ = 0U;
    musicTickCounter_ = 0U;

    if (deviceId_ != 0U) {
        SDL_UnlockAudioDevice(deviceId_);
    }
}

void SdlAudio::updateMusicSequencer() noexcept
{
    if (currentMusic_ == ShooterCore::MusicTrackId::None) {
        return;
    }

    if (musicTickCounter_ > 0U) {
        --musicTickCounter_;
        return;
    }

    if (musicStep_ >= kThexderTheme.size()) {
        if (musicLoop_) {
            musicStep_ = 0U;
        } else {
            currentMusic_ = ShooterCore::MusicTrackId::None;
            return;
        }
    }

    const auto& note { kThexderTheme[musicStep_] };
    ++musicStep_;

    // Play melody note on Voice 2 (Triangle/Bass) or Voice 1 (Pulse)
    auto& v { voices_[2] };
    v.active = true;
    v.waveform = ChannelWaveform::Triangle;
    v.frequency = note.frequency;
    v.sweepRate = 0.0f;
    v.amplitude = 0.18f;
    // Length in samples: durationTicks * (kSampleRate / 60)
    const float durationSec { static_cast<float>(note.durationTicks) / 60.0f };
    v.decayRate = 0.18f / durationSec;
    v.phase = 0.0f;

    musicTickCounter_ = static_cast<std::uint32_t>(durationSec * static_cast<float>(kSampleRate));
}

void SdlAudio::generatePcm(std::int16_t* stream, std::size_t lengthInBytes) noexcept
{
    if (stream == nullptr || lengthInBytes == 0U) {
        return;
    }

    const std::size_t totalSamples { lengthInBytes / sizeof(std::int16_t) };
    const std::size_t frameCount { totalSamples / 2U }; // Stereo 2 channels

    for (std::size_t f { 0U }; f < frameCount; ++f) {
        updateMusicSequencer();

        float mix { 0.0f };

        for (std::size_t vi { 0U }; vi < kVoiceCount; ++vi) {
            auto& v { voices_[vi] };
            if (!v.active) {
                continue;
            }

            // Frequency sweep
            if (v.sweepRate != 0.0f) {
                v.frequency += v.sweepRate * (1.0f / static_cast<float>(kSampleRate));
                if (v.frequency < 20.0f || v.frequency > 18000.0f) {
                    v.active = false;
                    continue;
                }
            }

            // Phase accumulation
            const float phaseStep { v.frequency / static_cast<float>(kSampleRate) };
            v.phase = std::fmod(v.phase + phaseStep, 1.0f);

            float sample { 0.0f };
            switch (v.waveform) {
            case ChannelWaveform::Pulse:
                sample = (v.phase < v.dutyCycle) ? 1.0f : -1.0f;
                break;
            case ChannelWaveform::Triangle: {
                const float t { std::fmod(2.0f * v.phase + 1.0f, 2.0f) };
                sample = 2.0f * std::abs(t - 1.0f) - 1.0f;
                break;
            }
            case ChannelWaveform::Noise: {
                // 15-bit LFSR pseudo-random generator
                const std::uint16_t bit { static_cast<std::uint16_t>(((v.lfsr) ^ (v.lfsr >> 1U)) & 0x0001U) };
                v.lfsr = static_cast<std::uint16_t>((v.lfsr >> 1U) | (bit << 14U));
                sample = (bit == 1U) ? 1.0f : -1.0f;
                break;
            }
            default:
                break;
            }

            // Amplitude decay
            v.amplitude -= v.decayRate * (1.0f / static_cast<float>(kSampleRate));
            if (v.amplitude <= 0.0f) {
                v.amplitude = 0.0f;
                v.active = false;
            }

            mix += sample * v.amplitude;
        }

        // Hard clamp to range [-1.0, 1.0] to prevent PCM clipping
        const float clampedMix { std::min(std::max(mix, -1.0f), 1.0f) };
        const auto pcmVal { static_cast<std::int16_t>(clampedMix * 32767.0f) };

        stream[f * 2U] = pcmVal; // Left
        stream[f * 2U + 1U] = pcmVal; // Right
    }
}

} // namespace SdlAdapter
