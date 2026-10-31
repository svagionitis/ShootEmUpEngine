#include "NestManager.h"

#include <cmath>
#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <array>

#include "AudioCommandQueue.h"
#include "EntityPool.h"
#include "Hitbox.h"
#include "ParticlePool.h"
#include "SupplyDropSystem.h"
#include "Types.h"

namespace ShooterCore {

bool NestManager::addNest(WorldPos pos, std::uint16_t health, float interval) noexcept
{
    if (nestCount_ >= kMaxNestSlots) {
        return false;
    }

    BioNest& n { nests_[nestCount_] };
    n.pos = pos;
    n.health = health;
    n.spawnInterval = interval;
    n.spawnTimer = 0.0f;
    n.active = true;
    n.destroyed = false;

    ++nestCount_;
    return true;
}

void NestManager::update(DeltaTime dt, WorldPos playerPos, EntityPool& entities, SupplyDropSystem& supplyDrops,
    ParticlePool& particles, AudioCommandQueue& audio) noexcept
{
    (void)supplyDrops;
    (void)audio;

    const float dtSec { dt.seconds() };
    if (dtSec <= 0.0f) {
        return;
    }

    for (std::size_t i { 0U }; i < nestCount_; ++i) {
        BioNest& n { nests_[i] };
        if (!n.active || n.destroyed) {
            continue;
        }

        const float dx { playerPos.x - n.pos.x };
        const float dy { playerPos.y - n.pos.y };
        const float dist { std::sqrt((dx * dx) + (dy * dy)) };

        const float currentInterval { dist <= kProximityRadius ? kEscalatedInterval : n.spawnInterval };
        n.spawnTimer += dtSec;

        if (n.spawnTimer >= currentInterval) {
            n.spawnTimer = 0.0f;
            if (!entities.isFull()) {
                // Alternate between DivingInterceptor and PatrolWalker
                const EntityType spawnType { (dropCounter_ % 2U == 0U) ? EntityType::DivingInterceptor
                                                                       : EntityType::PatrolWalker };

                const Hitbox box { WorldPos { n.pos.x - 8.0f, n.pos.y - 8.0f },
                    WorldPos { n.pos.x + 8.0f, n.pos.y + 8.0f } };

                const EntityId id { entities.spawn(spawnType, n.pos, box, 15U) };
                if (id.isValid()) {
                    Entity* e { entities.get(id) };
                    if (e != nullptr) {
                        e->vel = Velocity { (dx < 0.0f ? -60.0f : 60.0f), -40.0f };
                        e->facing = dx < 0.0f ? FacingDirection::Left : FacingDirection::Right;
                    }
                    particles.emitSparks(n.pos, 0.0f, 6U);
                }
            }
        }
    }
}

bool NestManager::damageNest(std::size_t idx, DamageValue dmg, SupplyDropSystem& supplyDrops, ParticlePool& particles,
    AudioCommandQueue& audio) noexcept
{
    if (idx >= nestCount_) {
        return false;
    }

    BioNest& n { nests_[idx] };
    if (!n.active || n.destroyed) {
        return false;
    }

    if (dmg.value() >= n.health) {
        n.health = 0U;
        n.destroyed = true;

        particles.emitSparks(n.pos, 0.0f, 24U);
        particles.emitDebris(n.pos, 12U);
        audio.pushSound(SoundEffectId::EnemyExplode);

        // Guaranteed tactical cargo canister drop
        const std::uint8_t dropPattern { static_cast<std::uint8_t>(dropCounter_ % 4U) };
        ++dropCounter_;

        CanisterType dropType { CanisterType::MissilePack };
        if (dropPattern == 1U) {
            dropType = CanisterType::EnergyBoost;
        } else if (dropPattern == 2U) {
            dropType = CanisterType::FlashBomb;
        } else if (dropPattern == 3U) {
            dropType = CanisterType::ShieldRecharge;
        }

        supplyDrops.spawnCanister(n.pos, dropType);
        return true;
    }

    n.health = static_cast<std::uint16_t>(n.health - dmg.value());
    particles.emitSparks(n.pos, 0.0f, 6U);
    return false;
}

void NestManager::clear() noexcept
{
    for (std::size_t i { 0U }; i < kMaxNestSlots; ++i) {
        nests_[i] = BioNest { };
    }
    nestCount_ = 0U;
    dropCounter_ = 0U;
}

} // namespace ShooterCore
