#include "TestHarness.h"

#include <cstdint>

#include "CanvasScaler.h"
#include "Types.h"

namespace SdlAdapter::Test {

void runCanvasScalerTests() noexcept
{
    LOG(INFO) << "Executing CanvasScaler Integer Aspect Ratio & Viewport Tests...";

    // 1. 1080p (1920x1080): 4x integer scale (1280x960), centered with pillarbox/letterbox
    const ShooterCore::DisplayViewport vp1080 { CanvasScaler::calculateViewport(
        1920, 1080, ShooterCore::AspectRatioMode::IntegerOnly) };
    TEST_ASSERT_EQ(vp1080.integerScale, 4U, "1080p must use integer scale 4x");
    TEST_ASSERT_EQ(vp1080.width, 1280, "1080p scaled width must be 1280");
    TEST_ASSERT_EQ(vp1080.height, 960, "1080p scaled height must be 960");
    TEST_ASSERT_EQ(vp1080.x, 320, "1080p horizontal pillarbox offset must be 320");
    TEST_ASSERT_EQ(vp1080.y, 60, "1080p vertical letterbox offset must be 60");

    // 2. 1440p (2560x1440): 6x integer scale (1920x1440), centered horizontally
    const ShooterCore::DisplayViewport vp1440 { CanvasScaler::calculateViewport(
        2560, 1440, ShooterCore::AspectRatioMode::IntegerOnly) };
    TEST_ASSERT_EQ(vp1440.integerScale, 6U, "1440p must use integer scale 6x");
    TEST_ASSERT_EQ(vp1440.width, 1920, "1440p scaled width must be 1920");
    TEST_ASSERT_EQ(vp1440.height, 1440, "1440p scaled height must be 1440");
    TEST_ASSERT_EQ(vp1440.x, 320, "1440p horizontal offset must be 320");
    TEST_ASSERT_EQ(vp1440.y, 0, "1440p vertical offset must be 0");

    // 3. 4K (3840x2160): 9x integer scale (2880x2160)
    const ShooterCore::DisplayViewport vp4k { CanvasScaler::calculateViewport(
        3840, 2160, ShooterCore::AspectRatioMode::IntegerOnly) };
    TEST_ASSERT_EQ(vp4k.integerScale, 9U, "4K must use integer scale 9x");
    TEST_ASSERT_EQ(vp4k.width, 2880, "4K scaled width must be 2880");
    TEST_ASSERT_EQ(vp4k.height, 2160, "4K scaled height must be 2160");
    TEST_ASSERT_EQ(vp4k.x, 480, "4K horizontal offset must be 480");
    TEST_ASSERT_EQ(vp4k.y, 0, "4K vertical offset must be 0");

    // 4. Compact window (400x300): 1x integer scale (320x240)
    const ShooterCore::DisplayViewport vpCompact { CanvasScaler::calculateViewport(
        400, 300, ShooterCore::AspectRatioMode::IntegerOnly) };
    TEST_ASSERT_EQ(vpCompact.integerScale, 1U, "400x300 must use integer scale 1x");
    TEST_ASSERT_EQ(vpCompact.width, 320, "Compact scaled width must be 320");
    TEST_ASSERT_EQ(vpCompact.height, 240, "Compact scaled height must be 240");
    TEST_ASSERT_EQ(vpCompact.x, 40, "Compact horizontal offset must be 40");
    TEST_ASSERT_EQ(vpCompact.y, 30, "Compact vertical offset must be 30");

    // 5. Stretch Mode: fills window
    const ShooterCore::DisplayViewport vpStretch { CanvasScaler::calculateViewport(
        1920, 1080, ShooterCore::AspectRatioMode::Stretch) };
    TEST_ASSERT_EQ(vpStretch.x, 0, "Stretch mode X must be 0");
    TEST_ASSERT_EQ(vpStretch.y, 0, "Stretch mode Y must be 0");
    TEST_ASSERT_EQ(vpStretch.width, 1920, "Stretch mode width must match window");
    TEST_ASSERT_EQ(vpStretch.height, 1080, "Stretch mode height must match window");

    // 6. UniformFit Mode: uniform scaling preserving 4:3
    const ShooterCore::DisplayViewport vpUniform { CanvasScaler::calculateViewport(
        1920, 1080, ShooterCore::AspectRatioMode::UniformFit) };
    TEST_ASSERT_EQ(vpUniform.height, 1080, "Uniform fit must fill height on 16:9");
    TEST_ASSERT_EQ(vpUniform.width, 1440, "Uniform fit 4:3 width for 1080p must be 1440");
    TEST_ASSERT_EQ(vpUniform.x, 240, "Uniform fit horizontal offset must be (1920-1440)/2 = 240");
    TEST_ASSERT_EQ(vpUniform.y, 0, "Uniform fit vertical offset must be 0");

    // 7. Error hardening: zero or negative input dimensions
    const ShooterCore::DisplayViewport vpZero { CanvasScaler::calculateViewport(
        0, -10, ShooterCore::AspectRatioMode::IntegerOnly) };
    TEST_ASSERT_EQ(vpZero.width, 320, "Zero input width must fallback to native 320");
    TEST_ASSERT_EQ(vpZero.height, 240, "Zero input height must fallback to native 240");
}

} // namespace SdlAdapter::Test
