#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include <array>
#include <iostream>

#include "EngineInput.h"
#include "EntityPool.h"
#include "GameEngine.h"
#include "Hitbox.h"
#include "TileMap.h"
#include "Types.h"

namespace ShooterCore::Test {

namespace {

    class StateHasher final {
    public:
        constexpr StateHasher() noexcept
            : hash_ { 0xCBF29CE484222325ULL }
        {
        }

        void update(const void* data, std::size_t size) noexcept
        {
            const auto* bytes = static_cast<const std::uint8_t*>(data);
            for (std::size_t i { 0U }; i < size; ++i) {
                hash_ ^= static_cast<std::uint64_t>(bytes[i]);
                hash_ *= 0x100000001B3ULL;
            }
        }

        template <typename T> void updatePod(const T& val) noexcept
        {
            update(&val, sizeof(T));
        }

        [[nodiscard]] constexpr std::uint64_t getHash() const noexcept
        {
            return hash_;
        }

    private:
        std::uint64_t hash_ { 0xCBF29CE484222325ULL };
    };

    [[nodiscard]] std::uint64_t hashEngineState(const GameEngine& engine) noexcept
    {
        StateHasher hasher { };

        const WorldPos pPos { engine.getPlayerPos() };
        hasher.updatePod(pPos.x);
        hasher.updatePod(pPos.y);

        const Velocity pVel { engine.getPlayerVel() };
        hasher.updatePod(pVel.dx);
        hasher.updatePod(pVel.dy);

        const Hitbox pBox { engine.getPlayerBox() };
        hasher.updatePod(pBox.min().x);
        hasher.updatePod(pBox.min().y);
        hasher.updatePod(pBox.max().x);
        hasher.updatePod(pBox.max().y);

        const auto pMode = static_cast<std::uint8_t>(engine.getPlayerMode());
        hasher.updatePod(pMode);

        const std::uint16_t energyVal = engine.getPlayerEnergy().value();
        hasher.updatePod(energyVal);

        const auto facingVal = static_cast<std::uint8_t>(engine.getPlayerFacing());
        hasher.updatePod(facingVal);

        const std::uint8_t aliveVal = engine.isPlayerAlive() ? 1U : 0U;
        hasher.updatePod(aliveVal);

        const CameraViewport cam { engine.getViewport() };
        hasher.updatePod(cam.pos.x);
        hasher.updatePod(cam.pos.y);
        hasher.updatePod(cam.width);
        hasher.updatePod(cam.height);

        const EntityPool& pool { engine.getEntityPool() };
        const std::size_t activeCount = pool.count();
        hasher.updatePod(activeCount);

        for (std::size_t i { 0U }; i < EntityPool::kCapacity; ++i) {
            const EntitySlot& slot = pool.slot(i);
            if (slot.active) {
                const auto entType = static_cast<std::uint8_t>(slot.data.type);
                hasher.updatePod(entType);
                hasher.updatePod(slot.data.pos.x);
                hasher.updatePod(slot.data.pos.y);
                hasher.updatePod(slot.data.vel.dx);
                hasher.updatePod(slot.data.vel.dy);
                hasher.updatePod(slot.data.health);
            }
        }

        return hasher.getHash();
    }

    [[nodiscard]] EngineInput generateScriptedInput(std::uint32_t frame) noexcept
    {
        EngineInput in { };

        // Scripted cyclical behavior mimicking player gameplay
        const std::uint32_t cycle = frame % 1200U;

        if (cycle < 200U) {
            // Run right
            in.setButton(EngineInput::kRight);
            if ((frame % 45U) == 0U) {
                in.setButton(EngineInput::kJump);
            }
        } else if (cycle < 220U) {
            // Transform to Jet
            in.setButton(EngineInput::kTransform);
        } else if (cycle < 700U) {
            // Fly Jet: thrust right and up/down
            in.setButton(EngineInput::kRight);
            if ((frame % 60U) < 30U) {
                in.setButton(EngineInput::kUp);
            } else {
                in.setButton(EngineInput::kDown);
            }
            // Fire weapon periodically
            if ((frame % 10U) < 7U) {
                in.setButton(EngineInput::kFire);
            }
        } else if (cycle < 720U) {
            // Transform to Robot in open chamber
            in.setButton(EngineInput::kTransform);
        } else if (cycle < 1000U) {
            // Walk left, fire shield and laser
            in.setButton(EngineInput::kLeft);
            if ((frame % 20U) < 10U) {
                in.setButton(EngineInput::kFire);
            }
            if ((frame % 50U) == 0U) {
                in.setButton(EngineInput::kShield);
            }
        } else {
            // Idle / recover
        }

        return in;
    }

