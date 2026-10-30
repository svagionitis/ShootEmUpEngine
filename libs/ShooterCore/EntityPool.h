#pragma once

#include <cstddef>
#include <cstdint>

#include <array>

#include "Hitbox.h"
#include "Types.h"

namespace ShooterCore {

/// @struct Entity
/// @brief Complete state data representation for an active simulation entity.
struct Entity final {
    EntityType type { EntityType::None };
    WorldPos pos { 0.0f, 0.0f };
    Velocity vel { 0.0f, 0.0f };
    Hitbox bounds { };
    std::uint16_t health { 1U };
    FacingDirection facing { FacingDirection::Left };
    float stateTimer { 0.0f };
    bool onGround { false };
};

/// @struct EntitySlot
/// @brief Internal pool element tracking entity state and generational recency.
struct EntitySlot final {
    Entity data { };
    std::uint16_t generation { 1U };
    bool active { false };
};

/// @class EntityPool
/// @brief Zero-heap generational object pool managing active game entities.
class EntityPool final {
public:
    static constexpr std::size_t kCapacity { 128U };

    EntityPool() noexcept;

    /// @brief Retrieve count of currently active entities.
    /// @return Active entity count.
    [[nodiscard]] std::size_t count() const noexcept
    {
        return activeCount_;
    }

    /// @brief Query maximum pool capacity.
    /// @return Maximum supported entities.
    [[nodiscard]] constexpr std::size_t capacity() const noexcept
    {
        return kCapacity;
    }

    /// @brief Check whether pool has reached maximum capacity.
    /// @return true if no free slots remain.
    [[nodiscard]] bool isFull() const noexcept
    {
        return activeCount_ >= kCapacity;
    }

    /// @brief Clear all active entities and reset free list.
    void clear() noexcept;

    /// @brief Allocate entity from pool.
    /// @param[in] type Entity classification category.
    /// @param[in] pos Continuous world position.
    /// @param[in] bounds Collision bounding box.
    /// @param[in] health Initial hit points.
    /// @return Generational EntityId handle (valid on success; invalid if pool full).
    EntityId spawn(EntityType type, WorldPos pos, Hitbox bounds, std::uint16_t health) noexcept;

    /// @brief Release entity back to pool.
    /// @param[in] id Generational entity handle.
    /// @return true if handle was valid and recycled; false otherwise.
    bool despawn(EntityId id) noexcept;

    /// @brief Retrieve mutable pointer to entity data if handle is valid.
    /// @param[in] id Generational handle.
    /// @return Pointer to entity, or nullptr if invalid or stale.
    [[nodiscard]] Entity* get(EntityId id) noexcept;

    /// @brief Retrieve const pointer to entity data if handle is valid.
    /// @param[in] id Generational handle.
    /// @return Const pointer to entity, or nullptr if invalid or stale.
    [[nodiscard]] const Entity* get(EntityId id) const noexcept;

    /// @brief Verify if handle is active and generation matches.
    /// @param[in] id Generational handle.
    /// @return true if active and recent.
    [[nodiscard]] bool isValid(EntityId id) const noexcept;

    /// @brief Get internal slot reference for direct index iteration.
    /// @param[in] idx Slot index.
    /// @return Const reference to slot.
    [[nodiscard]] const EntitySlot& slot(std::size_t idx) const noexcept
    {
        return slots_[idx < kCapacity ? idx : 0U];
    }

    /// @brief Get mutable internal slot reference for iteration.
    /// @param[in] idx Slot index.
    /// @return Mutable reference to slot.
    [[nodiscard]] EntitySlot& slot(std::size_t idx) noexcept
    {
        return slots_[idx < kCapacity ? idx : 0U];
    }

private:
    std::array<EntitySlot, kCapacity> slots_ { };
    std::array<std::uint16_t, kCapacity> freeIndices_ { };
    std::size_t freeCount_ { kCapacity };
    std::size_t activeCount_ { 0U };
};

} // namespace ShooterCore
