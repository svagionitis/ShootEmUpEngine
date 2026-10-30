#include "TestHarness.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <chrono>
#include <iostream>
#include <string_view>

#define SDL_MAIN_HANDLED
#include <SDL.h>

namespace {

struct SuiteEntry {
    std::string_view name;
    void (*runner)() noexcept;
};

const SuiteEntry kSuites[] = { { "SdlWindow", &SdlAdapter::Test::runSdlWindowTests },
    { "SdlInput", &SdlAdapter::Test::runSdlInputTests }, { "TimePacer", &SdlAdapter::Test::runTimePacerTests },
    { "SdlRenderer", &SdlAdapter::Test::runSdlRendererTests }, { "SdlAudio", &SdlAdapter::Test::runSdlAudioTests } };

} // namespace

int main(int argc, char* argv[])
{
    SDL_SetMainReady();

    // Force SDL headless dummy drivers if running in headless environment or tests
    (void)SDL_setenv("SDL_VIDEODRIVER", "dummy", 0);
    (void)SDL_setenv("SDL_AUDIODRIVER", "dummy", 0);

    std::string_view targetSuite { };

    for (int i { 1 }; i < argc; ++i) {
        const std::string_view arg { argv[i] };
        if (arg == "--list") {
            std::cout << "Available SdlAdapter test suites:\n";
            for (const auto& entry : kSuites) {
                std::cout << "  - " << entry.name << "\n";
            }
            return 0;
        }
        if (arg.rfind("--suite=", 0U) == 0U) {
            targetSuite = arg.substr(8U);
        }
    }

    std::cout << "[INFO] Starting SdlAdapter Automated Headless Tests...\n";
    const auto startTime = std::chrono::high_resolution_clock::now();

    for (const auto& entry : kSuites) {
        if (!targetSuite.empty() && entry.name != targetSuite) {
            continue;
        }

        std::cout << "[RUN ] Suite: " << entry.name << "\n";
        const std::uint32_t failBefore = SdlAdapter::Test::gTestStats.failedAssertions;

        entry.runner();

        if (SdlAdapter::Test::gTestStats.failedAssertions == failBefore) {
            std::cout << "[OK  ] Suite: " << entry.name << " passed.\n";
            ++SdlAdapter::Test::gTestStats.passedSuites;
        } else {
            std::cerr << "[FAIL] Suite: " << entry.name << " had failures!\n";
            ++SdlAdapter::Test::gTestStats.failedSuites;
        }
    }

    const auto endTime = std::chrono::high_resolution_clock::now();
    const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    std::cout << "\n======================================================\n";
    std::cout << "Test Summary: " << SdlAdapter::Test::gTestStats.passedSuites << " suites passed, "
              << SdlAdapter::Test::gTestStats.failedSuites << " suites failed. (" << elapsedMs << " ms total)\n";
    std::cout << "Assertions:   "
              << (SdlAdapter::Test::gTestStats.totalAssertions - SdlAdapter::Test::gTestStats.failedAssertions)
              << " passed, " << SdlAdapter::Test::gTestStats.failedAssertions << " failed.\n";
    std::cout << "======================================================\n";

    return (SdlAdapter::Test::gTestStats.failedAssertions == 0U) ? 0 : 1;
}
