#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include "AudioCommandQueue.h"
#include "BossController.h"
#include "EntityPool.h"
#include "Hitbox.h"
#include "ParticlePool.h"
#include "RenderCommandQueue.h"
#include "Types.h"

namespace ShooterCore::Test {

void runBossControllerTests() noexcept
{
    BossController boss { };
    AudioCommandQueue audioQueue { };
    EntityPool entities { };
    ParticlePool particles { };
    RenderCommandQueue renderQueue { };

    // 1. Initial State
    TEST_ASSERT(boss.phase() == BossPhase::Dormant, "Boss starts dormant");
    TEST_ASSERT(!boss.isDefeated(), "Boss is not defeated initially");

    // 2. Spawn Guardian in Sector Delta
    const WorldPos spawnPos { 3400.0f, 1500.0f };
    boss.spawn(spawnPos);
    TEST_ASSERT(boss.phase() == BossPhase::ShieldedBarrage, "Boss starts in ShieldedBarrage phase");
    TEST_ASSERT_NEAR(boss.corePosition().x, 3400.0f, 0.01f, "Core position X");
    TEST_ASSERT_NEAR(boss.corePosition().y, 1500.0f, 0.01f, "Core position Y");

    // Verify update cycle
    boss.update(DeltaTime { 0.016f }, WorldPos { 3200.0f, 1500.0f }, entities, particles, audioQueue);

    // 3. Core Invulnerability with Intact Armor
    const Hitbox coreAttackBox { { 3390.0f, 1490.0f }, { 3410.0f, 1510.0f } };
    TEST_ASSERT(boss.applyDamage(coreAttackBox, 50U, audioQueue), "Attack overlapping core must be processed");
    TEST_ASSERT(boss.getSegment(0).health.current() == 200U, "Core HP must be untouched while armor is intact");

    // 4. Destroy Left Armor Plate (Index 1, offset -24)
    const Hitbox leftArmorBox { { 3365.0f, 1490.0f }, { 3385.0f, 1510.0f } };
    boss.applyDamage(leftArmorBox, 150U, audioQueue);
    TEST_ASSERT(boss.getSegment(1).health.isDestroyed(), "Left armor plate must be destroyed");
    TEST_ASSERT(boss.phase() == BossPhase::ShieldedBarrage, "Still in ShieldedBarrage: right armor plate intact");

    // Core still protected
    boss.applyDamage(coreAttackBox, 50U, audioQueue);
    TEST_ASSERT(boss.getSegment(0).health.current() == 200U, "Core still protected by right armor plate");

    // 5. Destroy Right Armor Plate (Index 2, offset +24)
    const Hitbox rightArmorBox { { 3415.0f, 1490.0f }, { 3435.0f, 1510.0f } };
    boss.applyDamage(rightArmorBox, 150U, audioQueue);
    TEST_ASSERT(boss.getSegment(2).health.isDestroyed(), "Right armor plate must be destroyed");
    TEST_ASSERT(boss.phase() == BossPhase::CoreExposure, "Both armor plates destroyed: phase must be CoreExposure!");

    // 6. Direct Core Damage & Transition to Enraged
    boss.applyDamage(coreAttackBox, 100U, audioQueue);
    TEST_ASSERT(boss.getSegment(0).health.current() == 100U, "Core takes direct damage: HP must be 100");
    TEST_ASSERT(boss.phase() == BossPhase::CoreExposure, "Core at 100 HP remains CoreExposure");

    boss.applyDamage(coreAttackBox, 50U, audioQueue);
    TEST_ASSERT(boss.getSegment(0).health.current() == 50U, "Core HP drops to 50");
    TEST_ASSERT(boss.phase() == BossPhase::EnragedDesperation, "Core HP <= 60 must trigger EnragedDesperation!");

    // 7. Fatal Damage to Core
    boss.applyDamage(coreAttackBox, 60U, audioQueue);
    TEST_ASSERT(boss.getSegment(0).health.isDestroyed(), "Core must be destroyed");
    TEST_ASSERT(boss.phase() == BossPhase::Defeated, "Phase must be Defeated");
    TEST_ASSERT(boss.isDefeated(), "isDefeated() must return true");

    // 8. Render Command Emission
    boss.emitRender(renderQueue);
    TEST_ASSERT(renderQueue.count() == 0U, "Defeated boss must not emit render commands");
}

} // namespace ShooterCore::Test