    void setupTestLevel(TileMap& map) noexcept
    {
        // Floor on row 18
        for (std::size_t col { 0U }; col < 40U; ++col) {
            map.setTile(TileCoord { static_cast<std::uint32_t>(col), 18U }, TileType::SolidWall);
            map.setTile(TileCoord { static_cast<std::uint32_t>(col), 0U }, TileType::SolidWall);
        }

        // Outer boundary walls
        for (std::size_t row { 0U }; row < 20U; ++row) {
            map.setTile(TileCoord { 0U, static_cast<std::uint32_t>(row) }, TileType::SolidWall);
            map.setTile(TileCoord { 39U, static_cast<std::uint32_t>(row) }, TileType::SolidWall);
        }

        // Interior tunnels and destructible barriers
        for (std::size_t col { 10U }; col < 25U; ++col) {
            map.setTile(TileCoord { static_cast<std::uint32_t>(col), 8U }, TileType::SolidWall);
            map.setTile(TileCoord { static_cast<std::uint32_t>(col), 10U }, TileType::SolidWall);
        }

        // Destructible block barrier at row 9, col 15-17
        map.setTile(TileCoord { 15U, 9U }, TileType::DestructibleWall);
        map.setTile(TileCoord { 16U, 9U }, TileType::DestructibleWall);
        map.setTile(TileCoord { 17U, 9U }, TileType::DestructibleWall);
    }

} // namespace

void runDeterminismSoakTests() noexcept
{
    ScopedTimer timer { "runDeterminismSoakTests (10,000 frames)" };

    constexpr std::uint32_t kTotalFrames { 10000U };
    constexpr std::uint32_t kMilestoneStep { 1000U };
    constexpr std::size_t kMilestones { kTotalFrames / kMilestoneStep };

    std::array<std::uint64_t, kMilestones> hashesRun1 { };
    std::array<std::uint64_t, kMilestones> hashesRun2 { };

    const DeltaTime dt { 0.0166667f }; // 60 Hz fixed timestep

    // Execution Pass 1
    {
        TileMap map { 40U, 20U };
        setupTestLevel(map);

        GameEngine engine { };
        engine.initLevel(map, WorldPos { 32.0f, 64.0f });

        // Spawn some enemies into the pool
        EntityPool& pool = engine.getEntityPool();
        pool.spawn(EntityType::PatrolWalker, WorldPos { 160.0f, 270.0f },
            Hitbox { WorldPos { 160.0f, 270.0f }, WorldPos { 176.0f, 286.0f } }, 5U);
        pool.spawn(EntityType::HomingFlyer, WorldPos { 200.0f, 100.0f },
            Hitbox { WorldPos { 200.0f, 100.0f }, WorldPos { 216.0f, 116.0f } }, 10U);

        for (std::uint32_t frame { 1U }; frame <= kTotalFrames; ++frame) {
            const EngineInput in { generateScriptedInput(frame) };
            engine.step(dt, in);

            if ((frame % kMilestoneStep) == 0U) {
                const std::size_t idx = static_cast<std::size_t>((frame / kMilestoneStep) - 1U);
                hashesRun1[idx] = hashEngineState(engine);
            }
        }
    }

    // Execution Pass 2 (Independent fresh run with identical inputs)
    {
        TileMap map { 40U, 20U };
        setupTestLevel(map);

        GameEngine engine { };
        engine.initLevel(map, WorldPos { 32.0f, 64.0f });

        EntityPool& pool = engine.getEntityPool();
        pool.spawn(EntityType::PatrolWalker, WorldPos { 160.0f, 270.0f },
            Hitbox { WorldPos { 160.0f, 270.0f }, WorldPos { 176.0f, 286.0f } }, 5U);
        pool.spawn(EntityType::HomingFlyer, WorldPos { 200.0f, 100.0f },
            Hitbox { WorldPos { 200.0f, 100.0f }, WorldPos { 216.0f, 116.0f } }, 10U);

        for (std::uint32_t frame { 1U }; frame <= kTotalFrames; ++frame) {
            const EngineInput in { generateScriptedInput(frame) };
            engine.step(dt, in);

            if ((frame % kMilestoneStep) == 0U) {
                const std::size_t idx = static_cast<std::size_t>((frame / kMilestoneStep) - 1U);
                hashesRun2[idx] = hashEngineState(engine);

                // Immediate assertion check between Pass 1 and Pass 2
                TEST_ASSERT_EQ(
                    hashesRun2[idx], hashesRun1[idx], "Simulation state hash must be 100% deterministic at milestone");
            }
        }
    }

    // Verify all milestones are non-zero and match
    for (std::size_t i { 0U }; i < kMilestones; ++i) {
        TEST_ASSERT(hashesRun1[i] != 0ULL, "Milestone hash must not be zero");
        TEST_ASSERT_EQ(hashesRun1[i], hashesRun2[i], "State hashes across runs must be identical");
    }
}

} // namespace ShooterCore::Test
