#include "SecondaryArsenal.h"

#include <cmath>
#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <array>

#include "AudioCommandQueue.h"
#include "EntityPool.h"
#include "Hitbox.h"
#include "ParticlePool.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore {

namespace {
    constexpr float kDegToRad { 0.0174532925f };
    constexpr float kRadToDeg { 57.2957795f };

    [[nodiscard]] float normalizeAngleDeg(float deg) noexcept
    {
        while (deg > 180.0f) {
            deg -= 360.0f;
        }
        while (deg < -180.0f) {
            deg += 360.0f;
        }
        return deg;
    }
} // namespace

void SecondaryArsenal::cycleWeapon() noexcept
{
    switch (activeWeapon_) {
    case SecondaryWeaponType::None:
        activeWeapon_ = SecondaryWeaponType::StandardMissile;
        break;
    case SecondaryWeaponType::StandardMissile:
        activeWeapon_ = SecondaryWeaponType::HomingMissile;
        break;
    case SecondaryWeaponType::HomingMissile:
        activeWeapon_ = SecondaryWeaponType::NapalmBomb;
        break;
    case SecondaryWeaponType::NapalmBomb:
        activeWeapon_ = SecondaryWeaponType::FlashBomb;
        break;
    case SecondaryWeaponType::FlashBomb:
        activeWeapon_ = SecondaryWeaponType::DeflectorShield;
        break;
    case SecondaryWeaponType::DeflectorShield:
    default:
        activeWeapon_ = SecondaryWeaponType::None;
        break;
    }
}

void SecondaryArsenal::setWeapon(SecondaryWeaponType type) noexcept
{
    activeWeapon_ = type;
}

void SecondaryArsenal::replenishStandardMissiles(std::uint8_t count) noexcept
{
    stdMissileAmmo_.replenish(count);
}

void SecondaryArsenal::replenishHomingMissiles(std::uint8_t count) noexcept
{
    homingMissileAmmo_.replenish(count);
}

void SecondaryArsenal::replenishMissiles(std::uint8_t count) noexcept
{
    replenishHomingMissiles(count);
}

void SecondaryArsenal::replenishNapalm(std::uint8_t count) noexcept
{
    napalmAmmo_.replenish(count);
}

void SecondaryArsenal::replenishBombs(std::uint8_t count) noexcept
{
    bombAmmo_.replenish(count);
}

void SecondaryArsenal::replenishDeflector(std::uint8_t count) noexcept
{
    deflectorAmmo_.replenish(count);
}

