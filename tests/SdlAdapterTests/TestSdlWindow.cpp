#include "TestHarness.h"

#include <cstdint>

#include "SdlWindow.h"

namespace SdlAdapter::Test {

void runSdlWindowTests() noexcept
{
    // Test 1: Uninitialized window state
    {
        SdlWindow win;
        TEST_ASSERT_FALSE(win.isValid(), "Default SdlWindow must not be valid");
        TEST_ASSERT_TRUE(win.getWindow() == nullptr, "Default SdlWindow must have null window handle");
        TEST_ASSERT_TRUE(win.getRenderer() == nullptr, "Default SdlWindow must have null renderer handle");
    }

    // Test 2: Initialization with dummy backend
    {
        SdlWindow win;
        WindowConfig config { };
        config.title = "Test Window";
        config.windowWidth = 320;
        config.windowHeight = 240;
        config.logicalWidth = 320;
        config.logicalHeight = 240;
        config.vsync = false;

        const bool ok = win.init(config);
        TEST_ASSERT_TRUE(ok, "SdlWindow::init should succeed with dummy driver");
        TEST_ASSERT_TRUE(win.isValid(), "SdlWindow should be valid after init");
        TEST_ASSERT_TRUE(win.getWindow() != nullptr, "Window handle must be non-null after init");
        TEST_ASSERT_TRUE(win.getRenderer() != nullptr, "Renderer handle must be non-null after init");

        // Test 3: Move semantics
        SdlWindow winMoved { std::move(win) };
        TEST_ASSERT_TRUE(winMoved.isValid(), "Moved-to SdlWindow must be valid");
        TEST_ASSERT_FALSE(win.isValid(), "Moved-from SdlWindow must be invalid");
        TEST_ASSERT_TRUE(win.getWindow() == nullptr, "Moved-from window handle must be null");

        // Test 4: Explicit shutdown
        winMoved.shutdown();
        TEST_ASSERT_FALSE(winMoved.isValid(), "SdlWindow must be invalid after shutdown");
    }
}

} // namespace SdlAdapter::Test
