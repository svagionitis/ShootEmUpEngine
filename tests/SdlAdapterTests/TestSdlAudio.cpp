#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <array>
#include <cmath>

#include "AudioCommandQueue.h"
#include "SdlAudio.h"
#include "Types.h"

namespace SdlAdapter::Test {

void runSdlAudioTests() noexcept
{
    // Test 1: Offline sample generation for all sound presets
    {
        SdlAudio audio;

        std::array<std::int16_t, 512> pcmBuffer { };

        const ShooterCore::SoundEffectId allEffects[] = { ShooterCore::SoundEffectId::LaserFire,
            ShooterCore::SoundEffectId::LaserLoop, ShooterCore::SoundEffectId::LaserHit,
            ShooterCore::SoundEffectId::ShieldDeploy, ShooterCore::SoundEffectId::ShieldDrone,
            ShooterCore::SoundEffectId::ShieldDeflect, ShooterCore::SoundEffectId::TransformJet,
            ShooterCore::SoundEffectId::TransformRobot, ShooterCore::SoundEffectId::PlayerDamage,
            ShooterCore::SoundEffectId::PlayerDestroyed, ShooterCore::SoundEffectId::EnemyExplode,
            ShooterCore::SoundEffectId::BlockCrumble, ShooterCore::SoundEffectId::WallCrumble,
            ShooterCore::SoundEffectId::EnergyWarning, ShooterCore::SoundEffectId::MissileLaunch,
            ShooterCore::SoundEffectId::FlashBombExplode, ShooterCore::SoundEffectId::CanisterPickup,
            ShooterCore::SoundEffectId::CrystalChime, ShooterCore::SoundEffectId::EnemyShoot,
            ShooterCore::SoundEffectId::WeaponSelect, ShooterCore::SoundEffectId::SecurityAlarm,
            ShooterCore::SoundEffectId::BossRoar, ShooterCore::SoundEffectId::BossDamage,
            ShooterCore::SoundEffectId::BossExplode, ShooterCore::SoundEffectId::PowerUp };

        for (const auto effect : allEffects) {
            pcmBuffer.fill(0);
            audio.playSound(effect, 1.0f);
            audio.generatePcm(pcmBuffer.data(), pcmBuffer.size() * sizeof(std::int16_t));

            // Verify that non-zero samples were produced
            bool hasNonZero { false };
            for (const auto sample : pcmBuffer) {
                if (sample != 0) {
                    hasNonZero = true;
                    break;
                }
            }
            TEST_ASSERT_TRUE(hasNonZero, "Sound effect should synthesize non-zero PCM audio samples");
        }
    }

    // Test 2: Audio Command Queue Ingestion
    {
        SdlAudio audio;
        ShooterCore::AudioCommandQueue queue;
        (void)queue.pushSound(ShooterCore::SoundEffectId::LaserFire, 0.8f);
        (void)queue.pushMusic(ShooterCore::MusicTrackId::MainTheme, true);

        audio.processQueue(queue);

        std::array<std::int16_t, 1024> buffer { };
        audio.generatePcm(buffer.data(), buffer.size() * sizeof(std::int16_t));

        bool hasAudio { false };
        for (const auto s : buffer) {
            if (s != 0) {
                hasAudio = true;
                break;
            }
        }
        TEST_ASSERT_TRUE(hasAudio, "processQueue should trigger active synthesis");

        audio.stopMusic();
    }

    // Test 3: Natural decay to silence
    {
        SdlAudio audio;
        audio.playSound(ShooterCore::SoundEffectId::LaserHit, 0.5f);

        std::array<std::int16_t, 2048> buffer { };
        // Step multiple blocks to let envelope decay (~0.1s is 4410 frames = ~8820 samples)
        for (std::size_t i { 0U }; i < 10U; ++i) {
            audio.generatePcm(buffer.data(), buffer.size() * sizeof(std::int16_t));
        }

        // Final block should have decayed to absolute silence
        audio.generatePcm(buffer.data(), buffer.size() * sizeof(std::int16_t));
        bool isSilent { true };
        for (const auto s : buffer) {
            if (s != 0) {
                isSilent = false;
                break;
            }
        }
        TEST_ASSERT_TRUE(isSilent, "After sufficient decay time, voice must decay to absolute silence");
    }

    // Test 4: Looping sound and stopSound command
    {
        SdlAudio audio;
        audio.playSoundLoop(ShooterCore::SoundEffectId::LaserLoop, 0.7f);

        std::array<std::int16_t, 1024> buffer { };
        // Advance multiple blocks; looping sound must remain sustained
        for (std::size_t i { 0U }; i < 8U; ++i) {
            audio.generatePcm(buffer.data(), buffer.size() * sizeof(std::int16_t));
        }

        bool stillPlaying { false };
        for (const auto s : buffer) {
            if (s != 0) {
                stillPlaying = true;
                break;
            }
        }
        TEST_ASSERT_TRUE(stillPlaying, "Looping sound must sustain audio without cutting off");

        // Stop the loop
        audio.stopSound(ShooterCore::SoundEffectId::LaserLoop);

        // Step past release phase (~40ms = ~1764 samples)
        for (std::size_t i { 0U }; i < 10U; ++i) {
            audio.generatePcm(buffer.data(), buffer.size() * sizeof(std::int16_t));
        }

        bool stopped { true };
        for (const auto s : buffer) {
            if (s != 0) {
                stopped = false;
                break;
            }
        }
        TEST_ASSERT_TRUE(stopped, "After stopSound and release phase, audio must become silent");
    }

    // Test 5: Soft saturation limiter prevents sample overflow
    {
        SdlAudio audio;
        // Trigger all high-amplitude presets simultaneously
        audio.playSound(ShooterCore::SoundEffectId::FlashBombExplode, 1.0f);
        audio.playSound(ShooterCore::SoundEffectId::BossExplode, 1.0f);
        audio.playSound(ShooterCore::SoundEffectId::LaserFire, 1.0f);
        audio.playSound(ShooterCore::SoundEffectId::ShieldDeploy, 1.0f);
        audio.playMusic(ShooterCore::MusicTrackId::BossBattle, true);

        std::array<std::int16_t, 4096> buffer { };
        audio.generatePcm(buffer.data(), buffer.size() * sizeof(std::int16_t));

        bool overflowDetected { false };
        for (const auto s : buffer) {
            // Check that samples stay strictly within 16-bit range without wrapping
            if (s > 32767 || s < -32767) {
                overflowDetected = true;
                break;
            }
        }
        TEST_ASSERT_FALSE(overflowDetected, "Master soft limiter must clamp all samples within [-32767, 32767]");
    }
}

} // namespace SdlAdapter::Test