bool SecondaryArsenal::triggerFire(WorldPos craftPos, FacingDirection facing, EntityPool& entities,
    AudioCommandQueue& audio, ParticlePool& particles) noexcept
{
    if (fireCooldown_ > 0.0f) {
        return false;
    }

    if (activeWeapon_ == SecondaryWeaponType::StandardMissile) {
        if (!stdMissileAmmo_.hasAmmo()) {
            return false;
        }

        std::size_t freeIdx { kMaxStandardMissiles };
        for (std::size_t i { 0U }; i < kMaxStandardMissiles; ++i) {
            if (!standardMissiles_[i].active) {
                freeIdx = i;
                break;
            }
        }
        if (freeIdx >= kMaxStandardMissiles) {
            return false;
        }

        stdMissileAmmo_.consume();
        StandardMissile& m { standardMissiles_[freeIdx] };
        m.pos = WorldPos { craftPos.x, craftPos.y - 8.0f };
        const float dirSign { facing == FacingDirection::Left ? -1.0f : 1.0f };
        m.vel = Velocity { dirSign * kStandardMissileSpeed, 0.0f };
        m.smokeTimer = 0.0f;
        m.lifeTimer = kStandardMissileLife;
        m.active = true;

        ++activeStandardMissileCount_;
        fireCooldown_ = 0.25f;
        audio.pushSound(SoundEffectId::MissileLaunch);
        particles.emitSmoke(m.pos);
        return true;
    }

    if (activeWeapon_ == SecondaryWeaponType::HomingMissile) {
        if (!homingMissileAmmo_.hasAmmo()) {
            return false;
        }

        std::size_t freeIdx { kMaxMissiles };
        for (std::size_t i { 0U }; i < kMaxMissiles; ++i) {
            if (!missiles_[i].active) {
                freeIdx = i;
                break;
            }
        }
        if (freeIdx >= kMaxMissiles) {
            return false;
        }

        homingMissileAmmo_.consume();
        HomingMissile& m { missiles_[freeIdx] };
        m.pos = WorldPos { craftPos.x, craftPos.y - 12.0f };
        const float dirSign { facing == FacingDirection::Left ? -1.0f : 1.0f };
        m.vel = Velocity { dirSign * kInitialMissileSpeed, -30.0f };
        m.angleDeg = facing == FacingDirection::Left ? 180.0f : 0.0f;
        m.fuelSec = kMissileFuelTime;
        m.smokeTimer = 0.0f;
        m.targetId = findMissileTarget(m.pos, entities);
        m.active = true;

        ++activeMissileCount_;
        fireCooldown_ = kMissileFireCooldown;
        audio.pushSound(SoundEffectId::MissileLaunch);
        particles.emitSmoke(m.pos);
        return true;
    }

    if (activeWeapon_ == SecondaryWeaponType::NapalmBomb) {
        if (!napalmAmmo_.hasAmmo()) {
            return false;
        }

        std::size_t freeIdx { kMaxNapalmCanisters };
        for (std::size_t i { 0U }; i < kMaxNapalmCanisters; ++i) {
            if (!napalmCanisters_[i].active) {
                freeIdx = i;
                break;
            }
        }
        if (freeIdx >= kMaxNapalmCanisters) {
            return false;
        }

        napalmAmmo_.consume();
        NapalmCanister& c { napalmCanisters_[freeIdx] };
        c.pos = WorldPos { craftPos.x, craftPos.y + 8.0f };
        const float dirSign { facing == FacingDirection::Left ? -1.0f : 1.0f };
        c.vel = Velocity { dirSign * 40.0f, kNapalmDropSpeed };
        c.active = true;

        ++activeNapalmCanisterCount_;
        fireCooldown_ = 0.40f;
        audio.pushSound(SoundEffectId::NapalmDrop);
        return true;
    }

    if (activeWeapon_ == SecondaryWeaponType::FlashBomb) {
        if (!bombAmmo_.hasAmmo() || flashBomb_.active) {
            return false;
        }

        bombAmmo_.consume();
        flashBomb_.active = true;
        flashBomb_.center = craftPos;
        flashBomb_.currentRadius = 16.0f;
        flashBomb_.maxRadius = 320.0f;
        flashBomb_.timer = 0.0f;
        flashBomb_.duration = kFlashBombDuration;

        fireCooldown_ = 0.5f;
        audio.pushSound(SoundEffectId::FlashBombExplode);
        particles.emitShockwave(craftPos, 24U);
        return true;
    }

    if (activeWeapon_ == SecondaryWeaponType::DeflectorShield) {
        if (!deflectorAmmo_.hasAmmo() || deflector_.active) {
            return false;
        }

        deflectorAmmo_.consume();
        deflector_.active = true;
        deflector_.center = craftPos;
        deflector_.remainingSec = kDeflectorDuration;
        deflector_.hitpoints = kDeflectorMaxHp;

        fireCooldown_ = 0.5f;
        audio.pushSound(SoundEffectId::DeflectorActive);
        particles.emitSparks(craftPos, 0.0f, 16U);
        return true;
    }

    return false;
}

EntityId SecondaryArsenal::findMissileTarget(WorldPos missilePos, const EntityPool& entities) const noexcept
{
    EntityId bestId { };
    float bestDistSq { 320.0f * 320.0f };

    for (std::size_t i { 0U }; i < entities.capacity(); ++i) {
        const EntitySlot& slot { entities.slot(i) };
        if (!slot.active) {
            continue;
        }
        const Entity& e { slot.data };
        if (e.type == EntityType::Player || e.type == EntityType::Particle || e.type == EntityType::CargoCanister) {
            continue;
        }
        if (e.health == 0U) {
            continue;
        }

        const float dx { e.pos.x - missilePos.x };
        const float dy { e.pos.y - missilePos.y };
        const float distSq { (dx * dx) + (dy * dy) };

        if (distSq < bestDistSq) {
            bestDistSq = distSq;
            bestId = EntityId { static_cast<std::uint16_t>(i), slot.generation };
        }
    }

    return bestId;
}

