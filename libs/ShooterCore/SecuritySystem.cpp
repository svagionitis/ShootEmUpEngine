#include "SecuritySystem.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>

#include "AudioCommandQueue.h"
#include "Hitbox.h"
#include "Logger.h"
#include "RenderCommandQueue.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore {

bool SecuritySystem::registerGenerator(WorldPos pos, std::uint8_t barrierId) noexcept
{
    if (generatorCount_ >= kMaxGenerators) {
        LOG(WARNING) << "SecuritySystem: Generator capacity exhausted.";
        return false;
    }

    SecurityGenerator& gen { generators_[generatorCount_] };
    gen.pos = pos;
    gen.bounds = Hitbox { pos, WorldPos { pos.x + 32.0f, pos.y + 32.0f } };
    gen.health = GeneratorHealth { 100U };
    gen.linkedBarrierId = barrierId;
    gen.active = true;
    ++generatorCount_;
    return true;
}

bool SecuritySystem::registerBarrier(WorldPos pos, Hitbox bounds, std::uint8_t barrierId, std::uint8_t reqGen) noexcept
{
    if (barrierCount_ >= kMaxBarriers) {
        LOG(WARNING) << "SecuritySystem: Barrier capacity exhausted.";
        return false;
    }

    SecurityBarrier& barrier { barriers_[barrierCount_] };
    barrier.pos = pos;
    barrier.bounds = bounds;
    barrier.state = SecurityGateState::Locked;
    barrier.barrierId = barrierId;
    barrier.requiredGenerators = reqGen;
    barrier.destroyedGenerators = 0U;
    ++barrierCount_;
    return true;
}

bool SecuritySystem::damageAt(const Hitbox& hitArea, std::uint16_t damage, AudioCommandQueue& audioQueue) noexcept
{
    bool hitAny { false };

    for (std::size_t i { 0U }; i < generatorCount_; ++i) {
        SecurityGenerator& gen { generators_[i] };
        if (!gen.active) {
            continue;
        }

        if (hitArea.intersects(gen.bounds)) {
            gen.health.applyDamage(damage);
            hitAny = true;
            audioQueue.pushSound(SoundEffectId::LaserHit);

            if (!gen.health.isAlive()) {
                gen.active = false;
                audioQueue.pushSound(SoundEffectId::EnemyExplode);
                LOG(INFO) << "SecuritySystem: Generator " << i << " destroyed!";

                // Update linked barrier
                for (std::size_t b { 0U }; b < barrierCount_; ++b) {
                    SecurityBarrier& barrier { barriers_[b] };
                    if (barrier.barrierId == gen.linkedBarrierId) {
                        ++barrier.destroyedGenerators;
                        if (barrier.destroyedGenerators >= barrier.requiredGenerators) {
                            barrier.state = SecurityGateState::Open;
                            audioQueue.pushSound(SoundEffectId::BlockCrumble);
                            LOG(INFO) << "SecuritySystem: Forcefield barrier "
                                      << static_cast<unsigned>(barrier.barrierId) << " disengaged!";
                        }
                    }
                }
            }
        }
    }

    return hitAny;
}

void SecuritySystem::syncTiles(TileMap& map) noexcept
{
    const float tileSize { map.tileSize() };
    if (tileSize <= 0.0f) {
        return;
    }

    for (std::size_t b { 0U }; b < barrierCount_; ++b) {
        const SecurityBarrier& barrier { barriers_[b] };
        const TileCoord minCoord { barrier.bounds.min().toTileCoord(tileSize) };
        const TileCoord maxCoord { barrier.bounds.max().toTileCoord(tileSize) };

        const TileType targetType { (barrier.state == SecurityGateState::Open) ? TileType::Empty
                                                                               : TileType::Forcefield };

        for (std::uint32_t r { minCoord.row }; r <= maxCoord.row; ++r) {
            for (std::uint32_t c { minCoord.col }; c <= maxCoord.col; ++c) {
                map.setTile(TileCoord { c, r }, targetType);
            }
        }
    }
}

void SecuritySystem::emitRender(RenderCommandQueue& renderQueue) const noexcept
{
    // Draw active generators
    for (std::size_t i { 0U }; i < generatorCount_; ++i) {
        const SecurityGenerator& gen { generators_[i] };
        if (gen.active) {
            renderQueue.pushBar(gen.pos, gen.bounds.width(), gen.bounds.height(), 1.0f,
                ColorRgba { 255U, 220U, 40U, 255U }); // Yellow glowing generator core
        }
    }

    // Draw active barriers
    for (std::size_t b { 0U }; b < barrierCount_; ++b) {
        const SecurityBarrier& barrier { barriers_[b] };
        if (barrier.state != SecurityGateState::Open) {
            renderQueue.pushBar(barrier.pos, barrier.bounds.width(), barrier.bounds.height(), 1.0f,
                ColorRgba { 60U, 200U, 255U, 180U }); // Translucent cyan forcefield
        }
    }
}

bool SecuritySystem::isBarrierOpen(std::uint8_t barrierId) const noexcept
{
    for (std::size_t b { 0U }; b < barrierCount_; ++b) {
        if (barriers_[b].barrierId == barrierId) {
            return barriers_[b].state == SecurityGateState::Open;
        }
    }
    return true; // Unregistered barrier is considered open
}

std::size_t SecuritySystem::activeCount() const noexcept
{
    std::size_t count { 0U };
    for (std::size_t i { 0U }; i < generatorCount_; ++i) {
        if (generators_[i].active) {
            ++count;
        }
    }
    return count;
}

} // namespace ShooterCore
