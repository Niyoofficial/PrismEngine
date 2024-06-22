#pragma once
#include "Prism-Core/Base/KeyCode.h"
#include "SDL3/SDL_keycode.h"

namespace Prism::SDL
{
KeyCode GetPrismKeyCode(SDL_KeyCode sdlKeyCode);
ScanCode GetPrismScanCode(SDL_Scancode sdlScanCode);
KeyCode GetPrismKeyCodeFromMouseButton(uint8_t button);

SDL_KeyCode GetSDLKeyCode(KeyCode keyCode);
SDL_Scancode GetSDLScanCode(ScanCode scanCode);
}
