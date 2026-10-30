#include "TimePacer.h"

#include <cstdint>

#include <algorithm>
#include <chrono>

namespace SdlAdapter {

void TimePacer::start() noexcept
{
    lastTime_ = std::chrono::steady_clock::now();
    accumulator_ = 0.0f;
    lastFrameDelta_ = 0.0f;
    running_ = true;
}

std::uint32_t TimePacer::beginFrame() noexcept
{
    if (!running_) {
        start();
        return 0U;
    }

    const auto now = std::chrono::steady_clock::now();
    const auto elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(now - lastTime_).count();
    lastTime_ = now;

    const float frameSeconds = static_cast<float>(elapsedUs) / 1000000.0f;
    lastFrameDelta_ = frameSeconds;

    return stepSimulated(frameSeconds);
}

std::uint32_t TimePacer::stepSimulated(float dt) noexcept
{
    const float clampedDt = std::min(std::max(dt, 0.0f), kMaxAccumulatedTime);
    accumulator_ += clampedDt;

    std::uint32_t ticks { 0U };
    while (accumulator_ >= kFixedDeltaTime) {
        accumulator_ -= kFixedDeltaTime;
        ++ticks;
    }

    return ticks;
}

} // namespace SdlAdapter
