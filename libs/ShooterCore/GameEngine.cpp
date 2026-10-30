#include "GameEngine.h"

#include <cmath>
#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <array>

#include "CollisionSolver.h"
#include "Hitbox.h"
#include "TileMap.h"
#include "TransformSystem.h"
#include "Types.h"
#include "WeaponSystem.h"

namespace ShooterCore {

GameEngine::GameEngine() noexcept
{
    camera_.pos = WorldPos { 0.0f, 0.0f };
    camera_.width = 320.0f;
    camera_.height = 240.0f;
}

Hitbox GameEngine::computeHitbox(WorldPos pos) const noexcept
{
    float w { 0.0f };
    float h { 0.0f };
    TransformSystem::getDims(transformSys_.mode(), w, h);
    return Hitbox { WorldPos { pos.x - (w * 0.5f), pos.y - h }, WorldPos { pos.x + (w * 0.5f), pos.y } };
}

void GameEngine::initLevel(const TileMap& map, WorldPos playerSpawn) noexcept
{
    map_ = map;
    entities_.clear();
    renderQueue_.clear();
    audioQueue_.clear();

    playerPos_ = playerSpawn;
    playerVel_ = Velocity { 0.0f, 0.0f };
    playerFacing_ = FacingDirection::Right;
    energy_ = EnergyPoints { EnergyPoints::kDefaultMax };
    onGround_ = false;
    isAlive_ = true;

    playerBox_ = computeHitbox(playerPos_);

    updateCamera();
}

EntityId GameEngine::spawnEnemy(EntityType type, WorldPos pos, Hitbox bounds, std::uint16_t hp) noexcept
{
    return entities_.spawn(type, pos, bounds, hp);
}

void GameEngine::step(DeltaTime dt, EngineInput input) noexcept
{
    renderQueue_.clear();
    audioQueue_.clear();

    if (!isAlive_) {
        return;
    }

    updateInput(dt, input);
    updatePhysics(dt, input);
    updateWeapons(dt, input);
    updateEntities(dt);
    updateCamera();
    emitCommands();
}

void GameEngine::updateInput(DeltaTime dt, EngineInput input) noexcept
{
    if (input.isTransform()) {
        const TransformResult res { transformSys_.requestTransform(playerPos_, map_) };
        if (res == TransformResult::Success) {
            if (transformSys_.mode() == TransformMode::TransformingToJet) {
                audioQueue_.pushSound(SoundEffectId::TransformJet);
            } else {
                audioQueue_.pushSound(SoundEffectId::TransformRobot);
            }
        }
    }

    transformSys_.update(dt);
    playerBox_ = computeHitbox(playerPos_);
}

void GameEngine::updatePhysics(DeltaTime dt, EngineInput input) noexcept
{
    const float dtSec { dt.seconds() };
    if (dtSec <= 0.0f) {
        return;
    }

    const TransformMode mode { transformSys_.mode() };

    if (mode == TransformMode::Robot || mode == TransformMode::TransformingToRobot) {
        // Robot kinematics: Walking & jumping
        if (input.isLeft()) {
            playerVel_.dx = -kWalkSpeed;
            playerFacing_ = FacingDirection::Left;
        } else if (input.isRight()) {
            playerVel_.dx = kWalkSpeed;
            playerFacing_ = FacingDirection::Right;
        } else if (onGround_) {
            playerVel_.dx *= std::max(0.0f, 1.0f - (kGroundFriction * dtSec));
        }

        if (onGround_ && input.isJump()) {
            playerVel_.dy = kJumpSpeed;
            onGround_ = false;
        }

        // Gravity
        playerVel_.dy = std::min(playerVel_.dy + (kGravity * dtSec), 400.0f);
    } else {
        // Jet kinematics: Omnidirectional thrust, drag & no gravity
        if (input.isLeft()) {
            playerVel_.dx -= kJetThrust * dtSec;
            playerFacing_ = FacingDirection::Left;
        } else if (input.isRight()) {
            playerVel_.dx += kJetThrust * dtSec;
            playerFacing_ = FacingDirection::Right;
        } else {
            playerVel_.dx *= std::max(0.0f, 1.0f - (kJetDrag * dtSec));
        }

        if (input.isUp()) {
            playerVel_.dy -= kJetThrust * dtSec;
        } else if (input.isDown()) {
            playerVel_.dy += kJetThrust * dtSec;
        } else {
            playerVel_.dy *= std::max(0.0f, 1.0f - (kJetDrag * dtSec));
        }

        // Speed clamping
        playerVel_.dx = std::clamp(playerVel_.dx, -kJetMaxSpeed, kJetMaxSpeed);
        playerVel_.dy = std::clamp(playerVel_.dy, -kJetMaxSpeed, kJetMaxSpeed);
    }

    // Horizontal Swept AABB movement
    const Velocity vx { playerVel_.dx, 0.0f };
    const SweepHit hitX { CollisionSolver::sweepMap(playerBox_, vx, dtSec, map_) };
    if (hitX.collided) {
        playerPos_.x += vx.dx * dtSec * hitX.toi;
        playerVel_.dx = 0.0f;
    } else {
        playerPos_.x += vx.dx * dtSec;
    }
    playerBox_ = computeHitbox(playerPos_);

    // Vertical Swept AABB movement
    const Velocity vy { 0.0f, playerVel_.dy };
    const SweepHit hitY { CollisionSolver::sweepMap(playerBox_, vy, dtSec, map_) };
    if (hitY.collided) {
        playerPos_.y += vy.dy * dtSec * hitY.toi;
        if (hitY.normal == ContactNormal::Up) {
            onGround_ = true;
        }
        playerVel_.dy = 0.0f;
    } else {
        playerPos_.y += vy.dy * dtSec;
        if (mode == TransformMode::Robot) {
            onGround_ = false;
        }
    }
    playerBox_ = computeHitbox(playerPos_);
}

void GameEngine::updateWeapons(DeltaTime dt, EngineInput input) noexcept
{
    // 1. Harvest target candidates from EntityPool
    std::array<TargetCandidate, WeaponSystem::kMaxTargets> candidates { };
    std::size_t candCount { 0U };

    for (std::size_t i { 0U }; i < entities_.capacity() && candCount < WeaponSystem::kMaxTargets; ++i) {
        const EntitySlot& slt { entities_.slot(i) };
        if (slt.active && (slt.data.type == EntityType::PatrolWalker || slt.data.type == EntityType::HomingFlyer)) {
            candidates[candCount].id = static_cast<std::uint32_t>(i);
            candidates[candCount].bounds = slt.data.bounds;
            candidates[candCount].type = TargetType::Hostile;
            candidates[candCount].active = true;
            ++candCount;
        }
    }

    // 2. Also harvest adjacent destructible blocks from TileMap near player
    const TileCoord playerTile { playerPos_.toTileCoord(kTileSize) };
    const std::uint32_t minC { playerTile.col > 3U ? playerTile.col - 3U : 0U };
    const std::uint32_t maxC { std::min(map_.width(), playerTile.col + 4U) };
    const std::uint32_t minR { playerTile.row > 3U ? playerTile.row - 3U : 0U };
    const std::uint32_t maxR { std::min(map_.height(), playerTile.row + 4U) };

    for (std::uint32_t r { minR }; r < maxR && candCount < WeaponSystem::kMaxTargets; ++r) {
        for (std::uint32_t c { minC }; c < maxC && candCount < WeaponSystem::kMaxTargets; ++c) {
            const TileCoord coord { c, r };
            if (map_.getTile(coord) == TileType::DestructibleWall) {
                const WorldPos minPt { static_cast<float>(c) * kTileSize, static_cast<float>(r) * kTileSize };
                const WorldPos maxPt { minPt.x + kTileSize, minPt.y + kTileSize };
                candidates[candCount].id = (r * map_.width()) + c + 1000U;
                candidates[candCount].bounds = Hitbox { minPt, maxPt };
                candidates[candCount].type = TargetType::Destructible;
                candidates[candCount].active = true;
                ++candCount;
            }
        }
    }

    // 3. Process weapons
    weaponSys_.updateWeapons(dt, input.isFire(), input.isShield(), transformSys_.isTransforming(), transformSys_.mode(),
        playerFacing_, playerBox_, map_, candidates, candCount, energy_);

    // 4. Trigger audio cues
    if (weaponSys_.isLaserActive()) {
        audioQueue_.pushSound(SoundEffectId::LaserFire, 0.4f);
    }
    if (weaponSys_.isShieldActive()) {
        audioQueue_.pushSound(SoundEffectId::ShieldDeploy, 0.4f);
    }

    // 5. Apply laser damage to locked entity
    const BeamSegment beam { weaponSys_.getBeamSegment() };
    if (beam.active && beam.lockedToTarget && beam.targetId < 1000U) {
        const std::size_t slotIdx { static_cast<std::size_t>(beam.targetId) };
        if (slotIdx < entities_.capacity()) {
            EntitySlot& slt { entities_.slot(slotIdx) };
            if (slt.active) {
                if (slt.data.health > 1U) {
                    --slt.data.health;
                } else {
                    slt.data.health = 0U;
                    const EntityId eid { static_cast<std::uint16_t>(slotIdx), slt.generation };
                    entities_.despawn(eid);
                    energy_.replenish(EnergyPoints { 5U });
                    audioQueue_.pushSound(SoundEffectId::EnemyExplode);
                }
            }
        }
    }
}

void GameEngine::updateEntities(DeltaTime dt) noexcept
{
    const float dtSec { dt.seconds() };

    for (std::size_t i { 0U }; i < entities_.capacity(); ++i) {
        EntitySlot& slt { entities_.slot(i) };
        if (!slt.active) {
            continue;
        }

        Entity& ent { slt.data };

        if (ent.type == EntityType::PatrolWalker) {
            const float speed { ent.facing == FacingDirection::Left ? -40.0f : 40.0f };
            ent.vel.dx = speed;

            const Velocity evx { ent.vel.dx, 0.0f };
            const SweepHit ehit { CollisionSolver::sweepMap(ent.bounds, evx, dtSec, map_) };
            if (ehit.collided) {
                ent.facing = (ent.facing == FacingDirection::Left) ? FacingDirection::Right : FacingDirection::Left;
            } else {
                ent.pos.x += ent.vel.dx * dtSec;
            }
            ent.bounds = Hitbox { ent.pos, WorldPos { ent.pos.x + 16.0f, ent.pos.y + 16.0f } };
        }

        // Check overlap with player craft
        if (ent.bounds.intersects(playerBox_)) {
            const DamageValue dmg { 10U };
            const bool absorbed { weaponSys_.absorbImpact(dmg, energy_) };
            if (absorbed) {
                audioQueue_.pushSound(SoundEffectId::ShieldDeflect);
            } else {
                energy_.deduct(EnergyPoints { dmg.value() });
                audioQueue_.pushSound(SoundEffectId::PlayerDamage);
                if (energy_.isDepleted()) {
                    isAlive_ = false;
                }
            }
        }
    }
}

void GameEngine::updateCamera() noexcept
{
    const float halfW { camera_.width * 0.5f };
    const float halfH { camera_.height * 0.5f };
    const float mapPixelW { static_cast<float>(map_.width()) * kTileSize };
    const float mapPixelH { static_cast<float>(map_.height()) * kTileSize };

    const float maxCamX { std::max(0.0f, mapPixelW - camera_.width) };
    const float maxCamY { std::max(0.0f, mapPixelH - camera_.height) };

    camera_.pos.x = std::clamp(playerPos_.x - halfW, 0.0f, maxCamX);
    camera_.pos.y = std::clamp(playerPos_.y - halfH, 0.0f, maxCamY);
}

void GameEngine::emitCommands() noexcept
{
    // 1. Viewport-culled tiles
    const auto startCol { static_cast<std::size_t>(camera_.pos.x / kTileSize) };
    const std::size_t endCol { std::min(static_cast<std::size_t>(map_.width()),
        static_cast<std::size_t>((camera_.pos.x + camera_.width) / kTileSize) + 1U) };
    const auto startRow { static_cast<std::size_t>(camera_.pos.y / kTileSize) };
    const std::size_t endRow { std::min(static_cast<std::size_t>(map_.height()),
        static_cast<std::size_t>((camera_.pos.y + camera_.height) / kTileSize) + 1U) };

    for (std::size_t r { startRow }; r < endRow; ++r) {
        for (std::size_t c { startCol }; c < endCol; ++c) {
            const TileCoord coord { static_cast<std::uint32_t>(c), static_cast<std::uint32_t>(r) };
            const TileType tt { map_.getTile(coord) };
            if (tt != TileType::Empty) {
                const WorldPos tilePos { static_cast<float>(c) * kTileSize, static_cast<float>(r) * kTileSize };
                renderQueue_.pushTile(tt, tilePos, 0U);
            }
        }
    }

    // 2. Active entities
    for (std::size_t i { 0U }; i < entities_.capacity(); ++i) {
        const EntitySlot& slt { entities_.slot(i) };
        if (!slt.active) {
            continue;
        }

        const SpriteId spr { (slt.data.type == EntityType::PatrolWalker) ? SpriteId::EnemyPatrol
                                                                         : SpriteId::EnemyFlyer };
        renderQueue_.pushSprite(spr, slt.data.pos, slt.data.facing, 5U);
    }

    // 3. Player craft
    SpriteId playerSpr { SpriteId::PlayerRobotWalk1 };
    if (transformSys_.isTransforming()) {
        playerSpr = SpriteId::PlayerTransforming;
    } else if (transformSys_.mode() == TransformMode::Jet) {
        playerSpr = SpriteId::PlayerJet;
    } else if (!onGround_) {
        playerSpr = SpriteId::PlayerRobotJump;
    }
    renderQueue_.pushSprite(playerSpr, playerPos_, playerFacing_, 10U);

    // 4. Laser beam
    if (weaponSys_.isLaserActive()) {
        const BeamSegment beam { weaponSys_.getBeamSegment() };
        renderQueue_.pushBeam(beam.origin, beam.endpoint, ColorRgba { 255U, 64U, 64U, 255U });
    }

    // 5. Shield barrier
    if (weaponSys_.isShieldActive()) {
        const Hitbox shieldBox { WeaponSystem::computeShield(playerBox_) };
        renderQueue_.pushBar(
            shieldBox.min(), shieldBox.width(), shieldBox.height(), 1.0f, ColorRgba { 0U, 180U, 255U, 128U });
    }

    // 6. HUD energy bar
    const float energyPct { static_cast<float>(energy_.value()) / static_cast<float>(EnergyPoints::kDefaultMax) };
    renderQueue_.pushBar(WorldPos { 8.0f, 8.0f }, 100.0f, 6.0f, energyPct, ColorRgba { 0U, 220U, 255U, 255U });
}

} // namespace ShooterCore
