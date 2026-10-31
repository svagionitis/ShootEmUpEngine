#include "MissionBriefingSystem.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <array>
#include <string_view>

#include "AudioCommandQueue.h"
#include "Types.h"

namespace ThexderGame {

void MissionBriefingSystem::startBriefing(std::uint8_t stageNum) noexcept
{
    stageNum_ = stageNum;
    page_ = 0U;
    isComplete_ = false;
    loadPageText();
}

void MissionBriefingSystem::loadPageText() noexcept
{
    charTimer_ = 0.0f;
    visibleChars_ = 0U;
    textBuffer_.fill('\0');

    std::string_view msg;
    if (stageNum_ == 1U) {
        msg = page_ == 0U ? "COMMAND: ORBITAL DEFENSE INITIATED.\nINFILTRATE ENEMY BATTLE CRUISER."
                          : "MISSION: DESTROY CENTRAL POWER CORE.\nSECONDARY MISSILES AUTHORIZED.";
    } else if (stageNum_ == 2U) {
        msg = "COMMAND: DESCENT TO SURFACE BASE.\nHOSTILE SWARMS DETECTED AHEAD.";
    } else if (stageNum_ == 9U) {
        msg = "COMMAND: FINAL ASSAULT ON NEDIAM CORE.\nALL SECONDARY WEAPONS UNLOCKED.";
    } else {
        msg = "COMMAND: PROCEED THROUGH SECTOR.\nELIMINATE ALL HOSTILE GENERATORS.";
    }

    totalChars_ = std::min(msg.size(), kMaxTextLength - 1U);
    for (std::size_t i { 0U }; i < totalChars_; ++i) {
        textBuffer_[i] = msg[i];
    }
}

void MissionBriefingSystem::update(ShooterCore::DeltaTime dt, ShooterCore::AudioCommandQueue& audio) noexcept
{
    if (isComplete_ || visibleChars_ >= totalChars_) {
        return;
    }

    const float dtSec { dt.seconds() };
    if (dtSec <= 0.0f) {
        return;
    }

    charTimer_ += dtSec;
    const float charInterval { 1.0f / kCharsPerSecond };

    while (charTimer_ >= charInterval && visibleChars_ < totalChars_) {
        charTimer_ -= charInterval;
        ++visibleChars_;
        if ((visibleChars_ % 3U) == 0U) {
            audio.pushSound(ShooterCore::SoundEffectId::WeaponSelect);
        }
    }
}

void MissionBriefingSystem::fastForward() noexcept
{
    visibleChars_ = totalChars_;
}

bool MissionBriefingSystem::advancePage() noexcept
{
    if (isComplete_) {
        return false;
    }

    if (!isPageRevealed()) {
        fastForward();
        return true;
    }

    if (stageNum_ == 1U && page_ == 0U) {
        ++page_;
        loadPageText();
        return true;
    }

    isComplete_ = true;
    return false;
}

std::string_view MissionBriefingSystem::visibleText() const noexcept
{
    if (visibleChars_ == 0U) {
        return "";
    }
    return std::string_view(textBuffer_.data(), visibleChars_);
}

} // namespace ThexderGame
