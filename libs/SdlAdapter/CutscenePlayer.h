#pragma once

#include <cstddef>
#include <cstdint>

#include <array>
#include <string_view>

#include "SarArchiveReader.h"
#include "Types.h"

namespace SdlAdapter {

/// @enum class ArthurEmotion
/// @brief Emotional / situational portrait states for pilot Arthur.
enum class ArthurEmotion : std::uint8_t { Normal = 0U, Warning = 1U, Critical = 2U, Firing = 3U };

/// @class CutscenePlayer
/// @brief Unpacks, sequences, and animates Arthur anime portrait cutscenes (KAxx).
/// @details Pre-allocates raw frame buffers with strictly zero dynamic heap allocations (CERT ERR55-CPP).
class CutscenePlayer final {
public:
    static constexpr std::size_t kFrameWidth { 48U };
    static constexpr std::size_t kFrameHeight { 32U };
    static constexpr std::size_t kFrameBufferSize { kFrameWidth * kFrameHeight };
    static constexpr float kDefaultFps { 8.0f };

    CutscenePlayer() noexcept = default;

    /// @brief Load and unpack a keyframe asset (e.g. "KA00", "KA10") from an open SAR archive.
    /// @param[in] reader Open SAR archive containing target cutscene assets.
    /// @param[in] token 4-character resource token.
    /// @return true if cutscene frames were successfully decompressed.
    [[nodiscard]] bool loadCutscene(const SarArchiveReader& reader, std::string_view token) noexcept;

    /// @brief Advance frame timer and step blinking / lip-sync animation cycles.
    /// @param[in] dt Frame delta time.
    void update(ShooterCore::DeltaTime dt) noexcept;

    /// @brief Explicitly override current facial emotion state.
    /// @param[in] emotion Target ArthurEmotion.
    void setEmotion(ArthurEmotion emotion) noexcept;

    /// @brief Query active pilot emotion state.
    /// @return Current ArthurEmotion.
    [[nodiscard]] constexpr ArthurEmotion emotion() const noexcept
    {
        return emotion_;
    }

    /// @brief Query current animated sub-frame index (0: neutral, 1: mouth open, 2: blink).
    /// @return Frame index.
    [[nodiscard]] constexpr std::uint8_t currentFrameIndex() const noexcept
    {
        return currentFrameIdx_;
    }

    /// @brief Check whether valid cutscene data is loaded.
    /// @return true if cutscene buffer contains loaded data.
    [[nodiscard]] constexpr bool hasLoadedData() const noexcept
    {
        return hasData_;
    }

    /// @brief Access pointer to raw decompressed pixel buffer.
    /// @return Const pointer to decoded bytes.
    [[nodiscard]] const std::uint8_t* rawFrameData() const noexcept
    {
        return buffer_.data();
    }

private:
    std::array<std::uint8_t, 2048U> buffer_ { };
    std::size_t loadedSize_ { 0U };
    ArthurEmotion emotion_ { ArthurEmotion::Normal };
    float animTimer_ { 0.0f };
    std::uint8_t currentFrameIdx_ { 0U };
    bool hasData_ { false };
};

} // namespace SdlAdapter
