#pragma once

#include <cstddef>
#include <cstdint>

#include <algorithm>

namespace ShooterCore {

/// @brief Mechanical configuration of the player craft.
enum class TransformMode : std::uint8_t { Robot = 0U, Jet = 1U, TransformingToRobot = 2U, TransformingToJet = 3U };

/// @class EnergyPoints
/// @brief Strongly-typed player energy pool value object.
/// @details Represents the single energy pool used for lasers, shields, and life.
class EnergyPoints final {
public:
    static constexpr std::uint16_t kDefaultMax { 100U };

    /// @brief Construct zero energy points.
    constexpr EnergyPoints() noexcept
        : value_ { 0U }
    {
    }

    /// @brief Construct energy points clamped to maximum capacity.
    /// @param[in] val Desired energy amount.
    explicit constexpr EnergyPoints(std::uint16_t val) noexcept
        : value_ { std::min(val, kDefaultMax) }
    {
    }

    /// @brief Get raw numerical energy value.
    /// @return Current energy level.
    [[nodiscard]] constexpr std::uint16_t value() const noexcept
    {
        return value_;
    }

    /// @brief Check if energy is exhausted.
    /// @return true if zero; false otherwise.
    [[nodiscard]] constexpr bool isDepleted() const noexcept
    {
        return value_ == 0U;
    }

    /// @brief Check if pool can satisfy specified cost.
    /// @param[in] cost Amount required.
    /// @return true if available energy is greater than or equal to cost.
    [[nodiscard]] constexpr bool hasSufficient(EnergyPoints cost) const noexcept
    {
        return value_ >= cost.value_;
    }

    /// @brief Deduct energy with underflow protection.
    /// @param[in] cost Energy amount to remove.
    constexpr void deduct(EnergyPoints cost) noexcept
    {
        if (value_ <= cost.value_) {
            value_ = 0U;
            return;
        }
        value_ = static_cast<std::uint16_t>(value_ - cost.value_);
    }

    /// @brief Replenish energy clamped to maximum.
    /// @param[in] gain Energy amount to add.
    constexpr void replenish(EnergyPoints gain) noexcept
    {
        const std::uint32_t total { static_cast<std::uint32_t>(value_) + static_cast<std::uint32_t>(gain.value_) };
        const std::uint32_t maxVal { static_cast<std::uint32_t>(kDefaultMax) };
        value_ = static_cast<std::uint16_t>(std::min(total, maxVal));
    }

    /// @brief Equality comparison.
    [[nodiscard]] constexpr bool operator==(const EnergyPoints& other) const noexcept
    {
        return value_ == other.value_;
    }

    /// @brief Inequality comparison.
    [[nodiscard]] constexpr bool operator!=(const EnergyPoints& other) const noexcept
    {
        return value_ != other.value_;
    }

private:
    std::uint16_t value_ { 0U };
};

/// @class ShieldGauge
/// @brief Strongly-typed shield charge indicator.
class ShieldGauge final {
public:
    static constexpr std::uint8_t kMaxPercent { 100U };

    /// @brief Construct inactive shield gauge.
    constexpr ShieldGauge() noexcept
        : percentage_ { 0U }
    {
    }

    /// @brief Construct shield gauge clamped to 100%.
    /// @param[in] pct Percentage value.
    explicit constexpr ShieldGauge(std::uint8_t pct) noexcept
        : percentage_ { std::min(pct, kMaxPercent) }
    {
    }

    /// @brief Get shield percentage value.
    /// @return Percentage in range [0, 100].
    [[nodiscard]] constexpr std::uint8_t percentage() const noexcept
    {
        return percentage_;
    }

    /// @brief Check whether shield barrier is currently active.
    /// @return true if percentage is greater than zero.
    [[nodiscard]] constexpr bool isActive() const noexcept
    {
        return percentage_ > 0U;
    }

private:
    std::uint8_t percentage_ { 0U };
};

/// @struct TileCoord
/// @brief Discrete grid position in the tilemap.
struct TileCoord final {
    std::uint32_t col { 0U };
    std::uint32_t row { 0U };

