#include "ParticlePool.h"

#include <cmath>
#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <array>

#include "Types.h"

namespace ShooterCore {

namespace {
    constexpr float kTwoPi { 6.2831853f };
}

float ParticlePool::randomFloat(float minVal, float maxVal) noexcept
{
    // Deterministic 32-bit Xorshift LCG
    rngState_ ^= (rngState_ << 13U);
    rngState_ ^= (rngState_ >> 17U);
    rngState_ ^= (rngState_ << 5U);

    const auto norm { static_cast<float>(rngState_ & 0x00FFFFFFU) / static_cast<float>(0x00FFFFFFU) };
    return minVal + (norm * (maxVal - minVal));
}

std::size_t ParticlePool::allocateSlot() noexcept
{
    // 1. Search next free slot starting from nextSlot_
    for (std::size_t i { 0U }; i < kCapacity; ++i) {
        const std::size_t slotIdx { (nextSlot_ + i) % kCapacity };
        if (!pool_[slotIdx].active) {
            nextSlot_ = (slotIdx + 1U) % kCapacity;
            ++activeCount_;
            return slotIdx;
        }
    }

    // 2. Wrap-around recycle if pool is fully saturated
    const std::size_t recycleSlot { nextSlot_ };
    nextSlot_ = (nextSlot_ + 1U) % kCapacity;
    return recycleSlot;
}

void ParticlePool::update(DeltaTime dt) noexcept
{
    const float dtSec { dt.seconds() };
    if (dtSec <= 0.0f) {
        return;
    }

    std::size_t currentActive { 0U };

    for (std::size_t i { 0U }; i < kCapacity; ++i) {
        Particle& p { pool_[i] };
        if (!p.active) {
            continue;
        }

        p.lifetime -= dtSec;
        if (p.lifetime <= 0.0f) {
            p.active = false;
            continue;
        }

        // Physics integration
        p.pos.x += p.vel.dx * dtSec;
        p.pos.y += p.vel.dy * dtSec;

        if (p.type == ParticleType::WallDebris) {
            // Apply gravity and drag to rubble chunks
            p.vel.dy += 160.0f * dtSec;
            p.vel.dx *= std::max(0.0f, 1.0f - (1.5f * dtSec));
        } else if (p.type == ParticleType::ImpactSpark) {
            // High atmospheric drag for incandescent sparks
            p.vel.dx *= std::max(0.0f, 1.0f - (3.5f * dtSec));
            p.vel.dy *= std::max(0.0f, 1.0f - (3.5f * dtSec));
        } else if (p.type == ParticleType::ShieldRipple) {
            p.vel.dx *= std::max(0.0f, 1.0f - (2.0f * dtSec));
            p.vel.dy *= std::max(0.0f, 1.0f - (2.0f * dtSec));
        }

        // Alpha decay
        if (p.maxLifetime > 0.0f) {
            const float frac { std::clamp(p.lifetime / p.maxLifetime, 0.0f, 1.0f) };
            p.color.a = static_cast<std::uint8_t>(frac * 255.0f);
        }

        ++currentActive;
    }

    activeCount_ = currentActive;
}

void ParticlePool::emitSparks(WorldPos origin, float normalAngle, std::size_t count) noexcept
{
    const std::size_t emitLimit { std::min(count, static_cast<std::size_t>(16U)) };
    for (std::size_t i { 0U }; i < emitLimit; ++i) {
        const std::size_t idx { allocateSlot() };
        Particle& p { pool_[idx] };

        // Spread angle +- 45 deg around normal
        const float angleOffset { randomFloat(-0.785f, 0.785f) };
        const float angle { normalAngle + angleOffset };
        const float speed { randomFloat(60.0f, 200.0f) };

        p.pos = origin;
        p.vel = Velocity { std::cos(angle) * speed, std::sin(angle) * speed };
        p.maxLifetime = randomFloat(0.12f, 0.28f);
        p.lifetime = p.maxLifetime;
        p.size = randomFloat(1.5f, 2.5f);
        p.type = ParticleType::ImpactSpark;
        p.color = ColorRgba { 255U, static_cast<std::uint8_t>(randomFloat(200.0f, 255.0f)), 80U, 255U };
        p.active = true;
    }
}

void ParticlePool::emitDebris(WorldPos origin, std::size_t count) noexcept
{
    const std::size_t emitLimit { std::min(count, static_cast<std::size_t>(12U)) };
    for (std::size_t i { 0U }; i < emitLimit; ++i) {
        const std::size_t idx { allocateSlot() };
        Particle& p { pool_[idx] };

        const float angle { randomFloat(0.0f, kTwoPi) };
        const float speed { randomFloat(30.0f, 110.0f) };

        p.pos = origin;
        p.vel = Velocity { std::cos(angle) * speed, std::sin(angle) * speed - 40.0f };
        p.maxLifetime = randomFloat(0.35f, 0.65f);
        p.lifetime = p.maxLifetime;
        p.size = randomFloat(2.0f, 3.5f);
        p.type = ParticleType::WallDebris;
        p.color = ColorRgba { 190U, 120U, 60U, 255U };
        p.active = true;
    }
}

void ParticlePool::emitShieldSparks(WorldPos origin, float hitAngle, std::size_t count) noexcept
{
    const std::size_t emitLimit { std::min(count, static_cast<std::size_t>(16U)) };
    for (std::size_t i { 0U }; i < emitLimit; ++i) {
        const std::size_t idx { allocateSlot() };
        Particle& p { pool_[idx] };

        const float angleOffset { randomFloat(-1.0f, 1.0f) };
        const float angle { hitAngle + angleOffset };
        const float speed { randomFloat(40.0f, 140.0f) };

        p.pos = origin;
        p.vel = Velocity { std::cos(angle) * speed, std::sin(angle) * speed };
        p.maxLifetime = randomFloat(0.15f, 0.35f);
        p.lifetime = p.maxLifetime;
        p.size = randomFloat(1.5f, 3.0f);
        p.type = ParticleType::ShieldRipple;
        p.color = ColorRgba { 80U, 210U, 255U, 255U };
        p.active = true;
    }
}

void ParticlePool::clear() noexcept
{
    for (std::size_t i { 0U }; i < kCapacity; ++i) {
        pool_[i] = Particle { };
    }
    activeCount_ = 0U;
    nextSlot_ = 0U;
}

} // namespace ShooterCore
