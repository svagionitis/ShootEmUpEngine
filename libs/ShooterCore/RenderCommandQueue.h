#pragma once

#include <cstddef>
#include <cstdint>

#include <array>

#include "Hitbox.h"
#include "Types.h"

namespace ShooterCore {

/// @brief Tagged render command type discriminator.
enum class RenderCommandType : std::uint8_t {
    None = 0U,
    DrawSprite = 1U,
    DrawTile = 2U,
    DrawBeam = 3U,
    DrawBar = 4U,
    DrawLightning = 5U,
    DrawShield = 6U,
    DrawParticle = 7U,
    DrawShockwave = 8U
};

/// @struct RenderCommand
/// @brief Hardware-independent drawing instruction emitted by GameEngine.
struct RenderCommand final {
    RenderCommandType type { RenderCommandType::None };
    SpriteId sprite { SpriteId::None };
    TileType tileType { TileType::Empty };
    WorldPos pos { 0.0f, 0.0f };
    WorldPos endpoint { 0.0f, 0.0f };
    float width { 0.0f };
    float height { 0.0f };
    float fillPct { 0.0f };
    ColorRgba color { };
    ColorRgba glowColor { };
    FacingDirection facing { FacingDirection::Right };
    float rotationDegrees { 0.0f };
    std::uint8_t layer { 0U };
};

/// @class RenderCommandQueue
/// @brief Zero-heap fixed-capacity buffer storing draw calls for Tier 2 adapters.
/// @details Capacity is 2048 commands per frame, guaranteeing zero dynamic allocation.
class RenderCommandQueue final {
public:
    static constexpr std::size_t kCapacity { 2048U };

    constexpr RenderCommandQueue() noexcept = default;

    /// @brief Retrieve count of enqueued commands.
    /// @return Number of commands stored.
    [[nodiscard]] constexpr std::size_t count() const noexcept
    {
        return count_;
    }

    /// @brief Check whether queue has no stored commands.
    /// @return true if empty.
    [[nodiscard]] constexpr bool isEmpty() const noexcept
    {
        return count_ == 0U;
    }

    /// @brief Check whether queue reached maximum capacity.
    /// @return true if full.
    [[nodiscard]] constexpr bool isFull() const noexcept
    {
        return count_ >= kCapacity;
    }

    /// @brief Clear all enqueued render commands.
    constexpr void clear() noexcept
    {
        count_ = 0U;
        lightningArc_ = LightningArc { };
        shieldFx_ = ShieldFxState { };
    }

    /// @brief Retrieve active multi-segment lightning arc parameters.
    /// @return Const reference to active LightningArc struct.
    [[nodiscard]] constexpr const LightningArc& getLightning() const noexcept
    {
        return lightningArc_;
    }

    /// @brief Retrieve active shield forcefield presentation state.
    /// @return Const reference to active ShieldFxState struct.
    [[nodiscard]] constexpr const ShieldFxState& getShieldFx() const noexcept
    {
        return shieldFx_;
    }

    /// @brief Retrieve command by index.
    /// @param[in] idx Array index.
    /// @return Reference to command at index (or first element if out of bounds).
    [[nodiscard]] const RenderCommand& get(std::size_t idx) const noexcept;

    /// @brief Push sprite render command.
    /// @param[in] sprite Sprite graphic identifier.
    /// @param[in] pos Continuous world position.
    /// @param[in] facing Horizontal facing orientation.
    /// @param[in] layer Drawing layer for depth sorting.
    /// @param[in] rotationDegrees Optional rotation angle in degrees.
    /// @return true if successfully pushed; false if queue is full.
    bool pushSprite(SpriteId sprite, WorldPos pos, FacingDirection facing, std::uint8_t layer,
        float rotationDegrees = 0.0f) noexcept;

    /// @brief Push tile render command.
    /// @param[in] type Terrain material category.
    /// @param[in] pos World position of the tile.
    /// @param[in] layer Drawing layer.
    /// @return true if successfully pushed; false if queue is full.
    bool pushTile(TileType type, WorldPos pos, std::uint8_t layer) noexcept;

    /// @brief Push vector laser beam render command.
    /// @param[in] origin Beam starting position.
    /// @param[in] endpoint Beam terminating position.
    /// @param[in] color RGBA color of the beam.
    /// @return true if successfully pushed; false if queue is full.
    bool pushBeam(WorldPos origin, WorldPos endpoint, ColorRgba color) noexcept;

    /// @brief Push screen-space HUD bar command.
    /// @param[in] pos Screen coordinates of bar top-left.
    /// @param[in] width Total width in pixels.
    /// @param[in] height Total height in pixels.
    /// @param[in] pct Normalized fill fraction [0.0, 1.0].
    /// @param[in] color Fill color.
    /// @return true if successfully pushed; false if queue is full.
    bool pushBar(WorldPos pos, float width, float height, float pct, ColorRgba color) noexcept;

    /// @brief Push multi-segment lightning arc render command.
    /// @param[in] arc Multi-node lightning arc.
    /// @param[in] coreCol Intense inner core color.
    /// @param[in] glowCol Additive halo glow color.
    /// @param[in] layer Drawing layer.
    /// @return true if successfully pushed; false if queue is full.
    bool pushLightning(
        const LightningArc& arc, ColorRgba coreCol, ColorRgba glowCol, std::uint8_t layer = 12U) noexcept;

    /// @brief Push forcefield barrier plasma shield render command.
    /// @param[in] box Craft hitbox for shield boundary calculation.
    /// @param[in] fx Dynamic breathing and hit ripple parameters.
    /// @param[in] layer Drawing layer.
    /// @return true if successfully pushed; false if queue is full.
    bool pushShield(const Hitbox& box, const ShieldFxState& fx, std::uint8_t layer = 11U) noexcept;

    /// @brief Push visual particle render command.
    /// @param[in] pos Continuous world position.
    /// @param[in] color Particle color and opacity.
    /// @param[in] size Diameter in pixels.
    /// @param[in] layer Drawing layer.
    /// @return true if successfully pushed; false if queue is full.
    bool pushParticle(WorldPos pos, ColorRgba color, float size, std::uint8_t layer = 13U) noexcept;

    /// @brief Push expanding shockwave ring render command.
    /// @param[in] center Detonation center world position.
    /// @param[in] radius Expanding shockwave radius.
    /// @param[in] alpha Fade opacity [0.0, 1.0].
    /// @param[in] layer Drawing layer.
    /// @return true if successfully pushed; false if queue is full.
    bool pushShockwave(WorldPos center, float radius, float alpha, std::uint8_t layer = 14U) noexcept;

private:
    std::array<RenderCommand, kCapacity> commands_ { };
    std::size_t count_ { 0U };
    LightningArc lightningArc_ { };
    ShieldFxState shieldFx_ { };
};

} // namespace ShooterCore
