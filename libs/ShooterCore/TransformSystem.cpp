#include "TransformSystem.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>

#include <glog/logging.h>

#include "Hitbox.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore {

TransformResult TransformSystem::requestTransform(WorldPos currentPos, const TileMap& map, AnchorMode anchor) noexcept
{
    if (isTransforming()) {
        return TransformResult::Busy;
    }

    const TransformMode targetMode { (mode_ == TransformMode::Robot) ? TransformMode::Jet : TransformMode::Robot };

    const Hitbox projected { projectBox(currentPos, mode_, targetMode, anchor) };

    if (!map.hasClearance(projected)) {
        LOG(WARNING) << "TransformSystem: Transformation blocked due to insufficient clearance.";
        return TransformResult::RejectedObstruction;
    }

    if (targetMode == TransformMode::Jet) {
        mode_ = TransformMode::TransformingToJet;
    } else {
        mode_ = TransformMode::TransformingToRobot;
    }

    LOG(INFO) << "TransformSystem: Initiating transformation to "
              << ((targetMode == TransformMode::Jet) ? "Jet" : "Robot") << " mode.";

    transitionTimer_ = 0.0f;
    return TransformResult::Success;
}

void TransformSystem::update(DeltaTime dt) noexcept
{
    if (!isTransforming()) {
        return;
    }

    transitionTimer_ += dt.seconds();

    if (transitionTimer_ >= transitionDuration_) {
        if (mode_ == TransformMode::TransformingToJet) {
            mode_ = TransformMode::Jet;
        } else if (mode_ == TransformMode::TransformingToRobot) {
            mode_ = TransformMode::Robot;
        }
        LOG(INFO) << "TransformSystem: Transformation complete. Current mode: "
                  << ((mode_ == TransformMode::Jet) ? "Jet" : "Robot");
        transitionTimer_ = 0.0f;
    }
}

void TransformSystem::forceState(TransformMode targetMode) noexcept
{
    mode_ = targetMode;
    transitionTimer_ = 0.0f;
}

Hitbox TransformSystem::projectBox(
    WorldPos currentPos, TransformMode currentMode, TransformMode targetMode, AnchorMode anchor) noexcept
{
    float curW { 0.0f };
    float curH { 0.0f };
    getDims(currentMode, curW, curH);

    float tgtW { 0.0f };
    float tgtH { 0.0f };
    getDims(targetMode, tgtW, tgtH);

    float minX { currentPos.x };
    float minY { currentPos.y };

    if (anchor == AnchorMode::BottomCenter) {
        const float curBottomY { currentPos.y + curH };
        const float curCenterX { currentPos.x + (curW * 0.5f) };
        minX = curCenterX - (tgtW * 0.5f);
        minY = curBottomY - tgtH;
    } else if (anchor == AnchorMode::Center) {
        const float curCenterX { currentPos.x + (curW * 0.5f) };
        const float curCenterY { currentPos.y + (curH * 0.5f) };
        minX = curCenterX - (tgtW * 0.5f);
        minY = curCenterY - (tgtH * 0.5f);
    } else {
        minX = currentPos.x;
        minY = currentPos.y;
    }

    return Hitbox::fromDims(WorldPos { minX, minY }, tgtW, tgtH);
}

} // namespace ShooterCore
