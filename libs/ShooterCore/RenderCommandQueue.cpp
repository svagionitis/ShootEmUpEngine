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

bool RenderCommandQueue::pushLightning(
    const LightningArc& arc, ColorRgba coreCol, ColorRgba glowCol, std::uint8_t layer) noexcept
{
    if (count_ >= kCapacity) {
        return false;
    }

    lightningArc_ = arc;

    RenderCommand& cmd { commands_[count_] };
    cmd.type = RenderCommandType::DrawLightning;
    cmd.sprite = SpriteId::None;
    cmd.tileType = TileType::Empty;
    cmd.pos = arc.nodeCount > 0U ? arc.nodes[0U] : WorldPos { 0.0f, 0.0f };
    cmd.endpoint = arc.nodeCount > 0U ? arc.nodes[arc.nodeCount - 1U] : WorldPos { 0.0f, 0.0f };
    cmd.width = 0.0f;
    cmd.height = 0.0f;
    cmd.fillPct = 0.0f;
    cmd.color = coreCol;
    cmd.glowColor = glowCol;
    cmd.facing = FacingDirection::Right;
    cmd.rotationDegrees = 0.0f;
    cmd.layer = layer;

    ++count_;
    return true;
}

bool RenderCommandQueue::pushShield(const Hitbox& box, const ShieldFxState& fx, std::uint8_t layer) noexcept
{
    if (count_ >= kCapacity) {
        return false;
    }

    shieldFx_ = fx;

    RenderCommand& cmd { commands_[count_] };
    cmd.type = RenderCommandType::DrawShield;
    cmd.sprite = SpriteId::None;
    cmd.tileType = TileType::Empty;
    cmd.pos = box.min();
    cmd.endpoint = box.max();
    cmd.width = box.width();
    cmd.height = box.height();
    cmd.fillPct = 0.0f;
    cmd.color = ColorRgba { };
    cmd.glowColor = ColorRgba { };
    cmd.facing = FacingDirection::Right;
    cmd.rotationDegrees = 0.0f;
    cmd.layer = layer;

    ++count_;
    return true;
}

bool RenderCommandQueue::pushParticle(WorldPos pos, ColorRgba color, float size, std::uint8_t layer) noexcept
{
    if (count_ >= kCapacity) {
        return false;
    }

    RenderCommand& cmd { commands_[count_] };
    cmd.type = RenderCommandType::DrawParticle;
    cmd.sprite = SpriteId::None;
    cmd.tileType = TileType::Empty;
    cmd.pos = pos;
    cmd.endpoint = WorldPos { 0.0f, 0.0f };
    cmd.width = size;
    cmd.height = size;
    cmd.fillPct = 0.0f;
    cmd.color = color;
    cmd.glowColor = ColorRgba { };
    cmd.facing = FacingDirection::Right;
    cmd.rotationDegrees = 0.0f;
    cmd.layer = layer;

    ++count_;
    return true;
}

} // namespace ShooterCore
