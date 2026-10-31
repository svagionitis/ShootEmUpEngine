#include "TestHarness.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "HudOverlay.h"
#include "RenderCommandQueue.h"
#include "Types.h"

namespace ThexderGame::Test {

void runHudOverlayTests()
{
    LOG(INFO) << "Executing HudOverlay Unit & Zero-Heap Formatting Tests...";

    // 1. Stack buffer decimal formatting
    std::array<char, 16> buf { };

    std::size_t len { HudOverlay::formatDecimal(buf.data(), buf.size(), 0U, 1U) };
    TEST_ASSERT_EQ(len, 1U, "Formatting 0 with minDigits 1 length");
    TEST_ASSERT(std::string_view { buf.data() } == "0", "Formatted 0 value");

    len = HudOverlay::formatDecimal(buf.data(), buf.size(), 0U, 6U);
    TEST_ASSERT_EQ(len, 6U, "Formatting 0 with minDigits 6 length");
    TEST_ASSERT(std::string_view { buf.data() } == "000000", "Formatted 000000 value");

    len = HudOverlay::formatDecimal(buf.data(), buf.size(), 14500U, 7U);
    TEST_ASSERT_EQ(len, 7U, "Formatting 14500 with minDigits 7 length");
    TEST_ASSERT(std::string_view { buf.data() } == "0014500", "Formatted 0014500 value");

    len = HudOverlay::formatDecimal(buf.data(), buf.size(), 100U, 3U);
    TEST_ASSERT_EQ(len, 3U, "Formatting 100 with minDigits 3 length");
    TEST_ASSERT(std::string_view { buf.data() } == "100", "Formatted 100 value");

    len = HudOverlay::formatDecimal(buf.data(), buf.size(), 999999U, 4U);
    TEST_ASSERT_EQ(len, 6U, "Formatting 999999 length");
    TEST_ASSERT(std::string_view { buf.data() } == "999999", "Formatted 999999 value");

    // Buffer edge cases
    len = HudOverlay::formatDecimal(nullptr, 16U, 42U, 2U);
    TEST_ASSERT_EQ(len, 0U, "Nullptr buffer returns 0");
    len = HudOverlay::formatDecimal(buf.data(), 0U, 42U, 2U);
    TEST_ASSERT_EQ(len, 0U, "Zero capacity buffer returns 0");

    // 2. Text rasterization into command queue
    HudOverlay hud { };
    ShooterCore::RenderCommandQueue queue { };

    HudOverlay::drawText(queue, "AB", 10.0f, 10.0f, 1U, ShooterCore::ColorRgba { 255U, 255U, 255U, 255U });
    TEST_ASSERT(queue.count() > 0U, "drawText should enqueue non-zero pixel commands");
    const std::size_t countAfterText { queue.count() };

    // 3. Full HUD generation
    queue.clear();
    hud.renderHud(queue, ShooterCore::EnergyPoints { 100U }, ShooterCore::ShieldGauge { 100U }, 12500U, 50000U,
        ShooterCore::TransformMode::Robot, 0.0f);

    TEST_ASSERT(queue.count() > 0U, "renderHud must enqueue HUD commands");
    TEST_ASSERT(queue.count() < ShooterCore::RenderCommandQueue::kCapacity, "Commands must not overflow queue");

    // 4. Low energy alert rendering
    queue.clear();
    hud.renderHud(queue, ShooterCore::EnergyPoints { 10U }, ShooterCore::ShieldGauge { 50U }, 12500U, 50000U,
        ShooterCore::TransformMode::Jet, 0.1f);

    TEST_ASSERT(queue.count() > 0U, "Low-energy HUD must render commands");
    TEST_ASSERT(queue.count() < ShooterCore::RenderCommandQueue::kCapacity, "Low energy HUD must fit within capacity");
}

} // namespace ThexderGame::Test
