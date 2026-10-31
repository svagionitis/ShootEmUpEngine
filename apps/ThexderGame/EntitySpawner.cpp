#include "EntitySpawner.h"

#include <cstddef>
#include <cstdint>

#include "EntityPool.h"
#include "Hitbox.h"
#include "Types.h"

namespace ThexderGame {

namespace {
    constexpr float kActiveRadiusSq { 260.0f * 260.0f };
    constexpr SpawnerNode kDummyNode { };
} // namespace

EntitySpawner::EntitySpawner() noexcept
{
    initLevel1Nests();
}

void EntitySpawner::initLevel1Nests() noexcept
{
    nestCount_ = 0U;
    aerialWaveTimer_ = 0.0f;

    const auto addNest = [this](float x, float y, ShooterCore::EntityType type, float cooldown, std::uint16_t hp) {
        if (nestCount_ < kMaxNests) {
            nests_[nestCount_].pos = ShooterCore::WorldPos { x, y };
            nests_[nestCount_].spawnType = type;
            nests_[nestCount_].cooldownSec = cooldown;
            nests_[nestCount_].timerSec = 0.0f;
            nests_[nestCount_].health = hp;
            nests_[nestCount_].destroyed = false;
            ++nestCount_;
        }
    };

    // Authentic cavern nest positions across Level 1
    addNest(12.0f * 16.0f, 22.0f * 16.0f, ShooterCore::EntityType::PatrolWalker, 3.5f, 30U);
    addNest(28.0f * 16.0f, 22.0f * 16.0f, ShooterCore::EntityType::PatrolWalker, 3.0f, 30U);
    addNest(50.0f * 16.0f, 28.0f * 16.0f, ShooterCore::EntityType::HomingFlyer, 4.0f, 25U);
    addNest(72.0f * 16.0f, 16.0f * 16.0f, ShooterCore::EntityType::PatrolWalker, 3.5f, 30U);
    addNest(85.0f * 16.0f, 23.0f * 16.0f, ShooterCore::EntityType::HomingFlyer, 3.0f, 25U);
    addNest(105.0f * 16.0f, 27.0f * 16.0f, ShooterCore::EntityType::HomingFlyer, 4.0f, 25U);
    addNest(132.0f * 16.0f, 27.0f * 16.0f, ShooterCore::EntityType::PatrolWalker, 3.5f, 30U);
    addNest(145.0f * 16.0f, 27.0f * 16.0f, ShooterCore::EntityType::HomingFlyer, 3.0f, 35U);
}

void EntitySpawner::updateSpawners(
    ShooterCore::DeltaTime dt, ShooterCore::WorldPos playerPos, ShooterCore::EntityPool& pool) noexcept
{
    const float dtSec { dt.seconds() };

    for (std::size_t i { 0U }; i < nestCount_; ++i) {
        SpawnerNode& nest { nests_[i] };
        if (nest.destroyed) {
            continue;
        }

        const float dx { playerPos.x - nest.pos.x };
        const float dy { playerPos.y - nest.pos.y };
        const float distSq { (dx * dx) + (dy * dy) };

        if (distSq <= kActiveRadiusSq) {
            nest.timerSec += dtSec;
            if (nest.timerSec >= nest.cooldownSec) {
                nest.timerSec = 0.0f;

                // Safety invariant: Ensure pool has headroom before spawning
                if ((pool.capacity() - pool.count()) > 4U) {
                    const ShooterCore::Hitbox box { nest.pos,
                        ShooterCore::WorldPos { nest.pos.x + 16.0f, nest.pos.y + 16.0f } };
                    const auto hp { static_cast<std::uint16_t>(
                        (nest.spawnType == ShooterCore::EntityType::PatrolWalker) ? 2U : 1U) };
                    (void)pool.spawn(nest.spawnType, nest.pos, box, hp);
                }
            }
        }
    }

    // Periodic aerial waves in deep caverns
    aerialWaveTimer_ += dtSec;
    if (aerialWaveTimer_ >= 8.0f) {
        aerialWaveTimer_ = 0.0f;
        if ((pool.capacity() - pool.count()) > 4U) {
            const ShooterCore::WorldPos flyerPos { playerPos.x + 180.0f, playerPos.y - 32.0f };
            const ShooterCore::Hitbox fbox { flyerPos,
                ShooterCore::WorldPos { flyerPos.x + 16.0f, flyerPos.y + 16.0f } };
            (void)pool.spawn(ShooterCore::EntityType::HomingFlyer, flyerPos, fbox, 1U);
        }
    }
}

bool EntitySpawner::damageNest(std::size_t nestIndex, ShooterCore::DamageValue dmg) noexcept
{
    if (nestIndex >= nestCount_) {
        return false;
    }

    SpawnerNode& nest { nests_[nestIndex] };
    if (nest.destroyed) {
        return false;
    }

    if (nest.health <= dmg.value()) {
        nest.health = 0U;
        nest.destroyed = true;
        return true;
    }

    nest.health = static_cast<std::uint16_t>(nest.health - dmg.value());
    return false;
}

const SpawnerNode& EntitySpawner::getNest(std::size_t idx) const noexcept
{
    if (idx < nestCount_) {
        return nests_[idx];
    }
    return kDummyNode;
}

} // namespace ThexderGame
