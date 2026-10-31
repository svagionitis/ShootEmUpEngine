#include "GameEngine.h"

#include <cmath>
#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <array>

#include <glog/logging.h>

#include "AnimationController.h"
#include "BossController.h"
#include "CameraSystem.h"
#include "CollisionSolver.h"
#include "EnemyAiSystem.h"
#include "Hitbox.h"
#include "LabyrinthMap.h"
#include "MinimapRadar.h"
#include "NestManager.h"
#include "ParticlePool.h"
#include "SecondaryArsenal.h"
#include "SecretManager.h"
#include "SecuritySystem.h"
#include "SupplyDropSystem.h"
#include "TileMap.h"
#include "TransformSystem.h"
#include "Types.h"
#include "WeaponSystem.h"

namespace ShooterCore {

GameEngine::GameEngine() noexcept
{
    cameraSys_.setPosition(WorldPos { 0.0f, 0.0f });
    camera_ = cameraSys_.viewport();
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
    LOG(INFO) << "GameEngine: Initializing level with player spawn at (" << playerSpawn.x << ", " << playerSpawn.y
              << ").";

    map_ = map;
    entities_.clear();
    particlePool_.clear();
    secondaryArsenal_.clear();
    supplyDropSys_.clear();
    nestManager_.clear();
    renderQueue_.clear();
    audioQueue_.clear();
    prevCycleInput_ = false;

    playerPos_ = playerSpawn;
    playerVel_ = Velocity { 0.0f, 0.0f };
    playerFacing_ = FacingDirection::Right;
    playerMaxEnergy_ = EnergyPoints::kDefaultMax;
    energy_ = EnergyPoints { playerMaxEnergy_ };
    onGround_ = false;
    isAlive_ = true;

    playerBox_ = computeHitbox(playerPos_);

    cameraSys_.setPosition(playerPos_);
    updateCamera();
}

EntityId GameEngine::spawnEnemy(EntityType type, WorldPos pos, Hitbox bounds, std::uint16_t hp) noexcept
{
    const EntityId id { entities_.spawn(type, pos, bounds, hp) };
    if (id.isValid()) {
        LOG(INFO) << "GameEngine: Spawned enemy type " << static_cast<unsigned>(type) << " at (" << pos.x << ", "
                  << pos.y << ") with ID " << id.index() << " (gen " << id.generation() << ").";
    } else {
        LOG(WARNING) << "GameEngine: Failed to spawn enemy: Entity pool capacity exhausted.";
    }
    return id;
}

void GameEngine::step(DeltaTime dt, EngineInput input) noexcept
{
    renderQueue_.clear();
    audioQueue_.clear();

    if (!isAlive_) {
        return;
    }

    anim_.update(dt);
    particlePool_.update(dt);
    minimapRadar_.updateDiscovery(playerPos_);
    secretMgr_.checkPickup(playerBox_, energy_, playerMaxEnergy_, audioQueue_);
    securitySys_.syncTiles(map_);

    if (bossController_.phase() != BossPhase::Dormant && !bossController_.isDefeated()) {
        bossController_.update(dt, playerPos_, entities_, particlePool_, audioQueue_);
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
    playerCrouch_ = input.isDown() && (transformSys_.mode() == TransformMode::Robot);

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

    if (input.isCycleWeapon() && !prevCycleInput_) {
        secondaryArsenal_.cycleWeapon();
        audioQueue_.pushSound(SoundEffectId::WeaponSelect);
    }
    prevCycleInput_ = input.isCycleWeapon();

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
        if (slt.active
            && (slt.data.type == EntityType::PatrolWalker || slt.data.type == EntityType::HomingFlyer
                || slt.data.type == EntityType::DivingInterceptor || slt.data.type == EntityType::WallCrawler
                || slt.data.type == EntityType::CeilingTurret)) {
            candidates[candCount].id = static_cast<std::uint32_t>(i);
            candidates[candCount].bounds = slt.data.bounds;
            candidates[candCount].type = TargetType::Hostile;
            candidates[candCount].active = true;
            ++candCount;
        }
    }

    // 2. Harvest active BioNests
    const auto& nests { nestManager_.getNests() };
    for (std::size_t nIdx { 0U }; nIdx < nestManager_.nestCount() && candCount < WeaponSystem::kMaxTargets; ++nIdx) {
        if (nests[nIdx].active && !nests[nIdx].destroyed) {
            candidates[candCount].id = static_cast<std::uint32_t>(2000U + nIdx);
            candidates[candCount].bounds = Hitbox { WorldPos { nests[nIdx].pos.x - 16.0f, nests[nIdx].pos.y - 16.0f },
                WorldPos { nests[nIdx].pos.x + 16.0f, nests[nIdx].pos.y + 16.0f } };
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

    // 5. Apply laser damage to locked entity & emit visual FX
    const BeamSegment beam { weaponSys_.getBeamSegment() };
    if (beam.active) {
        const Hitbox beamHit { beam.endpoint, WorldPos { beam.endpoint.x + 8.0f, beam.endpoint.y + 8.0f } };
        securitySys_.damageAt(beamHit, 1U, audioQueue_);
        secretMgr_.damageSecretWall(beam.endpoint.toTileCoord(kTileSize), map_, audioQueue_);
        if (bossController_.phase() != BossPhase::Dormant && !bossController_.isDefeated()) {
            bossController_.applyDamage(beamHit, 1U, audioQueue_);
        }

        if (beam.lockedToTarget && beam.targetId < 1000U) {
            const std::size_t slotIdx { static_cast<std::size_t>(beam.targetId) };
            if (slotIdx < entities_.capacity()) {
                EntitySlot& slt { entities_.slot(slotIdx) };
                if (slt.active) {
                    particlePool_.emitSparks(beam.endpoint, 0.0f, 3U);
                    if (slt.data.health > 1U) {
                        --slt.data.health;
                    } else {
                        slt.data.health = 0U;
                        const EntityId eid { static_cast<std::uint16_t>(slotIdx), slt.generation };
                        entities_.despawn(eid);
                        LOG(INFO) << "GameEngine: Enemy ID " << eid.index() << " destroyed. Energy replenished (+5).";
                        energy_.replenish(EnergyPoints { 5U });
                        audioQueue_.pushSound(SoundEffectId::EnemyExplode);
                        particlePool_.emitDebris(beam.endpoint, 8U);
                        particlePool_.emitSparks(beam.endpoint, 0.0f, 6U);
                    }
                }
            }
        } else if (beam.lockedToTarget && beam.targetId >= 2000U) {
            const std::size_t nestIdx { static_cast<std::size_t>(beam.targetId - 2000U) };
            nestManager_.damageNest(nestIdx, DamageValue { 1U }, supplyDropSys_, particlePool_, audioQueue_);
        } else if (beam.lockedToTarget && beam.targetId >= 1000U) {
            particlePool_.emitDebris(beam.endpoint, 2U);
            particlePool_.emitSparks(beam.endpoint, 0.0f, 2U);
        } else {
            // Unlocked beam striking surface
            particlePool_.emitSparks(beam.endpoint, 0.0f, 1U);
        }
    }

    // 6. Secondary Arsenal & Tactical AI
    if (input.isFireSecondary()) {
        secondaryArsenal_.triggerFire(playerPos_, playerFacing_, entities_, audioQueue_, particlePool_);
    }

    secondaryArsenal_.update(dt, entities_, map_, particlePool_, audioQueue_);
    nestManager_.update(dt, playerPos_, entities_, supplyDropSys_, particlePool_, audioQueue_);
    enemyAi_.update(
        dt, playerPos_, playerBox_, weaponSys_.isShieldActive(), energy_, entities_, map_, audioQueue_, particlePool_);
    supplyDropSys_.update(dt, playerBox_, map_, secondaryArsenal_, energy_, playerShield_, audioQueue_, particlePool_);
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
                particlePool_.emitShieldSparks(ent.pos, 0.0f, 8U);
            } else {
                energy_.deduct(EnergyPoints { dmg.value() });
                audioQueue_.pushSound(SoundEffectId::PlayerDamage);
                particlePool_.emitSparks(playerPos_, 0.0f, 6U);
                if (energy_.isDepleted()) {
                    isAlive_ = false;
                }
            }
        }
    }
}

