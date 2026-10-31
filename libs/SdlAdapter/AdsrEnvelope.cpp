#include "AdsrEnvelope.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>

namespace SdlAdapter {

void AdsrEnvelope::setConfig(const AdsrConfig& config) noexcept
{
    config_ = config;
    config_.attackSec = std::max(config_.attackSec, 0.0001f);
    config_.decaySec = std::max(config_.decaySec, 0.0001f);
    config_.sustainLevel = std::clamp(config_.sustainLevel, 0.0f, 1.0f);
    config_.releaseSec = std::max(config_.releaseSec, 0.0001f);
}

void AdsrEnvelope::noteOn() noexcept
{
    state_ = EnvelopeState::Attack;
    releaseStartLevel_ = 1.0f;
    if (config_.attackSec <= 0.0002f) {
        level_ = 1.0f;
        state_ = EnvelopeState::Decay;
    }
}

void AdsrEnvelope::noteOff() noexcept
{
    if (state_ != EnvelopeState::Idle) {
        state_ = EnvelopeState::Release;
        releaseStartLevel_ = level_;
    }
}

void AdsrEnvelope::reset() noexcept
{
    state_ = EnvelopeState::Idle;
    level_ = 0.0f;
    releaseStartLevel_ = 1.0f;
}

float AdsrEnvelope::process(float sampleRate) noexcept
{
    const float safeRate { std::max(sampleRate, 1.0f) };
    const float dt { 1.0f / safeRate };

    switch (state_) {
    case EnvelopeState::Idle:
        level_ = 0.0f;
        break;

    case EnvelopeState::Attack: {
        const float attackDelta { dt / std::max(config_.attackSec, 0.0001f) };
        level_ += attackDelta;
        if (level_ >= 1.0f) {
            level_ = 1.0f;
            state_ = EnvelopeState::Decay;
        }
        break;
    }

    case EnvelopeState::Decay: {
        const float target { std::clamp(config_.sustainLevel, 0.0f, 1.0f) };
        const float decayDelta { (1.0f - target) * (dt / std::max(config_.decaySec, 0.0001f)) };
        level_ -= decayDelta;
        if (level_ <= target) {
            level_ = target;
            state_ = EnvelopeState::Sustain;
        }
        break;
    }

    case EnvelopeState::Sustain:
        level_ = std::clamp(config_.sustainLevel, 0.0f, 1.0f);
        if (level_ <= 0.0001f) {
            level_ = 0.0f;
            state_ = EnvelopeState::Idle;
        }
        break;

    case EnvelopeState::Release: {
        const float releaseDelta { (releaseStartLevel_ * dt) / std::max(config_.releaseSec, 0.0001f) };
        level_ -= releaseDelta;
        if (level_ <= 0.0001f) {
            level_ = 0.0f;
            state_ = EnvelopeState::Idle;
        }
        break;
    }

    default:
        state_ = EnvelopeState::Idle;
        level_ = 0.0f;
        break;
    }

    return level_;
}

} // namespace SdlAdapter
