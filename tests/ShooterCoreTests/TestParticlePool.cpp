#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include <array>

#include "ParticlePool.h"
#include "Types.h"

namespace ShooterCore::Test {

static void testParticlePoolInit() noexcept
{
    ParticlePool pool { };
    TEST_ASSERT(pool.activeCount() == 0U, "ParticlePool activeCount must start at 0");

    for (std::size_t i { 0U }; i < ParticlePool::kCapacity; ++i) {
        TEST_ASSERT(!pool.particles()[i].active, "All particles must initially be inactive");
    }
}

static void testParticlePoolEmitSparks() noexcept
{
    ParticlePool pool { };
    const WorldPos origin { 100.0f, 150.0f };

    pool.emitSparks(origin, 0.0f, 8U);
    TEST_ASSERT(pool.activeCount() == 8U, "Emitting 8 sparks must increase activeCount to 8");

    std::size_t activeCount { 0U };
    for (std::size_t i { 0U }; i < ParticlePool::kCapacity; ++i) {
        const Particle& p { pool.particles()[i] };
        if (p.active) {
            ++activeCount;
            TEST_ASSERT(p.type == ParticleType::ImpactSpark, "Particle type must be ImpactSpark");
            TEST_ASSERT(p.pos.x == origin.x && p.pos.y == origin.y, "Spark pos must match origin");
            TEST_ASSERT(p.lifetime > 0.0f, "Spark lifetime must be positive");
            TEST_ASSERT(p.color.a == 255U, "Initial alpha must be fully opaque");
        }
    }
    TEST_ASSERT(activeCount == 8U, "Active slot count must match 8");
}

static void testParticlePoolPhysicsAndDecay() noexcept
{
    ParticlePool pool { };
    const WorldPos origin { 200.0f, 100.0f };

    pool.emitDebris(origin, 4U);
    TEST_ASSERT(pool.activeCount() == 4U, "Emitting 4 debris particles must update activeCount to 4");

    // Advance simulation by 0.1 seconds
    const DeltaTime dt { 0.1f };
    pool.update(dt);

    for (std::size_t i { 0U }; i < ParticlePool::kCapacity; ++i) {
        const Particle& p { pool.particles()[i] };
        if (p.active) {
            TEST_ASSERT(p.pos.x != origin.x || p.pos.y != origin.y, "Debris pos must move with velocity");
            TEST_ASSERT(p.type == ParticleType::WallDebris, "Particle type must be WallDebris");
        }
    }

    // Advance simulation past all particle lifetimes (e.g. 1.5 seconds)
    const DeltaTime longDt { 1.5f };
    pool.update(longDt);

    TEST_ASSERT(pool.activeCount() == 0U, "All particles must expire and activeCount return to 0");
}

static void testParticlePoolSaturation() noexcept
{
    ParticlePool pool { };
    const WorldPos origin { 50.0f, 50.0f };

    // Emit 150 particles (exceeding 128 slot capacity)
    for (std::size_t i { 0U }; i < 15U; ++i) {
        pool.emitSparks(origin, 0.0f, 10U);
    }

    TEST_ASSERT(pool.activeCount() <= ParticlePool::kCapacity, "Active count must never exceed capacity");

    // Clear pool
    pool.clear();
    TEST_ASSERT(pool.activeCount() == 0U, "Pool clear must reset activeCount to 0");
}

void runParticlePoolTests() noexcept
{
    testParticlePoolInit();
    testParticlePoolEmitSparks();
    testParticlePoolPhysicsAndDecay();
    testParticlePoolSaturation();
}

} // namespace ShooterCore::Test
