#include "AudioCommandQueue.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>

#include "Types.h"

namespace ShooterCore {

const AudioCommand& AudioCommandQueue::get(std::size_t idx) const noexcept
{
    if (idx < count_) {
        return commands_[idx];
    }
    return commands_[0U];
}

bool AudioCommandQueue::pushSound(SoundEffectId id, float volume) noexcept
{
    if (count_ >= kCapacity) {
        return false;
    }

    AudioCommand& cmd { commands_[count_] };
    cmd.type = AudioCommandType::PlaySound;
    cmd.soundId = id;
    cmd.musicId = MusicTrackId::None;
    cmd.volume = std::clamp(volume, 0.0f, 1.0f);
    cmd.loop = false;

    ++count_;
    return true;
}

bool AudioCommandQueue::pushMusic(MusicTrackId id, bool loop) noexcept
{
    if (count_ >= kCapacity) {
        return false;
    }

    AudioCommand& cmd { commands_[count_] };
    cmd.type = AudioCommandType::PlayMusic;
    cmd.soundId = SoundEffectId::None;
    cmd.musicId = id;
    cmd.volume = 1.0f;
    cmd.loop = loop;

    ++count_;
    return true;
}

bool AudioCommandQueue::stopMusic() noexcept
{
    if (count_ >= kCapacity) {
        return false;
    }

    AudioCommand& cmd { commands_[count_] };
    cmd.type = AudioCommandType::StopMusic;
    cmd.soundId = SoundEffectId::None;
    cmd.musicId = MusicTrackId::None;
    cmd.volume = 0.0f;
    cmd.loop = false;

    ++count_;
    return true;
}

} // namespace ShooterCore
