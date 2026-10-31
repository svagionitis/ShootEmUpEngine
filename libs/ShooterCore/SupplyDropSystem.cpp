#include "SupplyDropSystem.h"

#include <cmath>
#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <array>

#include "AudioCommandQueue.h"
#include "Hitbox.h"
#include "ParticlePool.h"
#include "SecondaryArsenal.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore {

bool SupplyDropSystem::spawnCanister(WorldPos pos, CanisterType type, Velocity initialVel) noexcept
{
    std::size_t freeIdx { kMaxCanisterSlots };
    for (std::size_t i { 0U }; i < kMaxCanisterSlots; ++i) {
        if (!canisters_[i].active) {
            freeIdx = i;
            break;
        }
    }

    if (freeIdx >= kMaxCanisterSlots) {
        return false;
    }

    CargoCanister& c { canisters_[freeIdx] };
    c.pos = pos;
    c.vel = initialVel;
    c.type = type;
    c.bobTimer = 0.0f;
    c.lifeTimer = 18.0f;
    c.onGround = false;
    c.magnetized = false;
    c.active = true;

    ++activeCount_;
    return true;
}

void SupplyDropSystem::update(DeltaTime dt, Hitbox playerBox, const TileMap& map, SecondaryArsenal& arsenal,
    EnergyPoints& playerEnergy, ShieldGauge& playerShield, AudioCommandQueue& audio, ParticlePool& particles) noexcept
{
    const float dtSec { dt.seconds() };
    if (dtSec <= 0.0f) {
        return;
    }

    const WorldPos playerCenter { playerBox.min().x + (playerBox.width() * 0.5f),
        playerBox.min().y + (playerBox.height() * 0.5f) };

    for (std::size_t i { 0U }; i < kMaxCanisterSlots; ++i) {
        CargoCanister& c { canisters_[i] };
        if (!c.active) {
            continue;
        }

        c.lifeTimer -= dtSec;
        if (c.lifeTimer <= 0.0f) {
            c.active = false;
            if (activeCount_ > 0U) {
                --activeCount_;
            }
            continue;
        }

        c.bobTimer += dtSec * 3.0f;

        // 1. Magnetic Proximity Attraction toward Player
        const float dxToPlayer { playerCenter.x - c.pos.x };
        const float dyToPlayer { playerCenter.y - c.pos.y };
        const float distToPlayer { std::sqrt((dxToPlayer * dxToPlayer) + (dyToPlayer * dyToPlayer)) };

        if (distToPlayer <= kMagnetRadius || c.magnetized) {
            c.magnetized = true;
            if (distToPlayer > 1.0f) {
                c.vel.dx = (dxToPlayer / distToPlayer) * kMagnetSpeed;
                c.vel.dy = (dyToPlayer / distToPlayer) * kMagnetSpeed;
            }
        } else if (!c.onGround) {
            // Apply gentle downward gravity
            c.vel.dy = std::min(kTerminalVelocity, c.vel.dy + (kGravity * dtSec));
            c.vel.dx *= std::max(0.0f, 1.0f - (1.5f * dtSec));
        }

        // Integrate Position
        c.pos.x += c.vel.dx * dtSec;
        c.pos.y += c.vel.dy * dtSec;

        // Ground collision (if not magnetized)
        if (!c.magnetized) {
            const TileCoord groundTile { c.pos.toTileCoord(16.0f) };
            if (map.isSolid(groundTile)) {
                c.pos.y = (static_cast<float>(groundTile.row) * 16.0f) - 6.0f;
                c.vel.dx = 0.0f;
                c.vel.dy = 0.0f;
                c.onGround = true;
            }
        }

        // 2. Collection Overlap Check
        const Hitbox canisterBox { WorldPos { c.pos.x - 6.0f, c.pos.y - 6.0f },
            WorldPos { c.pos.x + 6.0f, c.pos.y + 6.0f } };

        if (playerBox.intersects(canisterBox)) {
            // Apply Payload
            switch (c.type) {
            case CanisterType::MissilePack:
                arsenal.replenishMissiles(6U);
                break;
            case CanisterType::FlashBomb:
                arsenal.replenishBombs(1U);
                break;
            case CanisterType::EnergyBoost:
                playerEnergy.replenish(EnergyPoints { 30U });
                break;
            case CanisterType::ShieldRecharge:
                playerShield = ShieldGauge { 100U };
                break;
            case CanisterType::NapalmPack:
                arsenal.replenishNapalm(3U);
                break;
            case CanisterType::DeflectorPack:
                arsenal.replenishDeflector(2U);
                break;
            }

            audio.pushSound(SoundEffectId::CanisterPickup);
            particles.emitSparks(c.pos, 1.57f, 12U);

            c.active = false;
            if (activeCount_ > 0U) {
                --activeCount_;
            }
        }
    }
}

void SupplyDropSystem::clear() noexcept
{
    for (std::size_t i { 0U }; i < kMaxCanisterSlots; ++i) {
        canisters_[i] = CargoCanister { };
    }
    activeCount_ = 0U;
}

} // namespace ShooterCore
