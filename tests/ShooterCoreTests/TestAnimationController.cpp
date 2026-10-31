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
}

} // namespace ShooterCore::Test
