#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include "Hitbox.h"
#include "Types.h"

namespace ShooterCore::Test {

void runHitboxTests() noexcept
{
    // 1. Dimensions test
    const WorldPos origin { 10.0f, 20.0f };
    const Hitbox boxA { Hitbox::fromDims(origin, 16.0f, 32.0f) };

    TEST_ASSERT(boxA.width() == 16.0f, "Hitbox width must equal 16");
    TEST_ASSERT(boxA.height() == 32.0f, "Hitbox height must equal 32");
    TEST_ASSERT(boxA.min().x == 10.0f && boxA.min().y == 20.0f, "Min pos must match origin");
    TEST_ASSERT(boxA.max().x == 26.0f && boxA.max().y == 52.0f, "Max pos must match origin + dims");

    // 2. Point containment
    TEST_ASSERT(boxA.contains(WorldPos { 10.0f, 20.0f }), "Hitbox must contain its top-left border");
    TEST_ASSERT(boxA.contains(WorldPos { 26.0f, 52.0f }), "Hitbox must contain its bottom-right border");
    TEST_ASSERT(boxA.contains(WorldPos { 15.0f, 30.0f }), "Hitbox must contain an interior point");
    TEST_ASSERT(!boxA.contains(WorldPos { 9.9f, 20.0f }), "Hitbox must not contain point to the left");
    TEST_ASSERT(!boxA.contains(WorldPos { 26.1f, 52.0f }), "Hitbox must not contain point to the right");

    // 3. Overlap / Intersection
    const Hitbox boxOverlap { Hitbox::fromDims(WorldPos { 20.0f, 30.0f }, 16.0f, 16.0f) };
    TEST_ASSERT(boxA.intersects(boxOverlap), "Overlapping hitboxes must intersect");
    TEST_ASSERT(boxOverlap.intersects(boxA), "Intersection must be symmetric");

    // 4. Disjoint boxes
    const Hitbox boxDisjoint { Hitbox::fromDims(WorldPos { 100.0f, 100.0f }, 16.0f, 16.0f) };
    TEST_ASSERT(!boxA.intersects(boxDisjoint), "Disjoint hitboxes must not intersect");

    // 5. Touching edges (no interior overlap)
    const Hitbox boxTouchingRight { Hitbox::fromDims(WorldPos { 26.0f, 20.0f }, 16.0f, 32.0f) };
    TEST_ASSERT(!boxA.intersects(boxTouchingRight), "Boxes that only touch boundaries must not intersect");
}

} // namespace ShooterCore::Test