void SecondaryArsenal::explodeStandardMissile(
    std::size_t idx, EntityPool& entities, TileMap& map, ParticlePool& particles, AudioCommandQueue& audio) noexcept
{
    StandardMissile& m { standardMissiles_[idx] };
    m.active = false;
    if (activeStandardMissileCount_ > 0U) {
        --activeStandardMissileCount_;
    }

    particles.emitSparks(m.pos, 0.0f, 24U);
    audio.pushSound(SoundEffectId::EnemyExplode);

    // Apply 50 direct kinetic damage / 25 splash damage to entities within 32 px
    for (std::size_t i { 0U }; i < entities.capacity(); ++i) {
        const EntitySlot& slot { entities.slot(i) };
        if (!slot.active) {
            continue;
        }
        Entity& e { *entities.get(EntityId { static_cast<std::uint16_t>(i), slot.generation }) };
        if (e.type == EntityType::Player || e.type == EntityType::Particle || e.type == EntityType::CargoCanister) {
            continue;
        }
        if (e.health == 0U) {
            continue;
        }

        const float dx { e.pos.x - m.pos.x };
        const float dy { e.pos.y - m.pos.y };
        const float distSq { (dx * dx) + (dy * dy) };
        if (distSq <= (32.0f * 32.0f)) {
            const std::uint16_t dmg { static_cast<std::uint16_t>(
                distSq <= (16.0f * 16.0f) ? kStandardMissileDamage : 25U) };
            if (e.health <= dmg) {
                e.health = 0U;
            } else {
                e.health = static_cast<std::uint16_t>(e.health - dmg);
            }
        }
    }

    // Shatter 3x3 destructible wall cluster
    const TileCoord centerTile { m.pos.toTileCoord(16.0f) };
    for (std::int32_t r { -1 }; r <= 1; ++r) {
        for (std::int32_t c { -1 }; c <= 1; ++c) {
            const auto checkRow { static_cast<std::int32_t>(centerTile.row) + r };
            const auto checkCol { static_cast<std::int32_t>(centerTile.col) + c };
            if (checkRow >= 0 && checkCol >= 0) {
                const TileCoord tc { static_cast<std::uint32_t>(checkCol), static_cast<std::uint32_t>(checkRow) };
                if (map.getTile(tc) == TileType::DestructibleWall) {
                    map.setTile(tc, TileType::Empty);
                    particles.emitDebris(WorldPos { static_cast<float>(tc.col) * 16.0f + 8.0f,
                                             static_cast<float>(tc.row) * 16.0f + 8.0f },
                        4U);
                }
            }
        }
    }
}

void SecondaryArsenal::explodeHomingMissile(
    std::size_t idx, EntityPool& entities, TileMap& map, ParticlePool& particles, AudioCommandQueue& audio) noexcept
{
    HomingMissile& m { missiles_[idx] };
    m.active = false;
    if (activeMissileCount_ > 0U) {
        --activeMissileCount_;
    }

    particles.emitSparks(m.pos, 0.0f, 16U);
    audio.pushSound(SoundEffectId::EnemyExplode);

    for (std::size_t i { 0U }; i < entities.capacity(); ++i) {
        const EntitySlot& slot { entities.slot(i) };
        if (!slot.active) {
            continue;
        }
        Entity& e { *entities.get(EntityId { static_cast<std::uint16_t>(i), slot.generation }) };
        if (e.type == EntityType::Player || e.type == EntityType::Particle || e.type == EntityType::CargoCanister) {
            continue;
        }
        if (e.health == 0U) {
            continue;
        }

        const float dx { e.pos.x - m.pos.x };
        const float dy { e.pos.y - m.pos.y };
        const float distSq { (dx * dx) + (dy * dy) };
        if (distSq <= (kMissileSplashRadius * kMissileSplashRadius)) {
            if (e.health <= kMissileSplashDamage) {
                e.health = 0U;
            } else {
                e.health = static_cast<std::uint16_t>(e.health - kMissileSplashDamage);
            }
        }
    }

    const TileCoord centerTile { m.pos.toTileCoord(16.0f) };
    for (std::int32_t r { -1 }; r <= 1; ++r) {
        for (std::int32_t c { -1 }; c <= 1; ++c) {
            const auto checkRow { static_cast<std::int32_t>(centerTile.row) + r };
            const auto checkCol { static_cast<std::int32_t>(centerTile.col) + c };
            if (checkRow >= 0 && checkCol >= 0) {
                const TileCoord tc { static_cast<std::uint32_t>(checkCol), static_cast<std::uint32_t>(checkRow) };
                if (map.getTile(tc) == TileType::DestructibleWall) {
                    map.damageTile(tc);
                }
            }
        }
    }
}

