#include "TestHarness.h"

#include <cstdint>

#include "AnimationController.h"
#include "Types.h"

namespace ShooterCore::Test {

void runAnimationTests() noexcept
{
    // Test 1: Initial state & reset invariants
    {
        AnimationController anim;
        TEST_ASSERT_EQ(anim.walkFrameIndex(), 0U, "Initial walk frame should be 0");
        TEST_ASSERT_EQ(anim.exhaustFrameIndex(), 0U, "Initial exhaust frame should be 0");

        anim.update(DeltaTime { 0.200f });
        TEST_ASSERT(anim.walkFrameIndex() > 0U, "Walk frame should advance after 200ms");

        anim.reset();
        TEST_ASSERT_EQ(anim.walkFrameIndex(), 0U, "Reset should restore walk frame to 0");
        TEST_ASSERT_EQ(anim.exhaustFrameIndex(), 0U, "Reset should restore exhaust frame to 0");
    }

    // Test 2: Walk cycle 3-frame stride progression
    {
        AnimationController anim;
        TEST_ASSERT(anim.currentRobotSprite(false, false, false) == SpriteId::PlayerRobotIdle,
            "Stationary grounded robot should display Idle frame");

        TEST_ASSERT(anim.currentRobotSprite(true, false, false) == SpriteId::PlayerRobotWalk1,
            "Initial moving robot should display Walk1");

        anim.update(DeltaTime { AnimationController::kWalkFrameDuration });
        TEST_ASSERT(anim.currentRobotSprite(true, false, false) == SpriteId::PlayerRobotWalk2,
            "After 1 frame duration, moving robot should display Walk2");

        anim.update(DeltaTime { AnimationController::kWalkFrameDuration });
        TEST_ASSERT(anim.currentRobotSprite(true, false, false) == SpriteId::PlayerRobotWalk3,
            "After 2 frame durations, moving robot should display Walk3");

        anim.update(DeltaTime { AnimationController::kWalkFrameDuration });
        TEST_ASSERT(anim.currentRobotSprite(true, false, false) == SpriteId::PlayerRobotWalk1,
            "After 3 frame durations, walk cycle should wrap to Walk1");
    }

    // Test 3: Crouch and Jump state precedence
    {
        AnimationController anim;
        TEST_ASSERT(anim.currentRobotSprite(true, false, true) == SpriteId::PlayerRobotCrouch,
            "Crouching state should take priority over horizontal movement");

        TEST_ASSERT(anim.currentRobotSprite(true, true, false) == SpriteId::PlayerRobotJump,
            "Airborne/jumping state should take priority over horizontal movement");
    }

    // Test 4: Transformation morph progression
    {
        AnimationController anim;
        // Transforming to Jet
        TEST_ASSERT(anim.currentMorphSprite(0.10f, true) == SpriteId::PlayerMorph1, "Early morph to jet is Morph1");
        TEST_ASSERT(anim.currentMorphSprite(0.50f, true) == SpriteId::PlayerMorph2, "Mid morph to jet is Morph2");
        TEST_ASSERT(anim.currentMorphSprite(0.85f, true) == SpriteId::PlayerMorph3, "Late morph to jet is Morph3");

        // Transforming back to Robot (reversed order)
        TEST_ASSERT(anim.currentMorphSprite(0.10f, false) == SpriteId::PlayerMorph3, "Early morph to robot is Morph3");
        TEST_ASSERT(anim.currentMorphSprite(0.50f, false) == SpriteId::PlayerMorph2, "Mid morph to robot is Morph2");
        TEST_ASSERT(anim.currentMorphSprite(0.85f, false) == SpriteId::PlayerMorph1, "Late morph to robot is Morph1");
    }

    // Test 5: Jet afterburner flame flicker
    {
        AnimationController anim;
        TEST_ASSERT(anim.currentJetSprite() == SpriteId::PlayerJet, "Jet chassis should be constant PlayerJet");
        TEST_ASSERT(anim.currentJetExhaust() == SpriteId::PlayerJetExhaust1, "Initial exhaust frame is Exhaust1");

        anim.update(DeltaTime { AnimationController::kExhaustFrameDuration });
        TEST_ASSERT(anim.currentJetExhaust() == SpriteId::PlayerJetExhaust2, "Exhaust should toggle to Exhaust2");

        anim.update(DeltaTime { AnimationController::kExhaustFrameDuration });
        TEST_ASSERT(anim.currentJetExhaust() == SpriteId::PlayerJetExhaust1, "Exhaust should toggle back to Exhaust1");
    }

    // Test 6: Hostile enemy animated frames
    {
        AnimationController anim;
        TEST_ASSERT(anim.currentEnemySprite(EntityType::PatrolWalker) == SpriteId::EnemyCrawler1,
            "PatrolWalker initial frame is Crawler1");
        TEST_ASSERT(anim.currentEnemySprite(EntityType::HomingFlyer) == SpriteId::EnemyBuzzer1,
            "HomingFlyer initial frame is Buzzer1");

        anim.update(DeltaTime { AnimationController::kEnemyFrameDuration });
        TEST_ASSERT(anim.currentEnemySprite(EntityType::PatrolWalker) == SpriteId::EnemyCrawler2,
            "PatrolWalker toggles to Crawler2");
        TEST_ASSERT(anim.currentEnemySprite(EntityType::HomingFlyer) == SpriteId::EnemyBuzzer2,
            "HomingFlyer toggles to Buzzer2");
    }

    // Test 7: Multiphase explosion progression
    {
        AnimationController anim;
        TEST_ASSERT(anim.currentExplosionSprite(0.10f) == SpriteId::ExplosionPhase1, "0.10 lifetime is Phase1");
        TEST_ASSERT(anim.currentExplosionSprite(0.35f) == SpriteId::ExplosionPhase2, "0.35 lifetime is Phase2");
        TEST_ASSERT(anim.currentExplosionSprite(0.65f) == SpriteId::ExplosionPhase3, "0.65 lifetime is Phase3");
        TEST_ASSERT(anim.currentExplosionSprite(0.90f) == SpriteId::ExplosionPhase4, "0.90 lifetime is Phase4");
    }

    // Test 8: Authentic EGA mecha sprite helpers and range invariants
    {
        for (std::uint8_t i { 0U }; i < 88U; ++i) {
            const SpriteId id { toEgaMechaSpriteId(i) };
            TEST_ASSERT(isEgaMechaSprite(id), "EGA sprite ID must be identified as EGA mecha");
            TEST_ASSERT_EQ(egaMechaFrameIndex(id), i, "Extracted frame index must match source");
        }
        TEST_ASSERT_FALSE(isEgaMechaSprite(SpriteId::PlayerRobotIdle), "Legacy ID should not be in EGA range");
        TEST_ASSERT_EQ(egaMechaFrameIndex(SpriteId::PlayerRobotIdle), 0U, "Legacy ID returns 0 frame index");
        TEST_ASSERT(isEgaMechaRobot(SpriteId::PlayerRobotIdle), "Legacy robot should be recognized as robot");
        TEST_ASSERT(isEgaMechaJet(SpriteId::PlayerJet), "Legacy jet should be recognized as jet");
        TEST_ASSERT(isEgaMechaJet(toEgaMechaSpriteId(60U)), "Frame 60 should be recognized as jet");
        TEST_ASSERT(isEgaMechaRobot(toEgaMechaSpriteId(10U)), "Frame 10 should be recognized as robot");
    }

    // Test 9: Authentic EGA mecha state mapping (Walk, Aim, Crouch, Jet, Morph, Shield)
    {
        AnimationController anim;
        const Velocity stationary { 0.0f, 0.0f };
        const Velocity walkRight { 100.0f, 0.0f };
        const Velocity walkLeft { -100.0f, 0.0f };

        // Idle Grounded Right (Frame 0) and Left (Frame 8)
        TEST_ASSERT(anim.currentEgaMechaSprite(TransformMode::Robot, FacingDirection::Right, false, false, false, false,
                        0.0f, stationary, 0.0f, false)
                == toEgaMechaSpriteId(0U),
            "Idle right mecha must be frame 0");
        TEST_ASSERT(anim.currentEgaMechaSprite(TransformMode::Robot, FacingDirection::Left, false, false, false, false,
                        0.0f, stationary, 0.0f, false)
                == toEgaMechaSpriteId(8U),
            "Idle left mecha must be frame 8");

        // 8-frame walking cycle right (0..7) and left (8..15)
        TEST_ASSERT(anim.currentEgaMechaSprite(TransformMode::Robot, FacingDirection::Right, true, false, false, false,
                        0.0f, walkRight, 0.0f, false)
                == toEgaMechaSpriteId(0U),
            "Walk step 0 right");
        anim.update(DeltaTime { AnimationController::kWalk8FrameDuration });
        TEST_ASSERT(anim.currentEgaMechaSprite(TransformMode::Robot, FacingDirection::Right, true, false, false, false,
                        0.0f, walkRight, 0.0f, false)
                == toEgaMechaSpriteId(1U),
            "Walk step 1 right");
        TEST_ASSERT(anim.currentEgaMechaSprite(TransformMode::Robot, FacingDirection::Left, true, false, false, false,
                        0.0f, walkLeft, 0.0f, false)
                == toEgaMechaSpriteId(9U),
            "Walk step 1 left");

        // Crouch (Frames 32..39)
        TEST_ASSERT(anim.currentEgaMechaSprite(TransformMode::Robot, FacingDirection::Right, false, false, true, false,
                        0.0f, stationary, 0.0f, false)
                == toEgaMechaSpriteId(32U),
            "Crouch stance frame");

        // Aiming Right (Frames 16..23) and Left (Frames 24..31)
        const SpriteId aimLevelRight { anim.currentEgaMechaSprite(
            TransformMode::Robot, FacingDirection::Right, false, false, false, true, 0.0f, stationary, 0.0f, false) };
        TEST_ASSERT(isEgaMechaSprite(aimLevelRight), "Aiming must return valid EGA sprite");
        const std::uint8_t aimIdxRight { egaMechaFrameIndex(aimLevelRight) };
        TEST_ASSERT(aimIdxRight >= 16U && aimIdxRight <= 23U, "Aim right index must be in 16..23");

        const SpriteId aimLevelLeft { anim.currentEgaMechaSprite(
            TransformMode::Robot, FacingDirection::Left, false, false, false, true, 0.0f, stationary, 0.0f, false) };
        const std::uint8_t aimIdxLeft { egaMechaFrameIndex(aimLevelLeft) };
        TEST_ASSERT(aimIdxLeft >= 24U && aimIdxLeft <= 31U, "Aim left index must be in 24..31");

        // Jet Flight 8-Headings (Frames 56..71)
        const Velocity jetEast { 200.0f, 0.0f };
        const SpriteId jetSprEast { anim.currentEgaMechaSprite(
            TransformMode::Jet, FacingDirection::Right, true, true, false, false, 0.0f, jetEast, 0.0f, false) };
        const std::uint8_t jetIdxEast { egaMechaFrameIndex(jetSprEast) };
        TEST_ASSERT(jetIdxEast >= 56U && jetIdxEast <= 71U, "Jet flight frame must be in 56..71");

        const Velocity jetNorth { 0.0f, -200.0f };
        const SpriteId jetSprNorth { anim.currentEgaMechaSprite(
            TransformMode::Jet, FacingDirection::Right, true, true, false, false, 0.0f, jetNorth, 0.0f, false) };
        TEST_ASSERT(jetSprNorth != jetSprEast, "Different velocity vectors must yield different jet heading frames");

        // Morphing progression (Frames 40..55)
        const SpriteId morphEarly { anim.currentEgaMechaSprite(TransformMode::TransformingToJet, FacingDirection::Right,
            false, false, false, false, 0.0f, stationary, 0.10f, false) };
        const SpriteId morphLate { anim.currentEgaMechaSprite(TransformMode::TransformingToJet, FacingDirection::Right,
            false, false, false, false, 0.0f, stationary, 0.90f, false) };
        TEST_ASSERT_EQ(egaMechaFrameIndex(morphEarly), 41U, "Early morph to jet frame");
        TEST_ASSERT_EQ(egaMechaFrameIndex(morphLate), 54U, "Late morph to jet frame");

        // Shield flash (Frames 80..87)
        anim.reset();
        const SpriteId shieldSpr { anim.currentEgaMechaSprite(
            TransformMode::Robot, FacingDirection::Right, false, false, false, false, 0.0f, stationary, 0.0f, true) };
        const std::uint8_t shieldIdx { egaMechaFrameIndex(shieldSpr) };
        TEST_ASSERT(shieldIdx >= 80U && shieldIdx <= 87U, "Active shield must trigger shield flash frames 80..87");
    }

    // Test 8: Authentic EGA mecha sprite helpers and range invariants
    {
        for (std::uint8_t i { 0U }; i < 88U; ++i) {
            const SpriteId id { toEgaMechaSpriteId(i) };
            TEST_ASSERT(isEgaMechaSprite(id), "EGA sprite ID must be identified as EGA mecha");
            TEST_ASSERT_EQ(egaMechaFrameIndex(id), i, "Extracted frame index must match source");
        }
        TEST_ASSERT_FALSE(isEgaMechaSprite(SpriteId::PlayerRobotIdle), "Legacy ID should not be in EGA range");
        TEST_ASSERT_EQ(egaMechaFrameIndex(SpriteId::PlayerRobotIdle), 0U, "Legacy ID returns 0 frame index");
        TEST_ASSERT(isEgaMechaRobot(SpriteId::PlayerRobotIdle), "Legacy robot should be recognized as robot");
        TEST_ASSERT(isEgaMechaJet(SpriteId::PlayerJet), "Legacy jet should be recognized as jet");
        TEST_ASSERT(isEgaMechaJet(toEgaMechaSpriteId(60U)), "Frame 60 should be recognized as jet");
        TEST_ASSERT(isEgaMechaRobot(toEgaMechaSpriteId(10U)), "Frame 10 should be recognized as robot");
    }

    // Test 9: Authentic EGA mecha state mapping (Walk, Aim, Crouch, Jet, Morph, Shield)
    {
        AnimationController anim;
        const Velocity stationary { 0.0f, 0.0f };
        const Velocity walkRight { 100.0f, 0.0f };
        const Velocity walkLeft { -100.0f, 0.0f };

        // Idle Grounded Right (Frame 0) and Left (Frame 8)
        TEST_ASSERT(anim.currentEgaMechaSprite(TransformMode::Robot, FacingDirection::Right, false, false, false, false,
                        0.0f, stationary, 0.0f, false)
                == toEgaMechaSpriteId(0U),
            "Idle right mecha must be frame 0");
        TEST_ASSERT(anim.currentEgaMechaSprite(TransformMode::Robot, FacingDirection::Left, false, false, false, false,
                        0.0f, stationary, 0.0f, false)
                == toEgaMechaSpriteId(8U),
            "Idle left mecha must be frame 8");

        // 8-frame walking cycle right (0..7) and left (8..15)
        TEST_ASSERT(anim.currentEgaMechaSprite(TransformMode::Robot, FacingDirection::Right, true, false, false, false,
                        0.0f, walkRight, 0.0f, false)
                == toEgaMechaSpriteId(0U),
            "Walk step 0 right");
        anim.update(DeltaTime { AnimationController::kWalk8FrameDuration });
        TEST_ASSERT(anim.currentEgaMechaSprite(TransformMode::Robot, FacingDirection::Right, true, false, false, false,
                        0.0f, walkRight, 0.0f, false)
                == toEgaMechaSpriteId(1U),
            "Walk step 1 right");
        TEST_ASSERT(anim.currentEgaMechaSprite(TransformMode::Robot, FacingDirection::Left, true, false, false, false,
                        0.0f, walkLeft, 0.0f, false)
                == toEgaMechaSpriteId(9U),
            "Walk step 1 left");

        // Crouch (Frames 32..39)
        TEST_ASSERT(anim.currentEgaMechaSprite(TransformMode::Robot, FacingDirection::Right, false, false, true, false,
                        0.0f, stationary, 0.0f, false)
                == toEgaMechaSpriteId(32U),
            "Crouch stance frame");

        // Aiming Right (Frames 16..23) and Left (Frames 24..31)
        const SpriteId aimLevelRight { anim.currentEgaMechaSprite(
            TransformMode::Robot, FacingDirection::Right, false, false, false, true, 0.0f, stationary, 0.0f, false) };
        TEST_ASSERT(isEgaMechaSprite(aimLevelRight), "Aiming must return valid EGA sprite");
        const std::uint8_t aimIdxRight { egaMechaFrameIndex(aimLevelRight) };
        TEST_ASSERT(aimIdxRight >= 16U && aimIdxRight <= 23U, "Aim right index must be in 16..23");

        const SpriteId aimLevelLeft { anim.currentEgaMechaSprite(
            TransformMode::Robot, FacingDirection::Left, false, false, false, true, 0.0f, stationary, 0.0f, false) };
        const std::uint8_t aimIdxLeft { egaMechaFrameIndex(aimLevelLeft) };
        TEST_ASSERT(aimIdxLeft >= 24U && aimIdxLeft <= 31U, "Aim left index must be in 24..31");

        // Jet Flight 8-Headings (Frames 56..71)
        const Velocity jetEast { 200.0f, 0.0f };
        const SpriteId jetSprEast { anim.currentEgaMechaSprite(
            TransformMode::Jet, FacingDirection::Right, true, true, false, false, 0.0f, jetEast, 0.0f, false) };
        const std::uint8_t jetIdxEast { egaMechaFrameIndex(jetSprEast) };
        TEST_ASSERT(jetIdxEast >= 56U && jetIdxEast <= 71U, "Jet flight frame must be in 56..71");

        const Velocity jetNorth { 0.0f, -200.0f };
        const SpriteId jetSprNorth { anim.currentEgaMechaSprite(
            TransformMode::Jet, FacingDirection::Right, true, true, false, false, 0.0f, jetNorth, 0.0f, false) };
        TEST_ASSERT(jetSprNorth != jetSprEast, "Different velocity vectors must yield different jet heading frames");

        // Morphing progression (Frames 40..55)
        const SpriteId morphEarly { anim.currentEgaMechaSprite(TransformMode::TransformingToJet, FacingDirection::Right,
            false, false, false, false, 0.0f, stationary, 0.10f, false) };
        const SpriteId morphLate { anim.currentEgaMechaSprite(TransformMode::TransformingToJet, FacingDirection::Right,
            false, false, false, false, 0.0f, stationary, 0.90f, false) };
        TEST_ASSERT_EQ(egaMechaFrameIndex(morphEarly), 41U, "Early morph to jet frame");
        TEST_ASSERT_EQ(egaMechaFrameIndex(morphLate), 54U, "Late morph to jet frame");

        // Shield flash (Frames 80..87)
        anim.reset();
        const SpriteId shieldSpr { anim.currentEgaMechaSprite(
            TransformMode::Robot, FacingDirection::Right, false, false, false, false, 0.0f, stationary, 0.0f, true) };
        const std::uint8_t shieldIdx { egaMechaFrameIndex(shieldSpr) };
        TEST_ASSERT(shieldIdx >= 80U && shieldIdx <= 87U, "Active shield must trigger shield flash frames 80..87");
    }
}

} // namespace ShooterCore::Test
