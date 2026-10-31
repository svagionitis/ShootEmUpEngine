#pragma once

#include <cstdint>

namespace ShooterCore {

/// @class EngineInput
/// @brief Strongly-typed normalized input bitmask for discrete engine actions.
/// @details Strictly zero external dependencies per Tier 1 boundaries.
class EngineInput final {
public:
    static constexpr std::uint16_t kNone { 0x0000U };
    static constexpr std::uint16_t kUp { 0x0001U };
    static constexpr std::uint16_t kDown { 0x0002U };
    static constexpr std::uint16_t kLeft { 0x0004U };
    static constexpr std::uint16_t kRight { 0x0008U };
    static constexpr std::uint16_t kJump { 0x0010U };
    static constexpr std::uint16_t kTransform { 0x0020U };
    static constexpr std::uint16_t kFire { 0x0040U };
    static constexpr std::uint16_t kShield { 0x0080U };
    static constexpr std::uint16_t kFireSecondary { 0x0100U };
    static constexpr std::uint16_t kCycleWeapon { 0x0200U };

    constexpr EngineInput() noexcept = default;

    /// @brief Construct an input state with an explicit bitmask.
    /// @param[in] mask Bitmask combination of actions.
    explicit constexpr EngineInput(std::uint16_t mask) noexcept
        : mask_ { mask }
    {
    }

    /// @brief Retrieve underlying raw bitmask.
    /// @return Unsigned 16-bit bitmask.
    [[nodiscard]] constexpr std::uint16_t raw() const noexcept
    {
        return mask_;
    }

    /// @brief Check whether Up is pressed.
    [[nodiscard]] constexpr bool isUp() const noexcept
    {
        return (mask_ & kUp) != 0U;
    }

    /// @brief Check whether Down is pressed.
    [[nodiscard]] constexpr bool isDown() const noexcept
    {
        return (mask_ & kDown) != 0U;
    }

    /// @brief Check whether Left is pressed.
    [[nodiscard]] constexpr bool isLeft() const noexcept
    {
        return (mask_ & kLeft) != 0U;
    }

    /// @brief Check whether Right is pressed.
    [[nodiscard]] constexpr bool isRight() const noexcept
    {
        return (mask_ & kRight) != 0U;
    }

    /// @brief Check whether Jump is pressed.
    [[nodiscard]] constexpr bool isJump() const noexcept
    {
        return (mask_ & kJump) != 0U;
    }

    /// @brief Check whether Transform is pressed.
    [[nodiscard]] constexpr bool isTransform() const noexcept
    {
        return (mask_ & kTransform) != 0U;
    }

    /// @brief Check whether Fire is pressed.
    [[nodiscard]] constexpr bool isFire() const noexcept
    {
        return (mask_ & kFire) != 0U;
    }

    /// @brief Check whether Shield is pressed.
    [[nodiscard]] constexpr bool isShield() const noexcept
    {
        return (mask_ & kShield) != 0U;
    }

    /// @brief Check whether FireSecondary is pressed.
    [[nodiscard]] constexpr bool isFireSecondary() const noexcept
    {
        return (mask_ & kFireSecondary) != 0U;
    }

    /// @brief Check whether CycleWeapon is pressed.
    [[nodiscard]] constexpr bool isCycleWeapon() const noexcept
    {
        return (mask_ & kCycleWeapon) != 0U;
    }

    /// @brief Set a button bit in the bitmask.
    /// @param[in] btn Button flag bit.
    constexpr void setButton(std::uint16_t btn) noexcept
    {
        mask_ |= btn;
    }

    /// @brief Clear a button bit from the bitmask.
    /// @param[in] btn Button flag bit.
    constexpr void clearButton(std::uint16_t btn) noexcept
    {
        mask_ &= static_cast<std::uint16_t>(~btn);
    }

    /// @brief Clear all buttons to neutral state.
    constexpr void clear() noexcept
    {
        mask_ = kNone;
    }

    /// @brief Equality comparison.
    [[nodiscard]] constexpr bool operator==(const EngineInput& o) const noexcept
    {
        return mask_ == o.mask_;
    }

    /// @brief Inequality comparison.
    [[nodiscard]] constexpr bool operator!=(const EngineInput& o) const noexcept
    {
        return !(*this == o);
    }

private:
    std::uint16_t mask_ { kNone };
};

} // namespace ShooterCore
