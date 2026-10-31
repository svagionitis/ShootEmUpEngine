#include "DualResolutionRenderer.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>

namespace SdlAdapter {

void DualResolutionRenderer::computeLetterboxRect(std::int32_t winWidth, std::int32_t winHeight, std::int32_t& outX,
    std::int32_t& outY, std::int32_t& outW, std::int32_t& outH) noexcept
{
    outX = 0;
    outY = 0;
    outW = winWidth;
    outH = winHeight;

    if (winWidth <= 0 || winHeight <= 0) {
        return;
    }

    // Standard 4:3 DOS CRT aspect ratio
    constexpr float kTargetAspect { 4.0f / 3.0f };
    const float currentAspect { static_cast<float>(winWidth) / static_cast<float>(winHeight) };

    if (currentAspect > kTargetAspect) {
        // Pillarboxed (black bars on left/right)
        outH = winHeight;
        outW = static_cast<std::int32_t>(static_cast<float>(winHeight) * kTargetAspect);
        outX = (winWidth - outW) / 2;
    } else {
        // Letterboxed (black bars on top/bottom)
        outW = winWidth;
        outH = static_cast<std::int32_t>(static_cast<float>(winWidth) / kTargetAspect);
        outY = (winHeight - outH) / 2;
    }
}

} // namespace SdlAdapter
