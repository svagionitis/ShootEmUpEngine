#include <algorithm>
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
DEFINE_int32(crt_mode, 2, "CRT optical filter profile (0=Off, 1=Subtle, 2=Arcade, 3=RetroCrt)");
DEFINE_bool(integer_scale, true, "Force pixel-perfect integer aspect ratio scaling");
DEFINE_bool(screen_shake, true, "Enable camera trauma and kinetic screen shake");

int main(int argc, char* argv[])
{
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    LOG(INFO) << "Starting Firehawk: Thexder - The Second Contact Engine...";

    SdlAdapter::SdlWindow window;
    SdlAdapter::WindowConfig winConfig;
    winConfig.title = "Firehawk: Thexder - The Second Contact";
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

    const auto crtModeVal { static_cast<ShooterCore::CrtFilterMode>(std::clamp(FLAGS_crt_mode, 0, 3)) };
    renderer.setCrtFilterMode(crtModeVal);
    renderer.setAspectRatioMode(
        FLAGS_integer_scale ? ShooterCore::AspectRatioMode::IntegerOnly : ShooterCore::AspectRatioMode::UniformFit);

    SdlAdapter::SdlAudio audio;
    if (!FLAGS_mute) {
        if (!audio.init()) {
            LOG(WARNING) << "Audio device could not be opened; continuing without sound.";
        } else {
            audio.playMusic(ShooterCore::MusicTrackId::MoonlightSonata);
        }
    }

    SdlAdapter::SdlInput input;
    SdlAdapter::TimePacer pacer;
    ThexderGame::GameState game;
    game.setGodMode(FLAGS_godmode);
    game.setScreenShakeEnabled(FLAGS_screen_shake);

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

        // Phase 14 Runtime Hotkeys: F1 (CRT), F2 (Aspect Scale), F3 (Screen Shake), F11 (Fullscreen)
        if (input.isCrtToggle()) {
            const ShooterCore::CrtFilterMode nextCrt { renderer.cycleCrtMode() };
            LOG(INFO) << "Toggled CRT filter profile: " << static_cast<int>(nextCrt);
        }
        if (input.isScaleToggle()) {
            const ShooterCore::AspectRatioMode nextScale { renderer.cycleAspectMode() };
            LOG(INFO) << "Toggled Aspect Ratio scaling: " << static_cast<int>(nextScale);
        }
        if (input.isShakeToggle()) {
            game.toggleScreenShake();
            LOG(INFO) << "Toggled Screen Shake: " << (game.isScreenShakeEnabled() ? "ENABLED" : "DISABLED");
        }
        if (input.isFullscreenToggle()) {
            window.toggleFullscreen();
            LOG(INFO) << "Toggled Fullscreen mode.";
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
    renderer.attachRenderer(nullptr);
    window.shutdown();

    LOG(INFO) << "Thexder Game terminated cleanly.";
    return EXIT_SUCCESS;
}
