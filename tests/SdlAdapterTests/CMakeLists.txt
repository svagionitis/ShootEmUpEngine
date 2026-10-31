add_executable(SdlAdapterTests
    Main.cpp
    TestSdlAudio.cpp
    TestSdlInput.cpp
    TestSdlRenderer.cpp
    TestSdlWindow.cpp
    TestTimePacer.cpp
)

target_link_libraries(SdlAdapterTests PRIVATE
    SdlAdapter
    ShooterCore
    GTest::gtest
    glog::glog
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)

target_include_directories(SdlAdapterTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/libs/ShooterCore
    ${CMAKE_SOURCE_DIR}/libs/SdlAdapter
)

copy_executable_dependencies(SdlAdapterTests)

apply_compiler_flags(SdlAdapterTests)

# Register Master Test Run
add_test(NAME SdlAdapterAllTests COMMAND SdlAdapterTests)
set_tests_properties(SdlAdapterAllTests PROPERTIES LABELS "master;all;sdl" TIMEOUT 30)

# Register Granular Modular Suites for CTest
macro(register_sdladapter_test TEST_NAME SUITE_NAME LABELS_STR TIMEOUT_SEC)
    add_test(NAME ${TEST_NAME} COMMAND SdlAdapterTests --suite=${SUITE_NAME})
    set_tests_properties(${TEST_NAME} PROPERTIES LABELS "${LABELS_STR}" TIMEOUT ${TIMEOUT_SEC})
endmacro()

register_sdladapter_test(SdlAdapter_Window    SdlWindow    "unit;window;sdl"    20)
register_sdladapter_test(SdlAdapter_Input     SdlInput     "unit;input;sdl"     10)
register_sdladapter_test(SdlAdapter_TimePacer TimePacer    "unit;timing;sdl"    10)
register_sdladapter_test(SdlAdapter_Renderer  SdlRenderer  "unit;renderer;sdl"  20)
register_sdladapter_test(SdlAdapter_Audio     SdlAudio     "unit;audio;sdl"     10)