void SecondaryArsenal::igniteNapalm(
    WorldPos impactPos, TileMap& map, ParticlePool& particles, AudioCommandQueue& audio) noexcept
{
    std::size_t freeIdx { kMaxNapalmPools };
    for (std::size_t i { 0U }; i < kMaxNapalmPools; ++i) {
        if (!napalmPools_[i].active) {
            freeIdx = i;
            break;
        }
    }

    if (freeIdx >= kMaxNapalmPools) {
        freeIdx = 0U; // Recycle oldest pool
    } else {
        ++activeNapalmPoolCount_;
    }

    NapalmPool& pool { napalmPools_[freeIdx] };
    pool.leftEdge = WorldPos { impactPos.x - 4.0f, impactPos.y };
    pool.rightEdge = WorldPos { impactPos.x + 4.0f, impactPos.y };
    pool.floorY = impactPos.y;
    pool.remainingSec = kNapalmDuration;
    pool.tickTimer = 0.0f;
    pool.active = true;

    audio.pushSound(SoundEffectId::NapalmBurn);
    particles.emitSmoke(impactPos);

    // Damage destructible walls immediately adjacent to floor
    const TileCoord tile { impactPos.toTileCoord(16.0f) };
    if (map.getTile(tile) == TileType::DestructibleWall) {
        map.damageTile(tile);
    }
}

