#include "SdlInput.h"

#include <cstdint>

#include <SDL.h>

#include "EngineInput.h"

namespace SdlAdapter {

bool SdlInput::pollEvents() noexcept
{
    previousInput_ = currentInput_;
    f1Pressed_ = false;
    f2Pressed_ = false;
    f3Pressed_ = false;
    f11Pressed_ = false;

    SDL_Event event { };
    while (SDL_PollEvent(&event) != 0) {
        switch (event.type) {
        case SDL_QUIT:
            quitRequested_ = true;
            break;

        case SDL_KEYDOWN:
            if (event.key.repeat == 0U) {
                handleKeyEvent(event.key.keysym.sym, true);
            }
            break;

        case SDL_KEYUP:
            handleKeyEvent(event.key.keysym.sym, false);
            break;

        case SDL_CONTROLLERBUTTONDOWN:
            handleButtonEvent(event.cbutton.button, true);
            break;

        case SDL_CONTROLLERBUTTONUP:
            handleButtonEvent(event.cbutton.button, false);
            break;

        default:
            break;
        }
    }

    return !quitRequested_;
}

void SdlInput::handleKeyEvent(std::int32_t keycode, bool isDown) noexcept
{
    std::uint16_t mask { ShooterCore::EngineInput::kNone };

    switch (keycode) {
    case SDLK_UP:
    case SDLK_w:
        mask = ShooterCore::EngineInput::kUp;
        break;

    case SDLK_DOWN:
    case SDLK_s:
        mask = ShooterCore::EngineInput::kDown;
        break;

    case SDLK_LEFT:
    case SDLK_a:
        mask = ShooterCore::EngineInput::kLeft;
        break;

    case SDLK_RIGHT:
    case SDLK_d:
        mask = ShooterCore::EngineInput::kRight;
        break;

    case SDLK_SPACE:
    case SDLK_z:
        mask = ShooterCore::EngineInput::kJump;
        break;

    case SDLK_TAB:
    case SDLK_RETURN:
        mask = ShooterCore::EngineInput::kTransform;
        break;

    case SDLK_x:
    case SDLK_k:
        mask = ShooterCore::EngineInput::kFire;
        break;

    case SDLK_c:
    case SDLK_LSHIFT:
    case SDLK_RSHIFT:
        mask = ShooterCore::EngineInput::kShield;
        break;

    case SDLK_v:
    case SDLK_f:
        mask = ShooterCore::EngineInput::kFireSecondary;
        break;

    case SDLK_q:
    case SDLK_e:
        mask = ShooterCore::EngineInput::kCycleWeapon;
        break;

    case SDLK_ESCAPE:
        if (isDown) {
            quitRequested_ = true;
        }
        return;

    case SDLK_F1:
        if (isDown) {
            f1Pressed_ = true;
        }
        return;

    case SDLK_F2:
        if (isDown) {
            f2Pressed_ = true;
        }
        return;

    case SDLK_F3:
        if (isDown) {
            f3Pressed_ = true;
        }
        return;

    case SDLK_F11:
        if (isDown) {
            f11Pressed_ = true;
        }
        return;

    default:
        return;
    }

    if (isDown) {
        currentInput_.setButton(mask);
    } else {
        currentInput_.clearButton(mask);
    }
}

void SdlInput::handleButtonEvent(std::uint8_t button, bool isDown) noexcept
{
    std::uint16_t mask { ShooterCore::EngineInput::kNone };

    switch (button) {
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
        mask = ShooterCore::EngineInput::kUp;
        break;

    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        mask = ShooterCore::EngineInput::kDown;
        break;

    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        mask = ShooterCore::EngineInput::kLeft;
        break;

    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        mask = ShooterCore::EngineInput::kRight;
        break;

    case SDL_CONTROLLER_BUTTON_A:
        mask = ShooterCore::EngineInput::kJump;
        break;

    case SDL_CONTROLLER_BUTTON_Y:
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
        mask = ShooterCore::EngineInput::kTransform;
        break;

    case SDL_CONTROLLER_BUTTON_X:
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
        mask = ShooterCore::EngineInput::kFire;
        break;

    case SDL_CONTROLLER_BUTTON_B:
        mask = ShooterCore::EngineInput::kShield;
        break;

    case SDL_CONTROLLER_BUTTON_START:
    case SDL_CONTROLLER_BUTTON_BACK:
        if (isDown) {
            quitRequested_ = true;
        }
        return;

    default:
        return;
    }

    if (isDown) {
        currentInput_.setButton(mask);
    } else {
        currentInput_.clearButton(mask);
    }
}

} // namespace SdlAdapter
