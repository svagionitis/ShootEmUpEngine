#pragma once

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <array>

namespace ShooterCore {

/// @brief Mechanical configuration of the player craft.
enum class TransformMode : std::uint8_t { Robot = 0U, Jet = 1U, TransformingToRobot = 2U, TransformingToJet = 3U };

/// @brief Result codes returned by transformation requests.
enum class TransformResult : std::uint8_t { Success = 0U, RejectedObstruction = 1U, AlreadyInState = 2U, Busy = 3U };

/// @brief Geometric anchor mode used to project hitboxes during transformation.
enum class AnchorMode : std::uint8_t { BottomCenter = 0U, Center = 1U, TopLeft = 2U };

/// @class DeltaTime
/// @brief Strongly-typed simulation timestep duration in seconds.
class DeltaTime final {
public:
    constexpr DeltaTime() noexcept = default;

    /// @brief Construct delta time in seconds, clamped to non-negative values.
    /// @param[in] seconds Timestep duration.
    explicit constexpr DeltaTime(float seconds) noexcept
        : seconds_ { seconds > 0.0f ? seconds : 0.0f }
    {
    }

    /// @brief Get duration in seconds.
    /// @return Non-negative seconds value.
    [[nodiscard]] constexpr float seconds() const noexcept
    {
        return seconds_;
    }

private:
    float seconds_ { 0.0f };
};

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

/// @brief Discrete facing orientation of the player craft.
enum class FacingDirection : std::uint8_t { Left = 0U, Right = 1U, Up = 2U, Down = 3U };

/// @brief Target category classification for auto-aim priority.
enum class TargetType : std::uint8_t { Hostile = 0U, Destructible = 1U };

/// @struct BeamSegment
/// @brief Spatial coordinates and state of the emitted laser beam for rendering.
struct BeamSegment final {
    WorldPos origin { 0.0f, 0.0f };
    WorldPos endpoint { 0.0f, 0.0f };
    bool lockedToTarget { false };
    bool active { false };
    std::uint32_t targetId { 0U };
};

/// @class DrainAccumulator
/// @brief Deterministic fractional energy deduction pacer.
class DrainAccumulator final {
public:
    constexpr DrainAccumulator() noexcept = default;

    /// @brief Add fractional consumption based on rate and delta time.
    /// @param[in] ratePerSecond Energy points consumed per second.
    /// @param[in] dt Timestep duration in seconds.
    constexpr void accumulate(float ratePerSecond, float dt) noexcept
    {
        if (ratePerSecond > 0.0f && dt > 0.0f) {
            accumulator_ += ratePerSecond * dt;
        }
    }

    /// @brief Extract accrued whole integer energy deductions.
    /// @return Integer points to deduct from pool.
    [[nodiscard]] constexpr std::uint16_t consumeWhole() noexcept
    {
        if (accumulator_ < 1.0f) {
            return 0U;
        }
        const auto whole { static_cast<std::uint16_t>(accumulator_) };
        accumulator_ -= static_cast<float>(whole);
        return whole;
    }

    /// @brief Reset accrued drain to zero.
    constexpr void reset() noexcept
    {
        accumulator_ = 0.0f;
    }

    /// @brief Query current fractional accumulation.
    /// @return Accrued sub-point drain.
    [[nodiscard]] constexpr float accrued() const noexcept
    {
        return accumulator_;
    }

private:
    float accumulator_ { 0.0f };
};

/// @class RotationAngle
/// @brief Strongly-typed angle in degrees for directional sprite rendering.
class RotationAngle final {
public:
    constexpr RotationAngle() noexcept = default;

    /// @brief Construct rotation angle in degrees.
    /// @param[in] deg Angle in degrees.
    explicit constexpr RotationAngle(float deg) noexcept
        : degrees_ { deg }
    {
    }

    /// @brief Retrieve raw degrees value.
    /// @return Angle in degrees.
    [[nodiscard]] constexpr float degrees() const noexcept
    {
        return degrees_;
    }