void GameEngine::updateCamera() noexcept
{
    const SectorBounds sector { WorldPos { 0.0f, 0.0f },
        WorldPos { static_cast<float>(map_.width()) * kTileSize, static_cast<float>(map_.height()) * kTileSize } };

    cameraSys_.update(DeltaTime { 0.016f }, playerPos_, sector);

    if (LabyrinthMap::getBossArenaBounds().contains(playerPos_) && bossController_.phase() != BossPhase::Defeated) {
        cameraSys_.lockArena(LabyrinthMap::getBossArenaBounds());
    }

    camera_ = cameraSys_.viewport();
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

        SpriteId spr { anim_.currentEnemySprite(slt.data.type) };
        if (spr == SpriteId::None) {
            if (slt.data.type == EntityType::PatrolWalker) {
                spr = SpriteId::EnemyPatrol;
            } else if (slt.data.type == EntityType::DivingInterceptor) {
                spr = SpriteId::EnemyInterceptor;
            } else if (slt.data.type == EntityType::WallCrawler) {
                spr = SpriteId::EnemyCrawler1;
            } else if (slt.data.type == EntityType::CeilingTurret) {
                spr = SpriteId::EnemyTurretCeiling;
            } else if (slt.data.type == EntityType::HostileBullet) {
                spr = SpriteId::EnemyBulletHostile;
            } else {
                spr = SpriteId::EnemyFlyer;
            }
        }
        renderQueue_.pushSprite(spr, slt.data.pos, slt.data.facing, 5U);
    }

    // 2b. BioNests
    const auto& nests { nestManager_.getNests() };
    for (std::size_t i { 0U }; i < nestManager_.nestCount(); ++i) {
        if (nests[i].active && !nests[i].destroyed) {
            renderQueue_.pushSprite(SpriteId::NestCoreActive, nests[i].pos, FacingDirection::Right, 4U);
        }
    }

    // 2c. Secondary Ordnance: Missiles & Shockwaves
    const auto& missiles { secondaryArsenal_.getMissiles() };
    for (std::size_t i { 0U }; i < SecondaryArsenal::kMaxMissiles; ++i) {
        if (missiles[i].active) {
            renderQueue_.pushSprite(
                SpriteId::MissileInFlight, missiles[i].pos, FacingDirection::Right, 8U, missiles[i].angleDeg);
        }
    }

    const FlashBombState& fb { secondaryArsenal_.getFlashBomb() };
    if (fb.active && fb.duration > 0.0f) {
        const float alpha { std::clamp(1.0f - (fb.timer / fb.duration), 0.0f, 1.0f) };
        renderQueue_.pushShockwave(fb.center, fb.currentRadius, alpha, 14U);
    }

    // 2d. Cargo Canisters
    const auto& canisters { supplyDropSys_.getCanisters() };
    for (std::size_t i { 0U }; i < SupplyDropSystem::kMaxCanisterSlots; ++i) {
        if (canisters[i].active) {
            SpriteId spr { SpriteId::CanisterMissile };
            if (canisters[i].type == CanisterType::FlashBomb) {
                spr = SpriteId::CanisterBomb;
            } else if (canisters[i].type == CanisterType::EnergyBoost) {
                spr = SpriteId::CanisterEnergy;
            } else if (canisters[i].type == CanisterType::ShieldRecharge) {
                spr = SpriteId::CanisterShield;
            }
            const WorldPos bobPos { canisters[i].pos.x, canisters[i].pos.y + (std::sin(canisters[i].bobTimer) * 2.0f) };
            renderQueue_.pushSprite(spr, bobPos, FacingDirection::Right, 6U);
        }
    }

    // 3. Player craft
    SpriteId playerSpr { SpriteId::PlayerRobotIdle };
    if (transformSys_.isTransforming()) {
        const float morphProgress { transformSys_.progress() };
        const bool toJet { transformSys_.mode() == TransformMode::TransformingToJet };
        playerSpr = anim_.currentMorphSprite(morphProgress, toJet);
    } else if (transformSys_.mode() == TransformMode::Jet) {
        playerSpr = anim_.currentJetSprite();

        // Emit animated afterburner exhaust
        const SpriteId exhaustSpr { anim_.currentJetExhaust() };
        const float exOffset { (playerFacing_ == FacingDirection::Left) ? 16.0f : -24.0f };
        renderQueue_.pushSprite(exhaustSpr, WorldPos { playerPos_.x + exOffset, playerPos_.y }, playerFacing_, 9U);
    } else {
        const bool isMoving { std::abs(playerVel_.dx) > 5.0f };
        playerSpr = anim_.currentRobotSprite(isMoving, !onGround_, playerCrouch_);
    }
    renderQueue_.pushSprite(playerSpr, playerPos_, playerFacing_, 10U);

    // 4. Laser beam & lightning arc
    if (weaponSys_.isLaserActive()) {
        const LightningArc& arc { weaponSys_.getLightningArc() };
        renderQueue_.pushLightning(arc, ColorRgba { 255U, 255U, 255U, 255U }, ColorRgba { 0U, 180U, 255U, 160U }, 12U);

        const BeamSegment beam { weaponSys_.getBeamSegment() };
        renderQueue_.pushBeam(beam.origin, beam.endpoint, ColorRgba { 0U, 220U, 255U, 255U });
    }

    // 5. Shield barrier
    if (weaponSys_.isShieldActive()) {
        const Hitbox shieldBox { WeaponSystem::computeShield(playerBox_) };
        const ShieldFxState& fx { weaponSys_.getShieldFx() };
        renderQueue_.pushShield(shieldBox, fx, 11U);
        renderQueue_.pushBar(
            shieldBox.min(), shieldBox.width(), shieldBox.height(), 1.0f, ColorRgba { 0U, 180U, 255U, 128U });
    }

    // 6. Visual FX particles
    const auto& particles { particlePool_.particles() };
    for (std::size_t i { 0U }; i < ParticlePool::kCapacity; ++i) {
        if (particles[i].active) {
            renderQueue_.pushParticle(particles[i].pos, particles[i].color, particles[i].size, 13U);
        }
    }

    // 7. HUD energy bar
    const float energyPct { static_cast<float>(energy_.value()) / static_cast<float>(playerMaxEnergy_) };
    renderQueue_.pushBar(WorldPos { 8.0f, 8.0f }, 100.0f, 6.0f, energyPct, ColorRgba { 0U, 220U, 255U, 255U });

    // 8. Phase 12: Security barriers & power generators
    securitySys_.emitRender(renderQueue_);

    // 9. Phase 12: Concealed Energy Tanks
    secretMgr_.emitRender(renderQueue_);

    // 10. Phase 12: Multi-segment Dreadnought Boss
    bossController_.emitRender(renderQueue_);

    // 11. Phase 12: Cockpit HUD Minimap Radar
    minimapRadar_.emitCommands(renderQueue_, playerPos_, securitySys_, bossController_.corePosition(),
        bossController_.phase() != BossPhase::Dormant && !bossController_.isDefeated());
}

} // namespace ShooterCore
