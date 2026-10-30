#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include <array>

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

        const ShooterCore::SoundEffectId allEffects[]
            = { ShooterCore::SoundEffectId::LaserFire, ShooterCore::SoundEffectId::LaserHit,
                  ShooterCore::SoundEffectId::ShieldDeploy, ShooterCore::SoundEffectId::ShieldDeflect,
                  ShooterCore::SoundEffectId::TransformJet, ShooterCore::SoundEffectId::TransformRobot,
                  ShooterCore::SoundEffectId::PlayerDamage, ShooterCore::SoundEffectId::EnemyExplode,
                  ShooterCore::SoundEffectId::BlockCrumble, ShooterCore::SoundEffectId::EnergyWarning };

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
}

} // namespace SdlAdapter::Test