    /// @brief Equality comparison.
    [[nodiscard]] constexpr bool operator==(const RotationAngle& o) const noexcept
    {
        return degrees_ == o.degrees_;
    }

    /// @brief Inequality comparison.
    [[nodiscard]] constexpr bool operator!=(const RotationAngle& o) const noexcept
    {
        return !(*this == o);
    }

private:
    float degrees_ { 0.0f };
};

/// @brief Visual sprite identifier for presentation rendering.
enum class SpriteId : std::uint16_t {
    None = 0U,
    PlayerRobotIdle = 1U,
    PlayerRobotWalk1 = 2U,
    PlayerRobotWalk2 = 3U,
    PlayerRobotWalk3 = 4U,
    PlayerRobotJump = 5U,
    PlayerRobotCrouch = 6U,
    PlayerJet = 7U,
    PlayerJetExhaust1 = 8U,
    PlayerJetExhaust2 = 9U,
    PlayerMorph1 = 10U,
    PlayerMorph2 = 11U,
    PlayerMorph3 = 12U,
    PlayerTransforming = 13U,
    EnemyPatrol = 20U,
    EnemyBuzzer1 = 21U,
    EnemyBuzzer2 = 22U,
    EnemyCrawler1 = 23U,
    EnemyCrawler2 = 24U,
    EnemyFlyer = 25U,
    EnemyManta1 = 26U,
    EnemyManta2 = 27U,
    EnemyBoss = 28U,
    NestCoreActive = 30U,
    NestCoreDestroyed = 31U,
    SparkParticle = 40U,
    Explosion = 41U,
    ExplosionPhase1 = 42U,
    ExplosionPhase2 = 43U,
    ExplosionPhase3 = 44U,
    ExplosionPhase4 = 45U,
    TileMetalWall = 50U,
    TileCircuitWall = 51U,
    TileDestructibleWall = 52U,
    TileHazardLava = 53U,
    MissileInFlight = 60U,
    CanisterMissile = 61U,
    CanisterBomb = 62U,
    CanisterEnergy = 63U,
    CanisterShield = 64U,
    EnemyInterceptor = 65U,
    EnemyTurretCeiling = 66U,
    EnemyBulletHostile = 67U
};

/// @brief Sound effect identifier for presentation audio playback.
enum class SoundEffectId : std::uint8_t {
    None = 0U,
    LaserFire = 1U,
    LaserHit = 2U,
    ShieldDeploy = 3U,
    ShieldDeflect = 4U,
    TransformRobot = 5U,
    TransformJet = 6U,
    PlayerDamage = 7U,
    EnemyExplode = 8U,
    BlockCrumble = 9U,
    EnergyWarning = 10U,
    MissileLaunch = 11U,
    FlashBombExplode = 12U,
    CanisterPickup = 13U,
    EnemyShoot = 14U,
    WeaponSelect = 15U
};

/// @brief Music track identifier for presentation audio stream.
enum class MusicTrackId : std::uint8_t { None = 0U, MainTheme = 1U, Level1Cavern = 2U, BossBattle = 3U, GameOver = 4U };

/// @brief Entity classification category.
enum class EntityType : std::uint8_t {
    None = 0U,
    Player = 1U,
    PatrolWalker = 2U,
    HomingFlyer = 3U,
    HostileBullet = 4U,
    Particle = 5U,
    DivingInterceptor = 6U,
    WallCrawler = 7U,
    CeilingTurret = 8U,
    CargoCanister = 9U
};

/// @class EntityId
/// @brief Generational handle uniquely referencing an active entity slot.
class EntityId final {
public:
    constexpr EntityId() noexcept = default;

    /// @brief Construct an entity generational handle.
    /// @param[in] idx Array slot index.
    /// @param[in] gen Generation counter.
    explicit constexpr EntityId(std::uint16_t idx, std::uint16_t gen) noexcept
        : index_ { idx }
        , generation_ { gen }
    {
    }

