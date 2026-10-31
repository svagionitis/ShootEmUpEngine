#pragma once

#include <cstddef>
#include <cstdint>

#include <array>

#include "AudioCommandQueue.h"
#include "BossEntity.h"
#include "Hitbox.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore {

/// @enum class ArenaState
/// @brief Status lifecycle of a boss encounter chamber.
enum class ArenaState : std::uint8_t { Dormant = 0U, LockedDown = 1U, Cleared = 2U };

/// @class BossArenaSystem
/// @brief Manages tactical boss arena perimeter lockdown, blast door seals, and exit warp portals.
/// @details Pre-allocates all blast door coordinates with strictly zero dynamic heap allocations (CERT ERR55-CPP).
class BossArenaSystem final {
public:
    static constexpr std::size_t kMaxBlastDoors { 8U };

    BossArenaSystem() noexcept = default;

    /// @brief Configure arena boundaries, trigger line, and exit portal position.
    /// @param[in] bounds World coordinates of arena bounding box.
    /// @param[in] entryX X coordinate threshold triggering chamber lockdown.
    /// @param[in] exitPortal World position where exit warp portal spawns on victory.
    void configureArena(Hitbox bounds, float entryX, WorldPos exitPortal) noexcept;

    /// @brief Register a blast door tile coordinate to be sealed during lockdown.
    /// @param[in] coord Tile grid coordinate.
    void addBlastDoor(TileCoord coord) noexcept;

    /// @brief Master frame tick checking player entry and boss defeat state transitions.
    /// @param[in] dt Frame delta time.
    /// @param[in] playerPos Mecha world coordinates.
    /// @param[in] boss Active modular boss entity.
    /// @param[in,out] map Tilemap modified for blast doors.
    /// @param[in,out] audio Audio command queue for lockdown alarm and victory fanfares.
    void update(
        DeltaTime dt, WorldPos playerPos, const BossEntity& boss, TileMap& map, AudioCommandQueue& audio) noexcept;

    /// @brief Check whether arena is currently in active combat lockdown.
    /// @return true if locked down.
    [[nodiscard]] constexpr bool isLockedDown() const noexcept
    {
        return state_ == ArenaState::LockedDown;
    }

    /// @brief Check whether arena encounter is successfully cleared.
    /// @return true if boss was defeated and doors unlocked.
    [[nodiscard]] constexpr bool isCleared() const noexcept
    {
        return state_ == ArenaState::Cleared;
    }

    /// @brief Query arena bounding box.
    /// @return Const reference to Hitbox bounds.
    [[nodiscard]] constexpr const Hitbox& arenaBounds() const noexcept
    {
        return arenaBounds_;
    }

    /// @brief Query world position of exit warp portal.
    /// @return Continuous WorldPos.
    [[nodiscard]] constexpr WorldPos exitPortal() const noexcept
    {
        return exitPortal_;
    }

    /// @brief Reset arena state and clear registered blast doors.
    void reset() noexcept;

private:
    void sealBlastDoors(TileMap& map, AudioCommandQueue& audio) noexcept;
    void openBlastDoors(TileMap& map, AudioCommandQueue& audio) noexcept;

    Hitbox arenaBounds_ { { 0.0f, 0.0f }, { 640.0f, 320.0f } };
    WorldPos exitPortal_ { 0.0f, 0.0f };
    float entryTriggerX_ { 0.0f };
    ArenaState state_ { ArenaState::Dormant };
    std::size_t blastDoorCount_ { 0U };
    std::array<TileCoord, kMaxBlastDoors> blastDoors_ { };
};

} // namespace ShooterCore