void SecondaryArsenal::update(
    DeltaTime dt, EntityPool& entities, TileMap& map, ParticlePool& particles, AudioCommandQueue& audio) noexcept
{
    const float dtSec { dt.seconds() };
    if (dtSec <= 0.0f) {
        return;
    }

    if (fireCooldown_ > 0.0f) {
        fireCooldown_ = std::max(0.0f, fireCooldown_ - dtSec);
    }

    // 1. Advance Deflector Shield
    if (deflector_.active) {
        deflector_.remainingSec -= dtSec;
        if (deflector_.remainingSec <= 0.0f || deflector_.hitpoints == 0U) {
            deflector_.active = false;
        } else {
            // Deflect hostile projectiles intersecting shield radius
            const float rSq { kDeflectorRadius * kDeflectorRadius };
            for (std::size_t i { 0U }; i < entities.capacity(); ++i) {
                const EntitySlot& slot { entities.slot(i) };
                if (!slot.active) {
                    continue;
                }
                Entity& e { *entities.get(EntityId { static_cast<std::uint16_t>(i), slot.generation }) };
                if (e.type != EntityType::HostileBullet) {
                    continue;
                }

                const float dx { e.pos.x - deflector_.center.x };
                const float dy { e.pos.y - deflector_.center.y };
                if (((dx * dx) + (dy * dy)) <= rSq) {
                    // Invert velocity and reflect
                    e.vel.dx = -e.vel.dx;
                    e.vel.dy = -e.vel.dy;
                    e.type = EntityType::Particle; // Neutralize as hostile
                    audio.pushSound(SoundEffectId::DeflectorReflect);
                    particles.emitSparks(e.pos, 0.0f, 6U);
                    if (deflector_.hitpoints <= 10U) {
                        deflector_.hitpoints = 0U;
                        deflector_.active = false;
                    } else {
                        deflector_.hitpoints = static_cast<std::uint16_t>(deflector_.hitpoints - 10U);
                    }
                }
            }
        }
    }

    // 2. Advance Standard Missiles
    for (std::size_t i { 0U }; i < kMaxStandardMissiles; ++i) {
        StandardMissile& m { standardMissiles_[i] };
        if (!m.active) {
            continue;
        }

        m.lifeTimer -= dtSec;
        if (m.lifeTimer <= 0.0f) {
            explodeStandardMissile(i, entities, map, particles, audio);
            continue;
        }

        m.smokeTimer += dtSec;
        if (m.smokeTimer >= 0.04f) {
            m.smokeTimer = 0.0f;
            particles.emitSmoke(m.pos);
        }

        m.pos.x += m.vel.dx * dtSec;
        m.pos.y += m.vel.dy * dtSec;

        const Hitbox missileBox { WorldPos { m.pos.x - 4.0f, m.pos.y - 4.0f },
            WorldPos { m.pos.x + 4.0f, m.pos.y + 4.0f } };

        bool collided { false };
        for (std::size_t eIdx { 0U }; eIdx < entities.capacity(); ++eIdx) {
            const EntitySlot& slot { entities.slot(eIdx) };
            if (!slot.active) {
                continue;
            }
            Entity& e { *entities.get(EntityId { static_cast<std::uint16_t>(eIdx), slot.generation }) };
            if (e.type == EntityType::Player || e.type == EntityType::Particle || e.type == EntityType::CargoCanister) {
                continue;
            }
            if (e.health == 0U) {
                continue;
            }

            if (missileBox.intersects(e.bounds)) {
                if (e.health <= kStandardMissileDamage) {
                    e.health = 0U;
                } else {
                    e.health = static_cast<std::uint16_t>(e.health - kStandardMissileDamage);
                }
                collided = true;
                break;
            }
        }

        if (collided) {
            explodeStandardMissile(i, entities, map, particles, audio);
            continue;
        }

        const TileCoord tile { m.pos.toTileCoord(16.0f) };
        if (map.isSolid(tile)) {
            explodeStandardMissile(i, entities, map, particles, audio);
            continue;
        }
    }

    // 3. Advance Homing Missiles
    for (std::size_t i { 0U }; i < kMaxMissiles; ++i) {
        HomingMissile& m { missiles_[i] };
        if (!m.active) {
            continue;
        }

        m.fuelSec -= dtSec;
        if (m.fuelSec <= 0.0f) {
            explodeHomingMissile(i, entities, map, particles, audio);
            continue;
        }

        m.smokeTimer += dtSec;
        if (m.smokeTimer >= 0.05f) {
            m.smokeTimer = 0.0f;
            particles.emitSmoke(m.pos);
        }

        if (!entities.isValid(m.targetId)) {
            m.targetId = findMissileTarget(m.pos, entities);
        } else {
            const Entity* target { entities.get(m.targetId) };
            if (target == nullptr || target->health == 0U) {
                m.targetId = findMissileTarget(m.pos, entities);
            }
        }

        if (entities.isValid(m.targetId)) {
            const Entity* target { entities.get(m.targetId) };
            if (target != nullptr) {
                const float dx { target->pos.x - m.pos.x };
                const float dy { target->pos.y - m.pos.y };
                const float desiredAngleDeg { std::atan2(dy, dx) * kRadToDeg };
                const float angleDiff { normalizeAngleDeg(desiredAngleDeg - m.angleDeg) };
                const float maxTurn { kMissileTurnRateDeg * dtSec };
                m.angleDeg += std::clamp(angleDiff, -maxTurn, maxTurn);
                m.angleDeg = normalizeAngleDeg(m.angleDeg);
            }
        }

        const float angleRad { m.angleDeg * kDegToRad };
        const float currentSpeed { std::sqrt((m.vel.dx * m.vel.dx) + (m.vel.dy * m.vel.dy)) };
        const float newSpeed { std::min(kMissileTopSpeed, currentSpeed + (kMissileThrust * dtSec)) };
        m.vel.dx = std::cos(angleRad) * newSpeed;
        m.vel.dy = std::sin(angleRad) * newSpeed;

        m.pos.x += m.vel.dx * dtSec;
        m.pos.y += m.vel.dy * dtSec;

        const Hitbox missileBox { WorldPos { m.pos.x - 3.0f, m.pos.y - 3.0f },
            WorldPos { m.pos.x + 3.0f, m.pos.y + 3.0f } };

        bool collided { false };
        for (std::size_t eIdx { 0U }; eIdx < entities.capacity(); ++eIdx) {
            const EntitySlot& slot { entities.slot(eIdx) };
            if (!slot.active) {
                continue;
            }
            Entity& e { *entities.get(EntityId { static_cast<std::uint16_t>(eIdx), slot.generation }) };
            if (e.type == EntityType::Player || e.type == EntityType::Particle || e.type == EntityType::CargoCanister) {
                continue;
            }
            if (e.health == 0U) {
                continue;
            }

            if (missileBox.intersects(e.bounds)) {
                if (e.health <= kMissileDirectDamage) {
                    e.health = 0U;
                } else {
                    e.health = static_cast<std::uint16_t>(e.health - kMissileDirectDamage);
                }
                collided = true;
                break;
            }
        }

        if (collided) {
            explodeHomingMissile(i, entities, map, particles, audio);
            continue;
        }

        const TileCoord tile { m.pos.toTileCoord(16.0f) };
        if (map.isSolid(tile)) {
            explodeHomingMissile(i, entities, map, particles, audio);
            continue;
        }
    }

    // 4. Advance Falling Napalm Canisters
    for (std::size_t i { 0U }; i < kMaxNapalmCanisters; ++i) {
        NapalmCanister& c { napalmCanisters_[i] };
        if (!c.active) {
            continue;
        }

        c.vel.dy += kNapalmGravity * dtSec;
        c.pos.x += c.vel.dx * dtSec;
        c.pos.y += c.vel.dy * dtSec;

        const TileCoord tile { c.pos.toTileCoord(16.0f) };
        if (map.isSolid(tile) || c.pos.y >= (static_cast<float>(map.height()) * 16.0f - 16.0f)) {
            c.active = false;
            if (activeNapalmCanisterCount_ > 0U) {
                --activeNapalmCanisterCount_;
            }
            igniteNapalm(c.pos, map, particles, audio);
        }
    }

    // 5. Advance Active Napalm Burning Pools
    for (std::size_t i { 0U }; i < kMaxNapalmPools; ++i) {
        NapalmPool& pool { napalmPools_[i] };
        if (!pool.active) {
            continue;
        }

        pool.remainingSec -= dtSec;
        if (pool.remainingSec <= 0.0f) {
            pool.active = false;
            if (activeNapalmPoolCount_ > 0U) {
                --activeNapalmPoolCount_;
            }
            continue;
        }

        // Expand pool horizontally
        const float spreadDelta { kNapalmSpreadSpeed * dtSec };
        pool.leftEdge.x -= spreadDelta;
        pool.rightEdge.x += spreadDelta;

        pool.tickTimer += dtSec;
        if (pool.tickTimer >= 0.1f) {
            pool.tickTimer = 0.0f;
            particles.emitSmoke(WorldPos { (pool.leftEdge.x + pool.rightEdge.x) * 0.5f, pool.floorY - 4.0f });

            // Inflict continuous burn damage (60 DPS -> 6 dmg per 0.1s tick)
            const Hitbox fireBox { WorldPos { pool.leftEdge.x, pool.floorY - 14.0f },
                WorldPos { pool.rightEdge.x, pool.floorY + 2.0f } };

            for (std::size_t eIdx { 0U }; eIdx < entities.capacity(); ++eIdx) {
                const EntitySlot& slot { entities.slot(eIdx) };
                if (!slot.active) {
                    continue;
                }
                Entity& e { *entities.get(EntityId { static_cast<std::uint16_t>(eIdx), slot.generation }) };
                if (e.type == EntityType::Player || e.type == EntityType::Particle
                    || e.type == EntityType::CargoCanister) {
                    continue;
                }
                if (e.health == 0U) {
                    continue;
                }

                if (fireBox.intersects(e.bounds)) {
                    constexpr std::uint16_t kTickDmg { 6U };
                    if (e.health <= kTickDmg) {
                        e.health = 0U;
                    } else {
                        e.health = static_cast<std::uint16_t>(e.health - kTickDmg);
                    }
                }
            }
        }
    }

    // 6. Advance Screen-Clearing Flash Bomb
    if (flashBomb_.active) {
        flashBomb_.timer += dtSec;
        flashBomb_.currentRadius += kFlashBombSpeed * dtSec;

        const float rSq { flashBomb_.currentRadius * flashBomb_.currentRadius };
        for (std::size_t i { 0U }; i < entities.capacity(); ++i) {
            const EntitySlot& slot { entities.slot(i) };
            if (!slot.active) {
                continue;
            }
            Entity& e { *entities.get(EntityId { static_cast<std::uint16_t>(i), slot.generation }) };
            if (e.type == EntityType::Player || e.type == EntityType::Particle || e.type == EntityType::CargoCanister) {
                continue;
            }
            if (e.health == 0U) {
                continue;
            }

            const float dx { e.pos.x - flashBomb_.center.x };
            const float dy { e.pos.y - flashBomb_.center.y };
            const float distSq { (dx * dx) + (dy * dy) };

            if (distSq <= rSq) {
                if (e.health <= kFlashBombDamage) {
                    e.health = 0U;
                } else {
                    e.health = static_cast<std::uint16_t>(e.health - kFlashBombDamage);
                }
            }
        }

        const std::int32_t tileRadius { static_cast<std::int32_t>(flashBomb_.currentRadius / 16.0f) };
        const TileCoord centerTile { flashBomb_.center.toTileCoord(16.0f) };

        for (std::int32_t r { -tileRadius }; r <= tileRadius; ++r) {
            for (std::int32_t c { -tileRadius }; c <= tileRadius; ++c) {
                const auto checkRow { static_cast<std::int32_t>(centerTile.row) + r };
                const auto checkCol { static_cast<std::int32_t>(centerTile.col) + c };
                if (checkRow >= 0 && checkCol >= 0) {
                    const TileCoord tc { static_cast<std::uint32_t>(checkCol), static_cast<std::uint32_t>(checkRow) };
                    if (map.getTile(tc) == TileType::DestructibleWall) {
                        const float dx { (static_cast<float>(tc.col) * 16.0f + 8.0f) - flashBomb_.center.x };
                        const float dy { (static_cast<float>(tc.row) * 16.0f + 8.0f) - flashBomb_.center.y };
                        if (((dx * dx) + (dy * dy)) <= rSq) {
                            map.setTile(tc, TileType::Empty);
                            particles.emitDebris(WorldPos { static_cast<float>(tc.col) * 16.0f + 8.0f,
                                                     static_cast<float>(tc.row) * 16.0f + 8.0f },
                                4U);
                        }
                    }
                }
            }
        }

        if (flashBomb_.timer >= flashBomb_.duration || flashBomb_.currentRadius >= flashBomb_.maxRadius) {
            flashBomb_.active = false;
        }
    }
}

void SecondaryArsenal::clear() noexcept
{
    for (std::size_t i { 0U }; i < kMaxStandardMissiles; ++i) {
        standardMissiles_[i] = StandardMissile { };
    }
    activeStandardMissileCount_ = 0U;

    for (std::size_t i { 0U }; i < kMaxMissiles; ++i) {
        missiles_[i] = HomingMissile { };
    }
    activeMissileCount_ = 0U;

    for (std::size_t i { 0U }; i < kMaxNapalmCanisters; ++i) {
        napalmCanisters_[i] = NapalmCanister { };
    }
    activeNapalmCanisterCount_ = 0U;

    for (std::size_t i { 0U }; i < kMaxNapalmPools; ++i) {
        napalmPools_[i] = NapalmPool { };
    }
    activeNapalmPoolCount_ = 0U;

    flashBomb_ = FlashBombState { };
    deflector_ = DeflectorState { };
    fireCooldown_ = 0.0f;
}

} // namespace ShooterCore
