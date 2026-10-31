#pragma once

#include <cstdint>
#include <string_view>

struct SDL_Window;
struct SDL_Renderer;

namespace SdlAdapter {

/// @struct WindowConfig
/// @brief Configuration settings for window and renderer initialization.
struct WindowConfig final {
    std::string_view title { "Thexder Shoot'Em-Up" };
    std::int32_t windowWidth { 960 };
    std::int32_t windowHeight { 720 };
    std::int32_t logicalWidth { 320 };
    std::int32_t logicalHeight { 240 };
    bool vsync { true };
    bool resizable { true };
    bool highDpi { true };
};

/// @class SdlWindow
/// @brief RAII container managing SDL_Window and hardware SDL_Renderer lifecycles.
class SdlWindow final {
public:
    SdlWindow() noexcept = default;
    ~SdlWindow() noexcept;

    SdlWindow(const SdlWindow&) = delete;
    SdlWindow& operator=(const SdlWindow&) = delete;

    SdlWindow(SdlWindow&& other) noexcept;
    SdlWindow& operator=(SdlWindow&& other) noexcept;

    /// @brief Initialize SDL video and create window and hardware renderer.
    /// @param[in] config Window parameters and logical resolution dimensions.
    /// @return true if initialization succeeded; false otherwise.
    [[nodiscard]] bool init(const WindowConfig& config) noexcept;

    /// @brief Release window and renderer resources and shut down video subsystem.
    void shutdown() noexcept;

    /// @brief Retrieve underlying SDL window handle.
    /// @return Pointer to SDL_Window, or nullptr if not initialized.
    [[nodiscard]] SDL_Window* getWindow() const noexcept;

    /// @brief Retrieve underlying hardware SDL renderer handle.
    /// @return Pointer to SDL_Renderer, or nullptr if not initialized.
    [[nodiscard]] SDL_Renderer* getRenderer() const noexcept;

    /// @brief Query whether context is successfully initialized.
    /// @return true if both window and renderer are valid.
    [[nodiscard]] bool isValid() const noexcept;

    /// @brief Toggle between windowed mode and desktop fullscreen.
    void toggleFullscreen() noexcept;

    /// @brief Query whether window is currently in fullscreen mode.
    /// @return true if fullscreen desktop flag is active.
    [[nodiscard]] bool isFullscreen() const noexcept;

private:
    SDL_Window* window_ { nullptr };
    SDL_Renderer* renderer_ { nullptr };
    bool videoInitialized_ { false };
};

} // namespace SdlAdapter
