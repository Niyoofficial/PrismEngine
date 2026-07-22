#include "pcpch.h"
#include "SDLTypeConversions.h"
#include "SDLPlatform.h"

#include "SDL3/SDL_mouse.h"

namespace Prism::SDL
{
KeyCode GetPrismKeyCode(SDL_Keycode sdlKeyCode)
{
	return static_cast<KeyCode>(sdlKeyCode);
}

ScanCode GetPrismScanCode(SDL_Scancode sdlScanCode)
{
	return static_cast<ScanCode>(sdlScanCode);
}

KeyCode GetPrismKeyCodeFromMouseButton(uint8_t button)
{
	switch (button)
	{
	case SDL_BUTTON_LEFT:
		return KeyCode::LeftMouseButton;
	case SDL_BUTTON_MIDDLE:
		return KeyCode::MiddleMouseButton;
	case SDL_BUTTON_RIGHT:
		return KeyCode::RightMouseButton;
	case SDL_BUTTON_X1:
		return KeyCode::MouseButton4;
	case SDL_BUTTON_X2:
		return KeyCode::MouseButton5;
	}

	return KeyCode::Unknown;
}

SDL_Keycode GetSDLKeyCode(KeyCode keyCode)
{
	return static_cast<SDL_Keycode>(keyCode);
}

SDL_Scancode GetSDLScanCode(ScanCode scanCode)
{
	return static_cast<SDL_Scancode>(scanCode);
}

SDL_FileDialogType GetSDLFileDialogType(Core::FileDialogType type)
{
    switch (type)
    {
    case Core::FileDialogType::OpenFile:
        return SDL_FILEDIALOG_OPENFILE;
    case Core::FileDialogType::SaveFile:
        return SDL_FILEDIALOG_SAVEFILE;
    case Core::FileDialogType::OpenFolder:
        return SDL_FILEDIALOG_OPENFOLDER;
	default:
		PE_ASSERT_NO_ENTRY();
		return {};
    }
}
}
