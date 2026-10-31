#include "CutscenePlayer.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <array>
#include <string_view>

#include "SarArchiveReader.h"
#include "Types.h"

namespace SdlAdapter {

bool CutscenePlayer::loadCutscene(const SarArchiveReader& reader, std::string_view token) noexcept
{
    loadedSize_ = 0U;
    hasData_ = false;

    if (!reader.readEntryDecompressed(token, buffer_.data(), buffer_.size(), loadedSize_)) {
        return false;
    }

    hasData_ = loadedSize_ > 0U;
    animTimer_ = 0.0f;
    currentFrameIdx_ = 0U;
    return hasData_;
}

void CutscenePlayer::setEmotion(ArthurEmotion emotion) noexcept
{
    emotion_ = emotion;
}

void CutscenePlayer::update(ShooterCore::DeltaTime dt) noexcept
{
    const float dtSec { dt.seconds() };
    if (dtSec <= 0.0f) {
        return;
    }

    animTimer_ += dtSec;
    const float frameDuration { 1.0f / kDefaultFps };
    if (animTimer_ >= frameDuration) {
        animTimer_ -= frameDuration;
        // Cycle frame index: 0 (neutral) -> 1 (talking) -> 0 -> 2 (blinking) -> 0
        currentFrameIdx_ = static_cast<std::uint8_t>((currentFrameIdx_ + 1U) % 4U);
    }
}

} // namespace SdlAdapter
