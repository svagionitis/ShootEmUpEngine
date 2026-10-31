#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include "DualResolutionRenderer.h"
#include "Types.h"

namespace SdlAdapter::Test {

void runDualResolutionRendererTests() noexcept
{
    LOG(INFO) << "Executing DualResolutionRenderer Tests...";

    // 1. Test coordinate transformations
    const ShooterCore::WorldPos mcgaPoint { 100.0f, 84.0f };
    const ShooterCore::WorldPos compositePoint { DualResolutionRenderer::playfieldToComposite(mcgaPoint) };
    TEST_ASSERT_EQ(compositePoint.x, 200.0f, "X must be scaled 2x in composite space");
    TEST_ASSERT_EQ(compositePoint.y, 84.0f, "Y must remain identical");

    const ShooterCore::WorldPos restoredPoint { DualResolutionRenderer::compositeToPlayfield(compositePoint) };
    TEST_ASSERT_EQ(restoredPoint.x, 100.0f, "Restored X must match original MCGA coordinate");
    TEST_ASSERT_EQ(restoredPoint.y, 84.0f, "Restored Y must match original MCGA coordinate");

    // 2. Test playfield boundary queries
    TEST_ASSERT_TRUE(
        DualResolutionRenderer::isInsidePlayfield(ShooterCore::WorldPos { 160.0f, 100.0f }), "Center must be inside");
    TEST_ASSERT_FALSE(DualResolutionRenderer::isInsidePlayfield(ShooterCore::WorldPos { 160.0f, 175.0f }),
        "Y >= 168 is outside playfield");
    TEST_ASSERT_FALSE(DualResolutionRenderer::isInsidePlayfield(ShooterCore::WorldPos { 330.0f, 100.0f }),
        "X >= 320 is outside playfield");

    // 3. Test dashboard boundary queries
    TEST_ASSERT_TRUE(DualResolutionRenderer::isInsideDashboard(ShooterCore::WorldPos { 320.0f, 180.0f }),
        "Dashboard center is inside");
    TEST_ASSERT_FALSE(DualResolutionRenderer::isInsideDashboard(ShooterCore::WorldPos { 320.0f, 160.0f }),
        "Y < 168 is outside dashboard");

    // 4. Test 4:3 letterboxing & pillarboxing math
    std::int32_t x { 0 };
    std::int32_t y { 0 };
    std::int32_t w { 0 };
    std::int32_t h { 0 };

    // Exact 4:3 window (1024x768)
    DualResolutionRenderer::computeLetterboxRect(1024, 768, x, y, w, h);
    TEST_ASSERT_EQ(x, 0, "No X offset for exact 4:3");
    TEST_ASSERT_EQ(y, 0, "No Y offset for exact 4:3");
    TEST_ASSERT_EQ(w, 1024, "Full width used");
    TEST_ASSERT_EQ(h, 768, "Full height used");

    // 16:9 widescreen window (1920x1080) -> Pillarboxed
    DualResolutionRenderer::computeLetterboxRect(1920, 1080, x, y, w, h);
    TEST_ASSERT_EQ(h, 1080, "Height fills window in pillarbox mode");
    TEST_ASSERT_EQ(w, 1440, "Width scaled to 1440 for 4:3 aspect");
    TEST_ASSERT_EQ(x, (1920 - 1440) / 2, "Pillarbox bars centered on X");
    TEST_ASSERT_EQ(y, 0, "Y offset must be 0 for pillarbox");
}

} // namespace SdlAdapter::Test
