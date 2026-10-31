#include "SecretManager.h"

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

bool SecretManager::registerSecretWall(TileCoord coord) noexcept
{
    if (secretWallCount_ >= kMaxSecretWalls) {
        LOG(WARNING) << "SecretManager: Secret wall capacity exhausted.";
        return false;
    }

    secretWalls_[secretWallCount_] = coord;
    ++secretWallCount_;
    return true;
}

bool SecretManager::registerEnergyTank(WorldPos pos) noexcept
{
    if (tankCount_ >= kMaxEnergyTanks) {
        LOG(WARNING) << "SecretManager: Energy tank capacity exhausted.";
        return false;
    }

    tanks_[tankCount_] = EnergyTank { pos };
    ++tankCount_;
    return true;
}

bool SecretManager::damageSecretWall(TileCoord coord, TileMap& map, AudioCommandQueue& audio) noexcept
{
    if (!map.isSecret(coord)) {
        return false;
    }

    const bool destroyed { map.damageTile(coord) };
    if (destroyed) {
        audio.pushSound(SoundEffectId::WallCrumble);
        LOG(INFO) << "SecretManager: Secret wall collapsed at (" << coord.col << ", " << coord.row << ")!";
    } else {
        audio.pushSound(SoundEffectId::BlockCrumble);
    }
    return true;
}

bool SecretManager::checkPickup(
    const Hitbox& playerBox, EnergyPoints& playerEnergy, std::uint16_t& maxEnergy, AudioCommandQueue& audio) noexcept
{
    for (std::size_t i { 0U }; i < tankCount_; ++i) {
        EnergyTank& tank { tanks_[i] };
        if (tank.isCollected()) {
            continue;
        }

        const Hitbox tankBox { tank.position(), WorldPos { tank.position().x + 16.0f, tank.position().y + 16.0f } };

        if (playerBox.intersects(tankBox)) {
            tank.collect();
            const std::uint16_t boostedMax { static_cast<std::uint16_t>(
                std::min(static_cast<std::uint32_t>(EnergyTank::kMaxPlayerEnergy),
                    static_cast<std::uint32_t>(maxEnergy) + static_cast<std::uint32_t>(EnergyTank::kCapacityBonus))) };
            maxEnergy = boostedMax;
            playerEnergy = EnergyPoints { maxEnergy, maxEnergy };
            audio.pushSound(SoundEffectId::PowerUp);
            LOG(INFO) << "SecretManager: Energy Tank acquired! Max energy expanded to " << maxEnergy;
            return true;
        }
    }

    return false;
}

void SecretManager::emitRender(RenderCommandQueue& renderQueue) const noexcept
{
    for (std::size_t i { 0U }; i < tankCount_; ++i) {
        const EnergyTank& tank { tanks_[i] };
        if (!tank.isCollected()) {
            renderQueue.pushBar(tank.position(), 16.0f, 16.0f, 1.0f,
                ColorRgba { 50U, 240U, 120U, 255U }); // Vibrant energy tank capsule
        }
    }
}

} // namespace ShooterCore
