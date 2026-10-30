#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include "CollisionSolver.h"
#include "Hitbox.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore::Test {

void runCollisionTests() noexcept
{
    // Test 1: Continuous Collision Detection - High Velocity Tunneling Prevention
    // Setup a 10x10 map with a solid wall at col 2 (x in [32, 48])
    TileMap map { 10U, 10U, 16.0f };
    for (std::uint32_t r { 0U }; r < 10U; ++r) {
        map.setTile(TileCoord { 2U, r }, TileType::SolidWall);
    }

    // Box at x = 0.0, width = 16, height = 16.
    // Box bounds: [0, 16]. Wall begins at x = 32. Distance = 16px.
    // Velocity dx = 1000 px/s, dt = 0.05s -> displacement = 50px!
    // With discrete physics, box jumps to x = 50, bypassing wall completely!
    const Hitbox bulletBox { Hitbox::fromDims(WorldPos { 0.0f, 16.0f }, 16.0f, 16.0f) };
    const Velocity highVel { 1000.0f, 0.0f };
    const float dt { 0.05f };

    const SweepHit sweepHit { CollisionSolver::sweepMap(bulletBox, highVel, dt, map) };
    TEST_ASSERT(sweepHit.collided, "High-velocity bullet must not tunnel through 1-tile wall");
    TEST_ASSERT(sweepHit.toi > 0.0f && sweepHit.toi < 1.0f, "TOI must be in interval (0.0, 1.0)");
    // Expected TOI: distance = 16px, displacement = 50px -> toi = 16 / 50 = 0.32
    TEST_ASSERT(sweepHit.toi >= 0.31f && sweepHit.toi <= 0.33f, "TOI must accurately reflect 16/50 ratio");
    TEST_ASSERT(sweepHit.normal == ContactNormal::Left, "Impacting right-side wall normal must point Left");
    TEST_ASSERT(sweepHit.hitTile.col == 2U && sweepHit.hitTile.row == 1U, "Hit tile must be (2, 1)");

    // Resolved move should halt box just before wall (x ≈ 16.0)
    const WorldPos resolvedBulletPos { CollisionSolver::resolveMove(bulletBox, highVel, dt, map) };
    TEST_ASSERT(resolvedBulletPos.x >= 15.9f && resolvedBulletPos.x <= 16.05f, "Bullet must halt at wall boundary");

    // Test 2: Contact Normal Directions for all 4 Cardinal Approaches
    const Hitbox centerBox { Hitbox::fromDims(WorldPos { 32.0f, 32.0f }, 16.0f, 16.0f) };

    // Moving left into wall at x = 16
    const Hitbox leftWall { Hitbox::fromDims(WorldPos { 0.0f, 32.0f }, 16.0f, 16.0f) };
    const SweepHit hitLeft { CollisionSolver::sweepBox(centerBox, Velocity { -100.0f, 0.0f }, 1.0f, leftWall) };
    TEST_ASSERT(hitLeft.collided, "Must collide moving left");
    TEST_ASSERT(hitLeft.normal == ContactNormal::Right, "Moving left into wall normal must point Right");

    // Moving down into floor at y = 48
    const Hitbox floorBox { Hitbox::fromDims(WorldPos { 32.0f, 64.0f }, 16.0f, 16.0f) };
    const SweepHit hitDown { CollisionSolver::sweepBox(centerBox, Velocity { 0.0f, 100.0f }, 1.0f, floorBox) };
    TEST_ASSERT(hitDown.collided, "Must collide falling down");
    TEST_ASSERT(hitDown.normal == ContactNormal::Up, "Falling down into floor normal must point Up");

    // Moving up into ceiling at y = 0
    const Hitbox ceilBox { Hitbox::fromDims(WorldPos { 32.0f, 0.0f }, 16.0f, 16.0f) };
    const SweepHit hitUp { CollisionSolver::sweepBox(centerBox, Velocity { 0.0f, -100.0f }, 1.0f, ceilBox) };
    TEST_ASSERT(hitUp.collided, "Must collide jumping up");
    TEST_ASSERT(hitUp.normal == ContactNormal::Down, "Jumping up into ceiling normal must point Down");

    // Test 3: Corner Sliding along Corridor Floor
    // Floor at row 3 (y = 48.0f).
    // Box at (16.0f, 32.0f), moving diagonally down-right: vx = 100, vy = 100, dt = 0.5s.
    TileMap slideMap { 10U, 10U, 16.0f };
    for (std::uint32_t c { 0U }; c < 10U; ++c) {
        slideMap.setTile(TileCoord { c, 3U }, TileType::SolidWall); // Floor
    }

    const Hitbox slidingPlayer { Hitbox::fromDims(WorldPos { 16.0f, 32.0f }, 16.0f, 16.0f) };
    const Velocity diagVel { 100.0f, 100.0f };
    const WorldPos slideResult { CollisionSolver::resolveMove(slidingPlayer, diagVel, 0.5f, slideMap) };

    // Player should advance horizontally by vx * dt = 50px -> x ≈ 16 + 50 = 66
    // Vertical motion should be arrested at y = 32 (touching floor at y = 48)
    TEST_ASSERT(
        slideResult.x >= 65.0f && slideResult.x <= 67.0f, "Horizontal sliding must preserve horizontal progress");
    TEST_ASSERT(slideResult.y >= 31.9f && slideResult.y <= 32.1f, "Vertical sliding must rest on floor");

    // Test 4: Moving away from adjacent wall must NOT register collision
    // Box at x = 16 touching wall at x = [0, 16]. Moving right (away from wall).
    const Hitbox adjacentBox { Hitbox::fromDims(WorldPos { 16.0f, 32.0f }, 16.0f, 16.0f) };
    const SweepHit moveAwayHit { CollisionSolver::sweepBox(adjacentBox, Velocity { 100.0f, 0.0f }, 1.0f, leftWall) };
    TEST_ASSERT(!moveAwayHit.collided, "Moving away from touching wall must not collide");

    // Test 5: Raycast Line of Sight
    TileMap rayMap { 10U, 10U, 16.0f };
    rayMap.setTile(TileCoord { 5U, 5U }, TileType::SolidWall);

    WorldPos hitPoint { 0.0f, 0.0f };
    // Clear path from (0, 0) to (40, 40)
    const bool clearPath { CollisionSolver::raycast(
        WorldPos { 0.0f, 0.0f }, WorldPos { 40.0f, 40.0f }, rayMap, hitPoint) };
    TEST_ASSERT(!clearPath, "Ray through open space must not hit");

    // Path intersecting tile (5, 5) which spans [80, 96] x [80, 96]
    const bool blockedPath { CollisionSolver::raycast(
        WorldPos { 0.0f, 88.0f }, WorldPos { 120.0f, 88.0f }, rayMap, hitPoint) };
    TEST_ASSERT(blockedPath, "Ray directed through wall must hit");
    TEST_ASSERT(hitPoint.x >= 79.9f && hitPoint.x <= 80.5f, "Ray hit point must land on wall front edge");
}

} // namespace ShooterCore::Test