    /// @brief Query internal slot index.
    /// @return Slot index.
    [[nodiscard]] constexpr std::uint16_t index() const noexcept
    {
        return index_;
    }

    /// @brief Query generation counter.
    /// @return Generation counter.
    [[nodiscard]] constexpr std::uint16_t generation() const noexcept
    {
        return generation_;
    }

    /// @brief Check whether handle references an allocated generation.
    /// @return true if generation is greater than 0.
    [[nodiscard]] constexpr bool isValid() const noexcept
    {
        return generation_ > 0U;
    }

    /// @brief Equality comparison.
    [[nodiscard]] constexpr bool operator==(const EntityId& o) const noexcept
    {
        return index_ == o.index_ && generation_ == o.generation_;
    }

    /// @brief Inequality comparison.
    [[nodiscard]] constexpr bool operator!=(const EntityId& o) const noexcept
    {
        return !(*this == o);
    }

private:
    std::uint16_t index_ { 0U };
    std::uint16_t generation_ { 0U };
};

/// @struct ColorRgba
/// @brief 32-bit RGBA color representation for vector beams and HUD elements.
struct ColorRgba final {
    std::uint8_t r { 255U };
    std::uint8_t g { 255U };
    std::uint8_t b { 255U };
    std::uint8_t a { 255U };

    [[nodiscard]] constexpr bool operator==(const ColorRgba& o) const noexcept
    {
        return r == o.r && g == o.g && b == o.b && a == o.a;
    }

    [[nodiscard]] constexpr bool operator!=(const ColorRgba& o) const noexcept
    {
        return !(*this == o);
    }
};

/// @struct CameraViewport
/// @brief World extents and position of the current view camera.
struct CameraViewport final {
    WorldPos pos { 0.0f, 0.0f };
    float width { 320.0f };
    float height { 240.0f };
};

inline constexpr std::size_t kMaxLightningNodes { 12U };
inline constexpr std::size_t kMaxParticles { 128U };

/// @struct LightningArc
/// @brief Multi-segment jittered trajectory of an electrical laser discharge.
struct LightningArc final {
    std::array<WorldPos, kMaxLightningNodes> nodes { };
    std::size_t nodeCount { 0U };
    bool active { false };
    bool lockedToTarget { false };
    std::uint32_t targetId { 0U };
};

/// @enum class ParticleType
/// @brief Category of combat particle debris.
enum class ParticleType : std::uint8_t {
    None = 0U,
    ImpactSpark = 1U,
    WallDebris = 2U,
    ShieldRipple = 3U,
    SmokeTrail = 4U,
    ShockwaveRing = 5U
};

/// @struct Particle
/// @brief Discrete visual particle entity.
struct Particle final {
    WorldPos pos { 0.0f, 0.0f };
    Velocity vel { 0.0f, 0.0f };
    ColorRgba color { 255U, 255U, 255U, 255U };
    float lifetime { 0.0f };
    float maxLifetime { 0.0f };
    float size { 1.0f };
    ParticleType type { ParticleType::None };
    bool active { false };
};

/// @struct ShieldFxState
/// @brief State describing force barrier breathing and impact transients.
struct ShieldFxState final {
    float breathRadius { 0.0f };
    float hitAngleRad { 0.0f };
    float hitIntensity { 0.0f };
    bool isLowEnergy { false };
};

/// @enum class SecondaryWeaponType
/// @brief Classification of auxiliary ordnance bays.
enum class SecondaryWeaponType : std::uint8_t { None = 0U, HomingMissile = 1U, FlashBomb = 2U };

/// @class SecondaryAmmo
/// @brief Strongly-typed ammunition magazine counter with underflow and capacity guards.
class SecondaryAmmo final {
public:
    constexpr SecondaryAmmo() noexcept = default;

    explicit constexpr SecondaryAmmo(std::uint8_t count, std::uint8_t maxCap) noexcept
        : count_ { std::min(count, maxCap) }
        , maxCap_ { maxCap }
    {
    }

