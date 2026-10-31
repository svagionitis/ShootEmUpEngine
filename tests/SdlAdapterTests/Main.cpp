#include "TestHarness.h"

#include <cstddef>
#include <cstdint>

#include <iostream>
#include <string>
#include <string_view>

#include <glog/logging.h>
#include <gtest/gtest.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>

TEST(SdlAdapterTests, Window)
{
    SdlAdapter::Test::runSdlWindowTests();
}

TEST(SdlAdapterTests, Input)
{
    SdlAdapter::Test::runSdlInputTests();
}

TEST(SdlAdapterTests, TimePacer)
{
    SdlAdapter::Test::runTimePacerTests();
}

TEST(SdlAdapterTests, Renderer)
{
    SdlAdapter::Test::runSdlRendererTests();
}

TEST(SdlAdapterTests, Audio)
{
    SdlAdapter::Test::runSdlAudioTests();
}

TEST(SdlAdapterTests, AdsrEnvelope)
{
    SdlAdapter::Test::runAdsrEnvelopeTests();
}

TEST(SdlAdapterTests, MusicTracker)
{
    SdlAdapter::Test::runMusicTrackerTests();
}

int main(int argc, char* argv[])
{
    SDL_SetMainReady();

    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = 1;

    LOG(INFO) << "Starting SdlAdapter GoogleTest & GLog Suite...";

    // Force SDL headless dummy drivers if running in headless environment or tests
    (void)SDL_setenv("SDL_VIDEODRIVER", "dummy", 0);
    (void)SDL_setenv("SDL_AUDIODRIVER", "dummy", 0);

    testing::InitGoogleTest(&argc, argv);

    // Support --suite=<Name> mapping to GoogleTest filter
    for (int i { 1 }; i < argc; ++i) {
        const std::string_view arg { argv[i] };
        if (arg.rfind("--suite=", 0U) == 0U) {
            std::string suiteName { arg.substr(8U) };
            if (suiteName == "SdlWindow") {
                suiteName = "Window";
            } else if (suiteName == "SdlInput") {
                suiteName = "Input";
            } else if (suiteName == "SdlRenderer") {
                suiteName = "Renderer";
            } else if (suiteName == "SdlAudio") {
                suiteName = "Audio";
            } else if (suiteName == "SdlAdsr") {
                suiteName = "AdsrEnvelope";
            } else if (suiteName == "SdlTracker") {
                suiteName = "MusicTracker";
            }
            const std::string suiteFilter { std::string("SdlAdapterTests.") + suiteName };
            ::testing::GTEST_FLAG(filter) = suiteFilter;
            LOG(INFO) << "Applied suite filter: " << suiteFilter;
        }
    }

    return RUN_ALL_TESTS();
}
