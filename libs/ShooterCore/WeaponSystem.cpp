#include "WeaponSystem.h"

#include <cmath>
#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <array>

#include <glog/logging.h>

#include "CollisionSolver.h"
#include "Hitbox.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore {

WorldPos WeaponSystem::computeMuzzle(const Hitbox& craftBox, FacingDirection facing) noexcept
{
    switch (facing) {
    case FacingDirection::Right:
        return WorldPos { craftBox.max().x, craftBox.min().y + (craftBox.height() * 0.5f) };
    case FacingDirection::Left:
        return WorldPos { craftBox.min().x, craftBox.min().y + (craftBox.height() * 0.5f) };
    case FacingDirection::Up:
        return WorldPos { craftBox.min().x + (craftBox.width() * 0.5f), craftBox.min().y };
    case FacingDirection::Down:
        return WorldPos { craftBox.min().x + (craftBox.width() * 0.5f), craftBox.max().y };
    default:
        return WorldPos { craftBox.max().x, craftBox.min().y + (craftBox.height() * 0.5f) };
    }
}

std::int32_t WeaponSystem::findBestTarget(WorldPos muzzle, FacingDirection facing, TransformMode mode,
    const TileMap& map, const std::array<TargetCandidate, kMaxTargets>& targets, std::size_t targetCount) noexcept
{
    if (targetCount == 0U) {
        return -1;
    }

    FacingDirection effectiveFacing { facing };
    if (mode == TransformMode::Jet
        && (effectiveFacing == FacingDirection::Up || effectiveFacing == FacingDirection::Down)) {
        effectiveFacing = FacingDirection::Right;
    }

    float fx { 1.0f };
    float fy { 0.0f };
    switch (effectiveFacing) {
    case FacingDirection::Right:
        fx = 1.0f;
        fy = 0.0f;
        break;
    case FacingDirection::Left:
        fx = -1.0f;
        fy = 0.0f;
        break;
    case FacingDirection::Up:
        fx = 0.0f;
        fy = -1.0f;
        break;
    case FacingDirection::Down:
        fx = 0.0f;
        fy = 1.0f;
        break;
    default:
        break;
    }

    const float maxRange { (mode == TransformMode::Jet) ? kLaserRangeJet : kLaserRangeRobot };
    const float coneCos { (mode == TransformMode::Jet) ? kJetConeCos : kRobotConeCos };

    std::int32_t bestIdx { -1 };
    float bestScore { -1.0f };

    const std::size_t candidateLimit { std::min(targetCount, kMaxTargets) };
    for (std::size_t i { 0U }; i < candidateLimit; ++i) {
        if (!targets[i].active) {
            continue;
        }

        const WorldPos centerPos { targets[i].center() };
        const float dx { centerPos.x - muzzle.x };
        const float dy { centerPos.y - muzzle.y };
        const float dist { std::sqrt((dx * dx) + (dy * dy)) };

        if (dist <= 0.001f || dist > maxRange) {
            continue;
        }

        const float cosAngle { ((dx * fx) + (dy * fy)) / dist };
        if (cosAngle < coneCos) {
            continue;
        }

        WorldPos hitObstacle { };
        const bool hitWall { CollisionSolver::raycast(muzzle, centerPos, map, hitObstacle) };
        if (hitWall) {
            const float hitDx { hitObstacle.x - muzzle.x };
            const float hitDy { hitObstacle.y - muzzle.y };
            const float hitDist { std::sqrt((hitDx * hitDx) + (hitDy * hitDy)) };
            if (hitDist < (dist - 2.0f)) {
                continue;
            }
        }

        const float distScore { 0.50f * (1.0f - (dist / maxRange)) };
        const float angleScore { 0.30f * std::max(0.0f, cosAngle) };
        const float threatScore { (targets[i].type == TargetType::Hostile) ? 0.20f : 0.0f };
        const float totalScore { distScore + angleScore + threatScore };

        if (totalScore > bestScore) {
            bestScore = totalScore;
            bestIdx = static_cast<std::int32_t>(i);
        }
    }

    return bestIdx;
}