    [[nodiscard]] constexpr bool operator==(const TileCoord& other) const noexcept
    {
        return col == other.col && row == other.row;
    }

    [[nodiscard]] constexpr bool operator!=(const TileCoord& other) const noexcept
    {
        return !(*this == other);
    }
};

/// @struct WorldPos
/// @brief Continuous 2D position in game world space.
struct WorldPos final {
    float x { 0.0f };
    float y { 0.0f };

    /// @brief Convert continuous world coordinate to discrete tile coordinate.
    /// @param[in] tileSize Dimension of a single square tile in world units.
    /// @return Discrete tile column and row.
    [[nodiscard]] constexpr TileCoord toTileCoord(float tileSize) const noexcept
    {
        if (tileSize <= 0.0f) {
            return TileCoord { 0U, 0U };
        }

        const float safeX { std::max(0.0f, x) };
        const float safeY { std::max(0.0f, y) };

        const auto colVal { static_cast<std::uint32_t>(safeX / tileSize) };
        const auto rowVal { static_cast<std::uint32_t>(safeY / tileSize) };

        return TileCoord { colVal, rowVal };
    }
};

/// @struct Velocity
/// @brief 2D velocity vector in world units per second.
struct Velocity final {
    float dx { 0.0f };
    float dy { 0.0f };
};

/// @class DamageValue
/// @brief Strongly-typed damage magnitude value object.
class DamageValue final {
public:
    constexpr DamageValue() noexcept
        : value_ { 0U }
    {
    }

    explicit constexpr DamageValue(std::uint16_t val) noexcept
        : value_ { val }
    {
    }

    [[nodiscard]] constexpr std::uint16_t value() const noexcept
    {
        return value_;
    }

private:
    std::uint16_t value_ { 0U };
};

/// @brief Classification of cell materials within the tilemap.
enum class TileType : std::uint8_t { Empty = 0U, SolidWall = 1U, DestructibleWall = 2U, Hazard = 3U };

/// @class TileHealth
/// @brief Durability state for destructible obstacle tiles.
class TileHealth final {
public:
    static constexpr std::uint8_t kDefaultMax { 3U };

    /// @brief Construct tile health initialized to maximum durability.
    constexpr TileHealth() noexcept
        : hits_ { kDefaultMax }
    {
    }

    /// @brief Construct tile health with explicit initial durability.
    /// @param[in] hits Number of impacts required to destroy.
    explicit constexpr TileHealth(std::uint8_t hits) noexcept
        : hits_ { std::min(hits, kDefaultMax) }
    {
    }

    /// @brief Get remaining hit points.
    /// @return Current health in range [0, kDefaultMax].
    [[nodiscard]] constexpr std::uint8_t value() const noexcept
    {
        return hits_;
    }

    /// @brief Check whether block is completely crumbled.
    /// @return true if zero hits remain.
    [[nodiscard]] constexpr bool isDestroyed() const noexcept
    {
        return hits_ == 0U;
    }

    /// @brief Deduct one hit point with underflow protection.
    constexpr void damage() noexcept
    {
        if (hits_ > 0U) {
            --hits_;
        }
    }

    /// @brief Reset durability to default maximum capacity.
    constexpr void reset() noexcept
    {
        hits_ = kDefaultMax;
    }

    [[nodiscard]] constexpr bool operator==(const TileHealth& other) const noexcept
    {
        return hits_ == other.hits_;
    }

    [[nodiscard]] constexpr bool operator!=(const TileHealth& other) const noexcept
    {
        return hits_ != other.hits_;
    }

private:
    std::uint8_t hits_ { kDefaultMax };
};

/// @brief Discrete collision surface normal direction.
enum class ContactNormal : std::uint8_t { None = 0U, Left = 1U, Right = 2U, Up = 3U, Down = 4U };

/// @struct SweepHit
/// @brief Result of continuous swept collision detection against geometry.
struct SweepHit final {
    bool collided { false };
    float toi { 1.0f };
    ContactNormal normal { ContactNormal::None };
    TileCoord hitTile { 0U, 0U };
    TileType tileType { TileType::Empty };
};

} // namespace ShooterCore
