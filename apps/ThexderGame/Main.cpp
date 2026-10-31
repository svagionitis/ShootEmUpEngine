#include <cstdint>
#include <cstdlib>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "GameState.h"
#include "RenderCommandQueue.h"
#include "SdlAudio.h"
#include "SdlInput.h"
#include "SdlRenderer.h"
#include "SdlWindow.h"
#include "TimePacer.h"
#include "Types.h"

DEFINE_int32(width, 960, "Physical display window width in pixels");
DEFINE_int32(height, 720, "Physical display window height in pixels");
DEFINE_bool(vsync, true, "Enable vertical retrace synchronization");
DEFINE_bool(mute, false, "Mute procedural chiptune synthesizer");
DEFINE_bool(godmode, false, "Invulnerable player craft for test navigation");
DEFINE_bool(headless, false, "Run in headless mode for automated verification");
DEFINE_int32(frames, 60, "Number of frames to simulate before exiting in headless mode");

int main(int argc, char* argv[])
{
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    LOG(INFO) << "Starting Thexder Game Engine (Phase 8)...";

    SdlAdapter::SdlWindow window;
    SdlAdapter::WindowConfig winConfig;
    winConfig.title = "Thexder - 1985 Game Arts Tribute";
    winConfig.windowWidth = FLAGS_width;
    winConfig.windowHeight = FLAGS_height;
    winConfig.logicalWidth = 320;
    winConfig.logicalHeight = 240;
    winConfig.vsync = FLAGS_vsync;

    if (!window.init(winConfig)) {
        LOG(ERROR) << "Failed to initialize SDL window and hardware renderer.";
        return EXIT_FAILURE;
    }

    SdlAdapter::SdlRenderer renderer;
    renderer.attachRenderer(window.getRenderer());

    SdlAdapter::SdlAudio audio;
    if (!FLAGS_mute) {
        if (!audio.init()) {
            LOG(WARNING) << "Audio device could not be opened; continuing without sound.";
        } else {
            audio.playMusic(ShooterCore::MusicTrackId::Level1Cavern);
        }
    }

    SdlAdapter::SdlInput input;
    SdlAdapter::TimePacer pacer;
    ThexderGame::GameState game;
    game.setGodMode(FLAGS_godmode);

    ShooterCore::RenderCommandQueue frameQueue;

    pacer.start();
    bool running { true };
    int simulatedFrames { 0 };

    while (running) {
        if (!input.pollEvents()) {
            running = false;
            break;
        }

        if (input.quitRequested()) {
            running = false;
            break;
        }

        const std::uint32_t ticks { pacer.beginFrame() };
        for (std::uint32_t t { 0U }; t < ticks; ++t) {
            game.step(ShooterCore::DeltaTime { pacer.fixedDelta() }, input.currentInput());
        }

        if (!FLAGS_mute) {
            audio.processQueue(game.getEngine().getAudioQueue());
        }

        game.buildRenderQueue(frameQueue);
        renderer.renderFrame(frameQueue);

        if (FLAGS_headless) {
            ++simulatedFrames;
            if (simulatedFrames >= FLAGS_frames) {
                running = false;
            }
        }
    }

    if (!FLAGS_mute) {
        audio.shutdown();
    }
    window.shutdown();

    LOG(INFO) << "Thexder Game terminated cleanly.";
    return EXIT_SUCCESS;
}
