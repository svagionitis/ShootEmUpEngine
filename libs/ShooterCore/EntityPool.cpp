#include "EntityPool.h"

#include <cstddef>
#include <cstdint>

#include "Hitbox.h"
#include "Types.h"

namespace ShooterCore {

EntityPool::EntityPool() noexcept
{
    clear();
}

void EntityPool::clear() noexcept
{
    activeCount_ = 0U;
    freeCount_ = kCapacity;

    for (std::size_t i { 0U }; i < kCapacity; ++i) {
        slots_[i].active = false;
        slots_[i].generation = 1U;
        slots_[i].data = Entity { };
        freeIndices_[i] = static_cast<std::uint16_t>(i);
    }
}

EntityId EntityPool::spawn(EntityType type, WorldPos pos, Hitbox bounds, std::uint16_t health) noexcept
{
    if (freeCount_ == 0U) {
        return EntityId { };
    }

    --freeCount_;
    const std::uint16_t slotIdx { freeIndices_[freeCount_] };
    EntitySlot& slot { slots_[slotIdx] };

    slot.active = true;
    slot.data.type = type;
    slot.data.pos = pos;
    slot.data.vel = Velocity { 0.0f, 0.0f };
    slot.data.bounds = bounds;
    slot.data.health = health;
    slot.data.facing = FacingDirection::Left;
    slot.data.stateTimer = 0.0f;
    slot.data.onGround = false;

    ++activeCount_;

    return EntityId { slotIdx, slot.generation };
}

bool EntityPool::despawn(EntityId id) noexcept
{
    if (!isValid(id)) {
        return false;
    }

    const std::uint16_t idx { id.index() };
    EntitySlot& slot { slots_[idx] };

    slot.active = false;

    auto nextGen { static_cast<std::uint16_t>(slot.generation + 1U) };
    if (nextGen == 0U) {
        nextGen = 1U;
    }
    slot.generation = nextGen;

    freeIndices_[freeCount_] = idx;
    ++freeCount_;

    if (activeCount_ > 0U) {
        --activeCount_;
    }

    return true;
}

Entity* EntityPool::get(EntityId id) noexcept
{
    if (!isValid(id)) {
        return nullptr;
    }
    return &slots_[id.index()].data;
}

const Entity* EntityPool::get(EntityId id) const noexcept
{
    if (!isValid(id)) {
        return nullptr;
    }
    return &slots_[id.index()].data;
}

bool EntityPool::isValid(EntityId id) const noexcept
{
    if (!id.isValid()) {
        return false;
    }

    const std::size_t idx { static_cast<std::size_t>(id.index()) };
    if (idx >= kCapacity) {
        return false;
    }

    const EntitySlot& slot { slots_[idx] };
    return slot.active && (slot.generation == id.generation());
}

} // namespace ShooterCore
