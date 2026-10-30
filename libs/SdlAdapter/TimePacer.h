#pragma once

#include <cstdint>

#include <chrono>

namespace SdlAdapter {

/// @class TimePacer
/// @brief High-precision frame pacer and 60 Hz fixed-timestep simulation accumulator.
class TimePacer final {
public:
    static constexpr float kFixedDeltaTime { 1.0f / 60.0f };
    static constexpr float kMaxAccumulatedTime { 0.25f };

    constexpr TimePacer() noexcept = default;

    /// @brief Start or reset the frame pacing clock.
    void start() noexcept;

    /// @brief Update frame timer and calculate accrued simulation ticks.
    /// @return Number of fixed 60 Hz simulation ticks to execute this frame.
    [[nodiscard]] std::uint32_t beginFrame() noexcept;

    /// @brief Query the fixed simulation delta time constant (1/60s).
    /// @return Fixed timestep in seconds.
    [[nodiscard]] constexpr float fixedDelta() const noexcept
    {
        return kFixedDeltaTime;
    }

    /// @brief Calculate fractional interpolation coefficient for sub-frame rendering.
    /// @return Alpha factor in range [0.0, 1.0].
    [[nodiscard]] constexpr float subframeAlpha() const noexcept
    {
        return accumulator_ / kFixedDeltaTime;
    }

    /// @brief Query the raw duration of the most recent frame in seconds.
    /// @return Delta time of previous frame in seconds.
    [[nodiscard]] constexpr float rawDelta() const noexcept
    {
        return lastFrameDelta_;
    }

    /// @brief Directly step accumulator by simulated delta time (useful for unit tests).
    /// @param[in] dt Delta time in seconds to accumulate.
    /// @return Number of ticks accumulated.
    [[nodiscard]] std::uint32_t stepSimulated(float dt) noexcept;

private:
    std::chrono::steady_clock::time_point lastTime_ { };
    float accumulator_ { 0.0f };
    float lastFrameDelta_ { 0.0f };
    bool running_ { false };
};

} // namespace SdlAdapter
