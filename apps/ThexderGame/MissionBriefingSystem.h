#pragma once

#include <cstddef>
#include <cstdint>

#include <array>
#include <string_view>

#include "AudioCommandQueue.h"
#include "Types.h"

namespace ThexderGame {

/// @class MissionBriefingSystem
/// @brief Orchestrates narrative mission directives and teletype typewriter communications.
/// @details Operates with strictly zero dynamic heap allocations (CERT ERR55-CPP).
class MissionBriefingSystem final {
public:
    static constexpr float kCharsPerSecond { 30.0f };
    static constexpr std::size_t kMaxTextLength { 256U };

    MissionBriefingSystem() noexcept = default;

    /// @brief Start mission briefing transmission for target stage.
    /// @param[in] stageNum Stage number (1..9).
    void startBriefing(std::uint8_t stageNum) noexcept;

    /// @brief Advance teletype character reveal animation.
    /// @param[in] dt Frame delta time.
    /// @param[in,out] audio Audio command queue for teletype click chirps.
    void update(ShooterCore::DeltaTime dt, ShooterCore::AudioCommandQueue& audio) noexcept;

    /// @brief Instantly reveal all characters in the current transmission page.
    void fastForward() noexcept;

    /// @brief Advance to next transmission page or conclude briefing.
    /// @return true if advanced to next page; false if briefing finished.
    bool advancePage() noexcept;

    /// @brief Check whether briefing transmission is concluded.
    /// @return true if complete.
    [[nodiscard]] constexpr bool isComplete() const noexcept
    {
        return isComplete_;
    }

    /// @brief Check whether current page characters have all been revealed.
    /// @return true if current page text is fully shown.
    [[nodiscard]] constexpr bool isPageRevealed() const noexcept
    {
        return visibleChars_ >= totalChars_;
    }

    /// @brief Query visible slice of current transmission text.
    /// @return String view of revealed characters.
    [[nodiscard]] std::string_view visibleText() const noexcept;

    /// @brief Query active stage being briefed.
    /// @return Stage number (1..9).
    [[nodiscard]] constexpr std::uint8_t stageNumber() const noexcept
    {
        return stageNum_;
    }

private:
    void loadPageText() noexcept;

    std::array<char, kMaxTextLength> textBuffer_ { };
    float charTimer_ { 0.0f };
    std::size_t visibleChars_ { 0U };
    std::size_t totalChars_ { 0U };
    std::uint8_t stageNum_ { 1U };
    std::uint8_t page_ { 0U };
    bool isComplete_ { true };
};

} // namespace ThexderGame
