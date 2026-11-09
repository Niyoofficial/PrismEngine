#pragma once
#include "Prism/Base/KeyCode.h"
#include "SDL3/SDL_dialog.h"
#include "SDL3/SDL_keycode.h"

namespace Prism::Core {
enum class FileDialogType;
}

namespace Prism::SDL
{
KeyCode GetPrismKeyCode(SDL_Keycode sdlKeyCode);
ScanCode GetPrismScanCode(SDL_Scancode sdlScanCode);
KeyCode GetPrismKeyCodeFromMouseButton(uint8_t button);

SDL_Keycode GetSDLKeyCode(KeyCode keyCode);
SDL_Scancode GetSDLScanCode(ScanCode scanCode);
SDL_FileDialogType GetSDLFileDialogType(Core::FileDialogType type);
}
