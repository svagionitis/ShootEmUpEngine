#pragma once

#include <cstdint>

#include "EngineInput.h"

namespace SdlAdapter {

/// @class SdlInput
/// @brief Event polling engine translating SDL events to ShooterCore::EngineInput.
class SdlInput final {
public:
    constexpr SdlInput() noexcept = default;

    /// @brief Poll all pending SDL events and update input states.
    /// @return false if an SDL_QUIT or exit request was received; true otherwise.
    [[nodiscard]] bool pollEvents() noexcept;

    /// @brief Retrieve normalized engine input state for current frame.
    /// @return Current input bitmask representation.
    [[nodiscard]] constexpr ShooterCore::EngineInput currentInput() const noexcept
    {
        return currentInput_;
    }

    /// @brief Check if a specific input action was triggered on this frame (rising edge).
    /// @param[in] action Bitmask action flag to inspect.
    /// @return true if button transitioned from released to pressed.
    [[nodiscard]] constexpr bool isJustPressed(std::uint16_t action) const noexcept
    {
        return ((currentInput_.raw() & action) != 0U) && ((previousInput_.raw() & action) == 0U);
    }

    /// @brief Check if an input action was released on this frame (falling edge).
    /// @param[in] action Bitmask action flag to inspect.
    /// @return true if button transitioned from pressed to released.
    [[nodiscard]] constexpr bool isJustReleased(std::uint16_t action) const noexcept
    {
        return ((currentInput_.raw() & action) == 0U) && ((previousInput_.raw() & action) != 0U);
    }

    /// @brief Check whether quit was requested.
    /// @return true if exit was signaled.
    [[nodiscard]] constexpr bool quitRequested() const noexcept
    {
        return quitRequested_;
    }

    /// @brief Reset all input states to neutral.
    constexpr void reset() noexcept
    {
        currentInput_ = ShooterCore::EngineInput { };
        previousInput_ = ShooterCore::EngineInput { };
        quitRequested_ = false;
    }

private:
    void handleKeyEvent(std::int32_t keycode, bool isDown) noexcept;
    void handleButtonEvent(std::uint8_t button, bool isDown) noexcept;

    ShooterCore::EngineInput currentInput_ { };
    ShooterCore::EngineInput previousInput_ { };
    bool quitRequested_ { false };
};

} // namespace SdlAdapter
