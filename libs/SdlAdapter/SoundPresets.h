#pragma once

#include <cstddef>
#include <cstdint>

#include "AdsrEnvelope.h"
#include "ChiptuneVoice.h"
#include "Types.h"

namespace SdlAdapter {

/// @enum SfxPriority
/// @brief Priority tier for arbitration and voice stealing.
enum class SfxPriority : std::uint8_t { Ambient = 0U, Feedback = 1U, Weapon = 2U, Critical = 3U };

/// @struct SfxPreset
/// @brief Synthesis definition for a procedural retro sound effect.
struct SfxPreset final {
    ShooterCore::SoundEffectId id { ShooterCore::SoundEffectId::None };
    SfxPriority priority { SfxPriority::Ambient };
    ChannelWaveform waveform { ChannelWaveform::Pulse };
    std::size_t preferredVoice { 0U };
    float frequency { 440.0f };
    float sweepRate { 0.0f };
    float dutyCycle { 0.50f };
    float vibratoRate { 0.0f };
    float vibratoDepth { 0.0f };
    float fmRatio { 2.0f };
    float fmModIndex { 1.5f };
    float volume { 1.0f };
    AdsrConfig adsr { };
};

/// @brief Retrieve static sound synthesis preset configuration for a sound effect ID.
/// @param[in] id Sound effect identifier.
/// @return Configured SfxPreset.
[[nodiscard]] inline SfxPreset getSfxPreset(ShooterCore::SoundEffectId id) noexcept
{
    SfxPreset p { };
    p.id = id;

    switch (id) {
    case ShooterCore::SoundEffectId::LaserFire:
        p.priority = SfxPriority::Weapon;
        p.waveform = ChannelWaveform::Pulse;
        p.preferredVoice = 0U;
        p.frequency = 960.0f;
        p.sweepRate = -3600.0f;
        p.dutyCycle = 0.25f;
        p.volume = 0.40f;
        p.adsr = { 0.002f, 0.140f, 0.0f, 0.030f };
        break;

    case ShooterCore::SoundEffectId::LaserLoop:
        p.priority = SfxPriority::Weapon;
        p.waveform = ChannelWaveform::Pulse;
        p.preferredVoice = 0U;
        p.frequency = 520.0f;
        p.dutyCycle = 0.25f;
        p.vibratoRate = 12.0f;
        p.vibratoDepth = 30.0f;
        p.volume = 0.35f;
        p.adsr = { 0.010f, 0.040f, 0.80f, 0.040f };
        break;

    case ShooterCore::SoundEffectId::LaserHit:
        p.priority = SfxPriority::Feedback;
        p.waveform = ChannelWaveform::NoiseMetallic;
        p.preferredVoice = 3U;
        p.frequency = 800.0f;
        p.volume = 0.45f;
        p.adsr = { 0.001f, 0.070f, 0.0f, 0.020f };
        break;

    case ShooterCore::SoundEffectId::ShieldDeploy:
        p.priority = SfxPriority::Weapon;
        p.waveform = ChannelWaveform::Pulse;
        p.preferredVoice = 1U;
        p.frequency = 320.0f;
        p.sweepRate = 2400.0f;
        p.dutyCycle = 0.50f;
        p.volume = 0.40f;
        p.adsr = { 0.005f, 0.200f, 0.0f, 0.040f };
        break;

    case ShooterCore::SoundEffectId::ShieldDrone:
        p.priority = SfxPriority::Weapon;
        p.waveform = ChannelWaveform::Pulse;
        p.preferredVoice = 1U;
        p.frequency = 120.0f;
        p.dutyCycle = 0.50f;
        p.vibratoRate = 6.0f;
        p.vibratoDepth = 8.0f;
        p.volume = 0.35f;
        p.adsr = { 0.020f, 0.050f, 0.75f, 0.050f };
        break;

    case ShooterCore::SoundEffectId::ShieldDeflect:
        p.priority = SfxPriority::Feedback;
        p.waveform = ChannelWaveform::Pulse;
        p.preferredVoice = 1U;
        p.frequency = 1100.0f;
        p.sweepRate = -1500.0f;
        p.dutyCycle = 0.125f;
        p.volume = 0.50f;
        p.adsr = { 0.002f, 0.120f, 0.0f, 0.020f };
        break;

    case ShooterCore::SoundEffectId::TransformJet:
        p.priority = SfxPriority::Feedback;
        p.waveform = ChannelWaveform::Triangle;
        p.preferredVoice = 2U;
        p.frequency = 180.0f;
        p.sweepRate = 1800.0f;
        p.volume = 0.55f;
        p.adsr = { 0.005f, 0.200f, 0.0f, 0.030f };
        break;

    case ShooterCore::SoundEffectId::TransformRobot:
        p.priority = SfxPriority::Feedback;
        p.waveform = ChannelWaveform::Triangle;
        p.preferredVoice = 2U;
        p.frequency = 540.0f;
        p.sweepRate = -1800.0f;
        p.volume = 0.55f;
        p.adsr = { 0.005f, 0.200f, 0.0f, 0.030f };
        break;

    case ShooterCore::SoundEffectId::PlayerDamage:
        p.priority = SfxPriority::Critical;
        p.waveform = ChannelWaveform::NoiseWhite;
        p.preferredVoice = 3U;
        p.frequency = 200.0f;
        p.volume = 0.60f;
        p.adsr = { 0.002f, 0.280f, 0.0f, 0.040f };
        break;

    case ShooterCore::SoundEffectId::PlayerDestroyed:
        p.priority = SfxPriority::Critical;
        p.waveform = ChannelWaveform::NoiseWhite;
        p.preferredVoice = 3U;
        p.frequency = 100.0f;
        p.volume = 0.75f;
        p.adsr = { 0.005f, 0.850f, 0.0f, 0.100f };
        break;

    case ShooterCore::SoundEffectId::EnemyExplode:
        p.priority = SfxPriority::Ambient;
        p.waveform = ChannelWaveform::NoiseWhite;
        p.preferredVoice = 3U;
        p.frequency = 160.0f;
        p.volume = 0.50f;
        p.adsr = { 0.002f, 0.380f, 0.0f, 0.050f };
        break;

    case ShooterCore::SoundEffectId::BlockCrumble:
    case ShooterCore::SoundEffectId::WallCrumble:
        p.priority = SfxPriority::Ambient;
        p.waveform = ChannelWaveform::NoiseWhite;
        p.preferredVoice = 3U;
        p.frequency = 400.0f;
        p.volume = 0.40f;
        p.adsr = { 0.002f, 0.120f, 0.0f, 0.020f };
        break;

    case ShooterCore::SoundEffectId::EnergyWarning:
        p.priority = SfxPriority::Critical;
        p.waveform = ChannelWaveform::Pulse;
        p.preferredVoice = 1U;
        p.frequency = 880.0f;
        p.dutyCycle = 0.50f;
        p.volume = 0.45f;
        p.adsr = { 0.002f, 0.080f, 0.0f, 0.020f };
        break;

    case ShooterCore::SoundEffectId::MissileLaunch:
        p.priority = SfxPriority::Weapon;
        p.waveform = ChannelWaveform::Pulse;
        p.preferredVoice = 0U;
        p.frequency = 660.0f;
        p.sweepRate = -800.0f;
        p.dutyCycle = 0.25f;
        p.volume = 0.45f;
        p.adsr = { 0.002f, 0.220f, 0.0f, 0.030f };
        break;

    case ShooterCore::SoundEffectId::FlashBombExplode:
        p.priority = SfxPriority::Critical;
        p.waveform = ChannelWaveform::NoiseWhite;
        p.preferredVoice = 3U;
        p.frequency = 80.0f;
        p.volume = 0.80f;
        p.adsr = { 0.005f, 0.650f, 0.0f, 0.080f };
        break;

    case ShooterCore::SoundEffectId::CanisterPickup:
        p.priority = SfxPriority::Feedback;
        p.waveform = ChannelWaveform::Pulse;
        p.preferredVoice = 0U;
        p.frequency = 587.33f;
        p.sweepRate = 1200.0f;
        p.dutyCycle = 0.50f;
        p.volume = 0.50f;
        p.adsr = { 0.002f, 0.220f, 0.0f, 0.030f };
        break;

    case ShooterCore::SoundEffectId::CrystalChime:
        p.priority = SfxPriority::Feedback;
        p.waveform = ChannelWaveform::FmOperator;
        p.preferredVoice = 4U;
        p.frequency = 1318.51f; // E6
        p.fmRatio = 3.5f;
        p.fmModIndex = 2.0f;
        p.volume = 0.45f;
        p.adsr = { 0.001f, 0.350f, 0.0f, 0.050f };
        break;

    case ShooterCore::SoundEffectId::EnemyShoot:
        p.priority = SfxPriority::Ambient;
        p.waveform = ChannelWaveform::Pulse;
        p.preferredVoice = 1U;
        p.frequency = 440.0f;
        p.sweepRate = -1000.0f;
        p.dutyCycle = 0.125f;
        p.volume = 0.35f;
        p.adsr = { 0.002f, 0.110f, 0.0f, 0.020f };
        break;

    case ShooterCore::SoundEffectId::WeaponSelect:
        p.priority = SfxPriority::Feedback;
        p.waveform = ChannelWaveform::Pulse;
        p.preferredVoice = 0U;
        p.frequency = 880.0f;
        p.dutyCycle = 0.50f;
        p.volume = 0.35f;
        p.adsr = { 0.002f, 0.060f, 0.0f, 0.015f };
        break;

    case ShooterCore::SoundEffectId::BossRoar:
        p.priority = SfxPriority::Critical;
        p.waveform = ChannelWaveform::FmOperator;
        p.preferredVoice = 4U;
        p.frequency = 80.0f;
        p.fmRatio = 1.414f;
        p.fmModIndex = 4.0f;
        p.volume = 0.65f;
        p.adsr = { 0.010f, 0.500f, 0.0f, 0.060f };
        break;

    case ShooterCore::SoundEffectId::BossDamage:
        p.priority = SfxPriority::Feedback;
        p.waveform = ChannelWaveform::NoiseMetallic;
        p.preferredVoice = 3U;
        p.frequency = 500.0f;
        p.volume = 0.50f;
        p.adsr = { 0.002f, 0.140f, 0.0f, 0.020f };
        break;

    case ShooterCore::SoundEffectId::BossExplode:
        p.priority = SfxPriority::Critical;
        p.waveform = ChannelWaveform::NoiseWhite;
        p.preferredVoice = 3U;
        p.frequency = 60.0f;
        p.volume = 0.85f;
        p.adsr = { 0.010f, 0.950f, 0.0f, 0.120f };
        break;

    case ShooterCore::SoundEffectId::PowerUp:
        p.priority = SfxPriority::Critical;
        p.waveform = ChannelWaveform::Pulse;
        p.preferredVoice = 0U;
        p.frequency = 440.0f;
        p.sweepRate = 1600.0f;
        p.dutyCycle = 0.25f;
        p.volume = 0.55f;
        p.adsr = { 0.005f, 0.350f, 0.0f, 0.050f };
        break;

    case ShooterCore::SoundEffectId::SecurityAlarm:
        p.priority = SfxPriority::Feedback;
        p.waveform = ChannelWaveform::Pulse;
        p.preferredVoice = 1U;
        p.frequency = 987.77f;
        p.sweepRate = -600.0f;
        p.dutyCycle = 0.50f;
        p.volume = 0.40f;
        p.adsr = { 0.002f, 0.120f, 0.0f, 0.020f };
        break;

    case ShooterCore::SoundEffectId::LaserStop:
    case ShooterCore::SoundEffectId::None:
    default:
        break;
    }

    return p;
}

} // namespace SdlAdapter
