#include "RenderCommandQueue.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>

#include "Types.h"

namespace ShooterCore {

const RenderCommand& RenderCommandQueue::get(std::size_t idx) const noexcept
{
    if (idx < count_) {
        return commands_[idx];
    }
    return commands_[0U];
}

bool RenderCommandQueue::pushSprite(
    SpriteId sprite, WorldPos pos, FacingDirection facing, std::uint8_t layer, float rotationDegrees) noexcept
{
    if (count_ >= kCapacity) {
        return false;
    }

    RenderCommand& cmd { commands_[count_] };
    cmd.type = RenderCommandType::DrawSprite;
    cmd.sprite = sprite;
    cmd.tileType = TileType::Empty;
    cmd.pos = pos;
    cmd.endpoint = WorldPos { 0.0f, 0.0f };
    cmd.width = 0.0f;
    cmd.height = 0.0f;
    cmd.fillPct = 0.0f;
    cmd.color = ColorRgba { };
    cmd.facing = facing;
    cmd.rotationDegrees = rotationDegrees;
    cmd.layer = layer;

    ++count_;
    return true;
}

bool RenderCommandQueue::pushTile(TileType type, WorldPos pos, std::uint8_t layer) noexcept
{
    if (count_ >= kCapacity) {
        return false;
    }

    RenderCommand& cmd { commands_[count_] };
    cmd.type = RenderCommandType::DrawTile;
    cmd.sprite = SpriteId::None;
    cmd.tileType = type;
    cmd.pos = pos;
    cmd.endpoint = WorldPos { 0.0f, 0.0f };
    cmd.width = 0.0f;
    cmd.height = 0.0f;
    cmd.fillPct = 0.0f;
    cmd.color = ColorRgba { };
    cmd.facing = FacingDirection::Right;
    cmd.layer = layer;

    ++count_;
    return true;
}

bool RenderCommandQueue::pushBeam(WorldPos origin, WorldPos endpoint, ColorRgba color) noexcept
{
    if (count_ >= kCapacity) {
        return false;
    }

    RenderCommand& cmd { commands_[count_] };
    cmd.type = RenderCommandType::DrawBeam;
    cmd.sprite = SpriteId::None;
    cmd.tileType = TileType::Empty;
    cmd.pos = origin;
    cmd.endpoint = endpoint;
    cmd.width = 0.0f;
    cmd.height = 0.0f;
    cmd.fillPct = 0.0f;
    cmd.color = color;
    cmd.facing = FacingDirection::Right;
    cmd.layer = 100U;

    ++count_;
    return true;
}

bool RenderCommandQueue::pushBar(WorldPos pos, float width, float height, float pct, ColorRgba color) noexcept
{
    if (count_ >= kCapacity) {
        return false;
    }

    RenderCommand& cmd { commands_[count_] };
    cmd.type = RenderCommandType::DrawBar;
    cmd.sprite = SpriteId::None;
    cmd.tileType = TileType::Empty;
    cmd.pos = pos;
    cmd.endpoint = WorldPos { 0.0f, 0.0f };
    cmd.width = width;
    cmd.height = height;
    cmd.fillPct = std::clamp(pct, 0.0f, 1.0f);
    cmd.color = color;
    cmd.facing = FacingDirection::Right;
    cmd.layer = 255U;

    ++count_;
    return true;
}

} // namespace ShooterCore
