#include "TestHarness.h"

#include <cstdint>

#include "RenderCommandQueue.h"
#include "SdlRenderer.h"
#include "SdlWindow.h"
#include "Types.h"

namespace SdlAdapter::Test {

void runSdlRendererTests() noexcept
{
    // Test 1: Null renderer safety
    {
        SdlRenderer renderer;
        TEST_ASSERT_TRUE(renderer.getRenderer() == nullptr, "Renderer handle should default to null");

        // Methods should safely return when renderer is null
        ShooterCore::RenderCommandQueue queue;
        renderer.renderFrame(queue);
        renderer.clearScreen();
        renderer.present();
    }

    // Test 2: Ingestion of commands under headless dummy driver
    {
        SdlWindow win;
        WindowConfig config { };
        config.windowWidth = 320;
        config.windowHeight = 240;
        config.logicalWidth = 320;
        config.logicalHeight = 240;
        config.vsync = false;

        const bool ok = win.init(config);
        TEST_ASSERT_TRUE(ok, "SdlWindow init should succeed");

        SdlRenderer renderer;
        renderer.attachRenderer(win.getRenderer());
        TEST_ASSERT_TRUE(renderer.getRenderer() == win.getRenderer(), "Renderer pointer should match");
        TEST_ASSERT_TRUE(renderer.getAtlas().isLoaded(), "SpriteAtlas should be initialized on attachRenderer");
        TEST_ASSERT_TRUE(renderer.getAtlas().getTexture() != nullptr, "SpriteAtlas texture should be non-null");

        const SDL_Rect robotWalkRect { renderer.getAtlas().getSourceRect(ShooterCore::SpriteId::PlayerRobotWalk1) };
        TEST_ASSERT_EQ(robotWalkRect.w, 16, "Robot walk sprite width should be 16");
        TEST_ASSERT_EQ(robotWalkRect.h, 32, "Robot walk sprite height should be 32");

        const SDL_Rect jetRect { renderer.getAtlas().getSourceRect(ShooterCore::SpriteId::PlayerJet) };
        TEST_ASSERT_EQ(jetRect.w, 32, "Jet sprite width should be 32");
        TEST_ASSERT_EQ(jetRect.h, 16, "Jet sprite height should be 16");

        const SDL_Rect tileRect { renderer.getAtlas().getTileSourceRect(ShooterCore::TileType::SolidWall) };
        TEST_ASSERT_EQ(tileRect.w, 16, "SolidWall tile width should be 16");
        TEST_ASSERT_EQ(tileRect.h, 16, "SolidWall tile height should be 16");

        // Build a rich test queue
        ShooterCore::RenderCommandQueue queue;
        (void)queue.pushTile(ShooterCore::TileType::SolidWall, ShooterCore::WorldPos { 0.0f, 0.0f }, 0U);
        (void)queue.pushTile(ShooterCore::TileType::DestructibleWall, ShooterCore::WorldPos { 16.0f, 0.0f }, 0U);
        (void)queue.pushTile(ShooterCore::TileType::Hazard, ShooterCore::WorldPos { 32.0f, 0.0f }, 0U);

        (void)queue.pushSprite(ShooterCore::SpriteId::PlayerRobotWalk1, ShooterCore::WorldPos { 50.0f, 50.0f },
            ShooterCore::FacingDirection::Right, 1U);
        (void)queue.pushSprite(ShooterCore::SpriteId::PlayerRobotWalk2, ShooterCore::WorldPos { 70.0f, 50.0f },
            ShooterCore::FacingDirection::Left, 1U);
        (void)queue.pushSprite(ShooterCore::SpriteId::PlayerRobotJump, ShooterCore::WorldPos { 90.0f, 50.0f },
            ShooterCore::FacingDirection::Right, 1U);
        (void)queue.pushSprite(ShooterCore::SpriteId::PlayerJet, ShooterCore::WorldPos { 120.0f, 50.0f },
            ShooterCore::FacingDirection::Right, 1U);
        (void)queue.pushSprite(ShooterCore::SpriteId::PlayerTransforming, ShooterCore::WorldPos { 160.0f, 50.0f },
            ShooterCore::FacingDirection::Left, 1U);
        (void)queue.pushSprite(ShooterCore::SpriteId::EnemyPatrol, ShooterCore::WorldPos { 190.0f, 50.0f },
            ShooterCore::FacingDirection::Left, 1U);
        (void)queue.pushSprite(ShooterCore::SpriteId::EnemyFlyer, ShooterCore::WorldPos { 210.0f, 50.0f },
            ShooterCore::FacingDirection::Right, 1U);
        (void)queue.pushSprite(ShooterCore::SpriteId::Explosion, ShooterCore::WorldPos { 240.0f, 50.0f },
            ShooterCore::FacingDirection::Right, 2U);
        (void)queue.pushSprite(ShooterCore::SpriteId::SparkParticle, ShooterCore::WorldPos { 260.0f, 50.0f },
            ShooterCore::FacingDirection::Right, 2U);

        (void)queue.pushBeam(ShooterCore::WorldPos { 50.0f, 50.0f }, ShooterCore::WorldPos { 200.0f, 50.0f },
            ShooterCore::ColorRgba { 255U, 100U, 50U, 200U });

        (void)queue.pushBar(ShooterCore::WorldPos { 10.0f, 10.0f }, 100.0f, 8.0f, 0.75f,
            ShooterCore::ColorRgba { 50U, 220U, 60U, 255U });

        TEST_ASSERT_TRUE(queue.count() > 10U, "Queue should contain populated commands");

        // Render entire frame without throwing or crashing
        renderer.renderFrame(queue);
    }
}

} // namespace SdlAdapter::Test
