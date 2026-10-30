#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include "AudioCommandQueue.h"
#include "RenderCommandQueue.h"
#include "Types.h"

namespace ShooterCore::Test {

void runCommandQueueTests() noexcept
{
    // Test 1: RenderCommandQueue defaults and pushing
    {
        RenderCommandQueue queue { };
        TEST_ASSERT(queue.isEmpty(), "Fresh RenderCommandQueue must be empty");
        TEST_ASSERT(queue.count() == 0U, "Count must be zero initially");
        TEST_ASSERT(!queue.isFull(), "Fresh RenderCommandQueue must not be full");

        const bool pushedSprite { queue.pushSprite(
            SpriteId::PlayerRobotWalk1, WorldPos { 10.0f, 20.0f }, FacingDirection::Left, 1U) };
        TEST_ASSERT(pushedSprite, "pushSprite must succeed");
        TEST_ASSERT(queue.count() == 1U, "Count must be 1 after pushSprite");

        const RenderCommand& cmd0 { queue.get(0U) };
        TEST_ASSERT(cmd0.type == RenderCommandType::DrawSprite, "cmd0 type must be DrawSprite");
        TEST_ASSERT(cmd0.sprite == SpriteId::PlayerRobotWalk1, "cmd0 sprite must match");
        TEST_ASSERT(cmd0.pos.x == 10.0f && cmd0.pos.y == 20.0f, "cmd0 pos must match");
        TEST_ASSERT(cmd0.facing == FacingDirection::Left, "cmd0 facing must match");
        TEST_ASSERT(cmd0.layer == 1U, "cmd0 layer must match");

        const bool pushedTile { queue.pushTile(TileType::SolidWall, WorldPos { 32.0f, 48.0f }, 0U) };
        TEST_ASSERT(pushedTile, "pushTile must succeed");
        const RenderCommand& cmd1 { queue.get(1U) };
        TEST_ASSERT(cmd1.type == RenderCommandType::DrawTile, "cmd1 type must be DrawTile");
        TEST_ASSERT(cmd1.tileType == TileType::SolidWall, "cmd1 tileType must match");

        const bool pushedBeam { queue.pushBeam(
            WorldPos { 10.0f, 10.0f }, WorldPos { 100.0f, 10.0f }, ColorRgba { 255U, 0U, 0U, 255U }) };
        TEST_ASSERT(pushedBeam, "pushBeam must succeed");
        const RenderCommand& cmd2 { queue.get(2U) };
        TEST_ASSERT(cmd2.type == RenderCommandType::DrawBeam, "cmd2 type must be DrawBeam");
        TEST_ASSERT(cmd2.color.r == 255U, "cmd2 color must match");

        const bool pushedBar { queue.pushBar(
            WorldPos { 0.0f, 0.0f }, 100.0f, 8.0f, 0.75f, ColorRgba { 0U, 255U, 0U, 255U }) };
        TEST_ASSERT(pushedBar, "pushBar must succeed");
        const RenderCommand& cmd3 { queue.get(3U) };
        TEST_ASSERT(cmd3.type == RenderCommandType::DrawBar, "cmd3 type must be DrawBar");
        TEST_ASSERT(cmd3.fillPct == 0.75f, "cmd3 fillPct must match");

        queue.clear();
        TEST_ASSERT(queue.isEmpty(), "clear() must empty RenderCommandQueue");
        TEST_ASSERT(queue.count() == 0U, "count must be 0 after clear");
    }

    // Test 2: RenderCommandQueue capacity limit
    {
        RenderCommandQueue queue { };
        for (std::size_t i { 0U }; i < RenderCommandQueue::kCapacity; ++i) {
            const bool ok { queue.pushTile(TileType::Empty, WorldPos { static_cast<float>(i), 0.0f }, 0U) };
            TEST_ASSERT(ok, "Fill loop push must succeed");
        }
        TEST_ASSERT(queue.isFull(), "Queue must be full at kCapacity");
        TEST_ASSERT(queue.count() == RenderCommandQueue::kCapacity, "Count must equal kCapacity");

        const bool overflowOk { queue.pushTile(TileType::SolidWall, WorldPos { 0.0f, 0.0f }, 0U) };
        TEST_ASSERT(!overflowOk, "Overflow push must return false");
        TEST_ASSERT(queue.count() == RenderCommandQueue::kCapacity, "Count must not increase beyond kCapacity");
    }

    // Test 3: AudioCommandQueue
    {
        AudioCommandQueue aqueue { };
        TEST_ASSERT(aqueue.isEmpty(), "Fresh AudioCommandQueue must be empty");

        const bool pushedSfx { aqueue.pushSound(SoundEffectId::LaserFire, 0.8f) };
        TEST_ASSERT(pushedSfx, "pushSound must succeed");
        TEST_ASSERT(aqueue.count() == 1U, "Count must be 1 after pushSound");

        const AudioCommand& acmd0 { aqueue.get(0U) };
        TEST_ASSERT(acmd0.type == AudioCommandType::PlaySound, "Type must be PlaySound");
        TEST_ASSERT(acmd0.soundId == SoundEffectId::LaserFire, "soundId must be LaserFire");
        TEST_ASSERT(acmd0.volume == 0.8f, "Volume must match");

        const bool pushedMusic { aqueue.pushMusic(MusicTrackId::Level1Cavern, true) };
        TEST_ASSERT(pushedMusic, "pushMusic must succeed");
        const AudioCommand& acmd1 { aqueue.get(1U) };
        TEST_ASSERT(acmd1.type == AudioCommandType::PlayMusic, "Type must be PlayMusic");
        TEST_ASSERT(acmd1.musicId == MusicTrackId::Level1Cavern, "musicId must match");
        TEST_ASSERT(acmd1.loop, "loop must be true");

        const bool pushedStop { aqueue.stopMusic() };
        TEST_ASSERT(pushedStop, "stopMusic must succeed");
        const AudioCommand& acmd2 { aqueue.get(2U) };
        TEST_ASSERT(acmd2.type == AudioCommandType::StopMusic, "Type must be StopMusic");

        aqueue.clear();
        TEST_ASSERT(aqueue.isEmpty(), "clear() must empty AudioCommandQueue");
    }
}

} // namespace ShooterCore::Test
