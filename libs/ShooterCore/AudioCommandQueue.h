#pragma once

#include <cstddef>
#include <cstdint>

#include <array>

#include "Types.h"

namespace ShooterCore {

/// @brief Tagged audio command discriminator.
enum class AudioCommandType : std::uint8_t {
    None = 0U,
    PlaySound = 1U,
    StopSound = 2U,
    PlayMusic = 3U,
    StopMusic = 4U
};

/// @struct AudioCommand
/// @brief Decoupled audio trigger emitted by GameEngine.
struct AudioCommand final {
    AudioCommandType type { AudioCommandType::None };
    SoundEffectId soundId { SoundEffectId::None };
    MusicTrackId musicId { MusicTrackId::None };
    float volume { 1.0f };
    bool loop { false };
};

/// @class AudioCommandQueue
/// @brief Zero-heap fixed-capacity buffer storing audio triggers.
class AudioCommandQueue final {
public:
    static constexpr std::size_t kCapacity { 64U };

    constexpr AudioCommandQueue() noexcept = default;

    /// @brief Retrieve count of enqueued commands.
    /// @return Number of commands stored.
    [[nodiscard]] constexpr std::size_t count() const noexcept
    {
        return count_;
    }

    /// @brief Check whether queue has no stored commands.
    /// @return true if empty.
    [[nodiscard]] constexpr bool isEmpty() const noexcept
    {
        return count_ == 0U;
    }

    /// @brief Check whether queue reached maximum capacity.
    /// @return true if full.
    [[nodiscard]] constexpr bool isFull() const noexcept
    {
        return count_ >= kCapacity;
    }

    /// @brief Clear all enqueued audio commands.
    constexpr void clear() noexcept
    {
        count_ = 0U;
    }

    /// @brief Retrieve command by index.
    /// @param[in] idx Array index.
    /// @return Reference to command at index.
    [[nodiscard]] const AudioCommand& get(std::size_t idx) const noexcept;

    /// @brief Push sound effect playback command.
    /// @param[in] id Sound effect identifier.
    /// @param[in] volume Playback amplitude in range [0.0, 1.0].
    /// @return true if successfully pushed; false if queue is full.
    bool pushSound(SoundEffectId id, float volume = 1.0f) noexcept;

    /// @brief Push continuous looping sound effect playback command.
    /// @param[in] id Sound effect identifier.
    /// @param[in] volume Playback amplitude in range [0.0, 1.0].
    /// @return true if successfully pushed; false if queue is full.
    bool pushSoundLoop(SoundEffectId id, float volume = 1.0f) noexcept;

    /// @brief Push stop sound effect command.
    /// @param[in] id Sound effect identifier to halt.
    /// @return true if successfully pushed; false if queue is full.
    bool stopSound(SoundEffectId id) noexcept;

    /// @brief Push music track playback command.
    /// @param[in] id Music track identifier.
    /// @param[in] loop Whether track loops indefinitely.
    /// @return true if successfully pushed; false if queue is full.
    bool pushMusic(MusicTrackId id, bool loop = true) noexcept;

    /// @brief Push stop music command.
    /// @return true if successfully pushed; false if queue is full.
    bool stopMusic() noexcept;

private:
    std::array<AudioCommand, kCapacity> commands_ { };
    std::size_t count_ { 0U };
};

} // namespace ShooterCore