    [[nodiscard]] constexpr std::uint8_t count() const noexcept
    {
        return count_;
    }

    [[nodiscard]] constexpr std::uint8_t maxCapacity() const noexcept
    {
        return maxCap_;
    }

    [[nodiscard]] constexpr bool hasAmmo() const noexcept
    {
        return count_ > 0U;
    }

    [[nodiscard]] constexpr bool isFull() const noexcept
    {
        return count_ >= maxCap_;
    }

    constexpr void consume() noexcept
    {
        if (count_ > 0U) {
            --count_;
        }
    }

    constexpr void replenish(std::uint8_t amount) noexcept
    {
        const std::uint16_t total { static_cast<std::uint16_t>(
            static_cast<std::uint16_t>(count_) + static_cast<std::uint16_t>(amount)) };
        count_ = static_cast<std::uint8_t>(std::min(total, static_cast<std::uint16_t>(maxCap_)));
    }

    [[nodiscard]] constexpr bool operator==(const SecondaryAmmo& o) const noexcept
    {
        return count_ == o.count_ && maxCap_ == o.maxCap_;
    }

    [[nodiscard]] constexpr bool operator!=(const SecondaryAmmo& o) const noexcept
    {
        return !(*this == o);
    }

private:
    std::uint8_t count_ { 0U };
    std::uint8_t maxCap_ { 0U };
};

/// @enum class CanisterType
/// @brief Classification of tactical supply cargo canisters.
enum class CanisterType : std::uint8_t { MissilePack = 0U, FlashBomb = 1U, EnergyBoost = 2U, ShieldRecharge = 3U };

/// @enum class EnemyAiType
/// @brief Classification of enemy AI behavioral controllers.
enum class EnemyAiType : std::uint8_t {
    PatrolWalker = 0U,
    DivingInterceptor = 1U,
    WallCrawler = 2U,
    CeilingTurret = 3U
};

/// @enum class AiBehaviorState
/// @brief State tags for hostile entity state machines.
enum class AiBehaviorState : std::uint8_t {
    Idle = 0U,
    Stalk = 1U,
    AcquireLock = 2U,
    HighSpeedSwoop = 3U,
    BreakAway = 4U,
    Crawling = 5U,
    Aiming = 6U
};

inline constexpr std::size_t kMaxActiveMissiles { 8U };
inline constexpr std::size_t kMaxCanisters { 16U };
inline constexpr std::size_t kMaxNests { 16U };

/// @struct HomingMissile
/// @brief State data for an airborne self-guided micro-missile.
struct HomingMissile final {
    WorldPos pos { 0.0f, 0.0f };
    Velocity vel { 0.0f, 0.0f };
    EntityId targetId { };
    float fuelSec { 2.5f };
    float smokeTimer { 0.0f };
    float angleDeg { 0.0f };
    bool active { false };
};

/// @struct FlashBombState
/// @brief State data for an active screen-clearing thermonuclear shockwave.
struct FlashBombState final {
    WorldPos center { 0.0f, 0.0f };
    float currentRadius { 0.0f };
    float maxRadius { 320.0f };
    float timer { 0.0f };
    float duration { 0.35f };
    bool active { false };
};

/// @struct CargoCanister
/// @brief State data for a dropped supply cargo container.
struct CargoCanister final {
    WorldPos pos { 0.0f, 0.0f };
    Velocity vel { 0.0f, 0.0f };
    CanisterType type { CanisterType::MissilePack };
    float bobTimer { 0.0f };
    float lifeTimer { 15.0f };
    bool onGround { false };
    bool magnetized { false };
    bool active { false };
};

/// @struct BioNest
/// @brief Stationary organic subterranean generator hive.
struct BioNest final {
    WorldPos pos { 0.0f, 0.0f };
    float spawnTimer { 0.0f };
    float spawnInterval { 4.0f };
    std::uint16_t health { 80U };
    bool active { false };
    bool destroyed { false };
};

} // namespace ShooterCore
