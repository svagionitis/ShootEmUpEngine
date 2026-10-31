add_library(SdlAdapter STATIC
    AdsrEnvelope.cpp
    AdsrEnvelope.h
    CanvasScaler.cpp
    CanvasScaler.h
    ChiptuneVoice.cpp
    ChiptuneVoice.h
    CrtPostProcessor.cpp
    CrtPostProcessor.h
    EgaAssetLoader.cpp
    EgaAssetLoader.h
    EgaBugLoader.cpp
    EgaBugLoader.h
    EgaMapLoader.cpp
    EgaMapLoader.h
    EgaRobotLoader.cpp
    EgaRobotLoader.h
    MusicTracker.cpp
    MusicTracker.h
    SarArchiveReader.cpp
    SarArchiveReader.h
    SdlAudio.cpp
    SdlAudio.h
    SdlInput.cpp
    SdlInput.h
    SdlRenderer.cpp
    SdlRenderer.h
    SdlWindow.cpp
    SdlWindow.h
    SoundPresets.h
    SpriteAtlas.cpp
    SpriteAtlas.h
    TimePacer.cpp
    TimePacer.h
    TrackBossBattle.h
    TrackFireHawk.h
    TrackMoonlightSonata.h
)

target_include_directories(SdlAdapter PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(SdlAdapter PUBLIC
    ShooterCore
    glog::glog
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

apply_compiler_flags(SdlAdapter)
