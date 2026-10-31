#include "BossArenaSystem.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <array>

#include "AudioCommandQueue.h"
#include "BossEntity.h"
#include "Hitbox.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore {

void BossArenaSystem::configureArena(Hitbox bounds, float entryX, WorldPos exitPortal) noexcept
{
    arenaBounds_ = bounds;
    entryTriggerX_ = entryX;
    exitPortal_ = exitPortal;
    state_ = ArenaState::Dormant;
    blastDoorCount_ = 0U;
}

void BossArenaSystem::addBlastDoor(TileCoord coord) noexcept
{
    if (blastDoorCount_ < kMaxBlastDoors) {
        blastDoors_[blastDoorCount_++] = coord;
    }
}

void BossArenaSystem::reset() noexcept
{
    state_ = ArenaState::Dormant;
    blastDoorCount_ = 0U;
}

void BossArenaSystem::sealBlastDoors(TileMap& map, AudioCommandQueue& audio) noexcept
{
    for (std::size_t i { 0U }; i < blastDoorCount_; ++i) {
        map.setTile(blastDoors_[i], TileType::SolidWall);
    }
    audio.pushSound(SoundEffectId::SecurityAlarm);
}

void BossArenaSystem::openBlastDoors(TileMap& map, AudioCommandQueue& audio) noexcept
{
    for (std::size_t i { 0U }; i < blastDoorCount_; ++i) {
        map.setTile(blastDoors_[i], TileType::Empty);
    }
    audio.pushSound(SoundEffectId::PowerUp);
}

void BossArenaSystem::update(
    DeltaTime dt, WorldPos playerPos, const BossEntity& boss, TileMap& map, AudioCommandQueue& audio) noexcept
{
    (void)dt;
    if (state_ == ArenaState::Dormant) {
        if (playerPos.x >= entryTriggerX_ && playerPos.x <= arenaBounds_.max().x) {
            state_ = ArenaState::LockedDown;
            sealBlastDoors(map, audio);
        }
    } else if (state_ == ArenaState::LockedDown) {
        if (boss.isDefeated()) {
            state_ = ArenaState::Cleared;
            openBlastDoors(map, audio);
        }
    }
}

} // namespace ShooterCore