void WeaponSystem::updateWeapons(DeltaTime dt, bool fireIntent, bool shieldIntent, bool isTransforming,
    TransformMode mode, FacingDirection facing, const Hitbox& craftBox, TileMap& map,
    const std::array<TargetCandidate, kMaxTargets>& targets, std::size_t targetCount, EnergyPoints& energy) noexcept
{
    const bool canOperate { !isTransforming && !energy.isDepleted() };
    const bool shouldFire { fireIntent && canOperate };
    const bool shouldShield { shieldIntent && canOperate };

    if (shouldFire) {
        laserActive_ = true;
        laserDrain_.accumulate(kLaserDrainRate, dt.seconds());

        FacingDirection effectiveFacing { facing };
        if (mode == TransformMode::Jet
            && (effectiveFacing == FacingDirection::Up || effectiveFacing == FacingDirection::Down)) {
            effectiveFacing = FacingDirection::Right;
        }

        float fx { 1.0f };
        float fy { 0.0f };
        switch (effectiveFacing) {
        case FacingDirection::Right:
            fx = 1.0f;
            fy = 0.0f;
            break;
        case FacingDirection::Left:
            fx = -1.0f;
            fy = 0.0f;
            break;
        case FacingDirection::Up:
            fx = 0.0f;
            fy = -1.0f;
            break;
        case FacingDirection::Down:
            fx = 0.0f;
            fy = 1.0f;
            break;
        default:
            break;
        }

        const float maxRange { (mode == TransformMode::Jet) ? kLaserRangeJet : kLaserRangeRobot };
        const WorldPos muzzle { computeMuzzle(craftBox, effectiveFacing) };
        const std::int32_t bestIdx { findBestTarget(muzzle, effectiveFacing, mode, map, targets, targetCount) };

        if (bestIdx >= 0) {
            const auto targetIndex { static_cast<std::size_t>(bestIdx) };
            beam_.active = true;
            beam_.lockedToTarget = true;
            beam_.targetId = targets[targetIndex].id;
            beam_.origin = muzzle;
            beam_.endpoint = targets[targetIndex].center();

            if (targets[targetIndex].type == TargetType::Destructible) {
                tileDamagePacer_.accumulate(kTileDamageRate, dt.seconds());
                const TileCoord coord { beam_.endpoint.toTileCoord(map.tileSize()) };
                while (tileDamagePacer_.consumeWhole() > 0U) {
                    map.damageTile(coord);
                }
            } else {
                tileDamagePacer_.reset();
            }
        } else {
            const WorldPos projectedEnd { muzzle.x + (fx * maxRange), muzzle.y + (fy * maxRange) };
            WorldPos wallHit { };
            const bool hitSolid { CollisionSolver::raycast(muzzle, projectedEnd, map, wallHit) };

            beam_.active = true;
            beam_.lockedToTarget = false;
            beam_.targetId = 0U;
            beam_.origin = muzzle;

            if (hitSolid) {
                beam_.endpoint = wallHit;
                const TileCoord coord { wallHit.toTileCoord(map.tileSize()) };
                if (map.isDestructible(coord)) {
                    tileDamagePacer_.accumulate(kTileDamageRate, dt.seconds());
                    while (tileDamagePacer_.consumeWhole() > 0U) {
                        map.damageTile(coord);
                    }
                } else {
                    tileDamagePacer_.reset();
                }
            } else {
                beam_.endpoint = projectedEnd;
                tileDamagePacer_.reset();
            }
        }
    } else {
        laserActive_ = false;
        beam_.active = false;
        beam_.lockedToTarget = false;
        beam_.targetId = 0U;
        tileDamagePacer_.reset();
    }

    if (shouldShield) {
        shieldActive_ = true;
        shieldDrain_.accumulate(kShieldDrainRate, dt.seconds());
    } else {
        shieldActive_ = false;
    }

    const std::uint16_t laserDeduction { laserDrain_.consumeWhole() };
    const std::uint16_t shieldDeduction { shieldDrain_.consumeWhole() };
    const auto totalDeduction { static_cast<std::uint16_t>(laserDeduction + shieldDeduction) };

    if (totalDeduction > 0U) {
        energy.deduct(EnergyPoints { totalDeduction });
    }

    if (energy.isDepleted()) {
        laserActive_ = false;
        shieldActive_ = false;
        beam_.active = false;
        beam_.lockedToTarget = false;
        beam_.targetId = 0U;
    }
}

bool WeaponSystem::absorbImpact(DamageValue damage, EnergyPoints& energy) noexcept
{
    if (!shieldActive_) {
        return false;
    }

    const std::uint16_t cost { damage.value() };
    energy.deduct(EnergyPoints { cost });

    LOG(INFO) << "WeaponSystem: Energy barrier absorbed " << cost << " damage points.";

    if (energy.isDepleted()) {
        shieldActive_ = false;
        LOG(WARNING) << "WeaponSystem: Energy barrier collapsed due to energy exhaustion.";
    }

    return true;
}

void WeaponSystem::reset() noexcept
{
    laserActive_ = false;
    shieldActive_ = false;
    beam_ = BeamSegment { };
    laserDrain_.reset();
    shieldDrain_.reset();
    tileDamagePacer_.reset();
}

} // namespace ShooterCore
