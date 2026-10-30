#include "SdlWindow.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <array>
#include <string_view>
#include <utility>

#include <SDL.h>

namespace SdlAdapter {

SdlWindow::~SdlWindow() noexcept
{
    shutdown();
}

SdlWindow::SdlWindow(SdlWindow&& other) noexcept
    : window_ { other.window_ }
    , renderer_ { other.renderer_ }
    , videoInitialized_ { other.videoInitialized_ }
{
    other.window_ = nullptr;
    other.renderer_ = nullptr;
    other.videoInitialized_ = false;
}

SdlWindow& SdlWindow::operator=(SdlWindow&& other) noexcept
{
    if (this != &other) {
        shutdown();
        window_ = other.window_;
        renderer_ = other.renderer_;
        videoInitialized_ = other.videoInitialized_;

        other.window_ = nullptr;
        other.renderer_ = nullptr;
        other.videoInitialized_ = false;
    }
    return *this;
}

bool SdlWindow::init(const WindowConfig& config) noexcept
{
    shutdown();

    if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
        return false;
    }
    videoInitialized_ = true;

    std::array<char, 128> titleBuf { };
    const std::size_t copyLen { std::min(config.title.size(), titleBuf.size() - 1U) };
    for (std::size_t i { 0U }; i < copyLen; ++i) {
        titleBuf[i] = config.title[i];
    }
    titleBuf[copyLen] = '\0';

    std::uint32_t winFlags { SDL_WINDOW_SHOWN };
    if (config.resizable) {
        winFlags |= SDL_WINDOW_RESIZABLE;
    }
    if (config.highDpi) {
        winFlags |= SDL_WINDOW_ALLOW_HIGHDPI;
    }

    window_ = SDL_CreateWindow(titleBuf.data(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, config.windowWidth,
        config.windowHeight, winFlags);

    if (window_ == nullptr) {
        shutdown();
        return false;
    }

    std::uint32_t renFlags { SDL_RENDERER_ACCELERATED };
    if (config.vsync) {
        renFlags |= SDL_RENDERER_PRESENTVSYNC;
    }

    renderer_ = SDL_CreateRenderer(window_, -1, renFlags);
    if (renderer_ == nullptr) {
        // Fallback to software renderer if hardware accelerated fails (e.g. In virtualized/dummy environments)
        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
    }

    if (renderer_ == nullptr) {
        shutdown();
        return false;
    }

    if (config.logicalWidth > 0 && config.logicalHeight > 0) {
        (void)SDL_RenderSetLogicalSize(renderer_, config.logicalWidth, config.logicalHeight);
    }

    return true;
}

void SdlWindow::shutdown() noexcept
{
    if (renderer_ != nullptr) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }

    if (window_ != nullptr) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }

    if (videoInitialized_) {
        SDL_QuitSubSystem(SDL_INIT_VIDEO);
        videoInitialized_ = false;
    }
}

SDL_Window* SdlWindow::getWindow() const noexcept
{
    return window_;
}

SDL_Renderer* SdlWindow::getRenderer() const noexcept
{
    return renderer_;
}

bool SdlWindow::isValid() const noexcept
{
    return window_ != nullptr && renderer_ != nullptr;
}

} // namespace SdlAdapter
