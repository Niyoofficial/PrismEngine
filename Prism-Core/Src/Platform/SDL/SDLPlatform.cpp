#include "pcpch.h"
#include "SDLPlatform.h"

#include "Prism-Core/Utilities/Duration.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_timer.h"

namespace Prism::SDL
{
SDLPlatform::SDLPlatform()
{
	SDL_Init(SDL_INIT_EVERYTHING);
}

SDLPlatform::~SDLPlatform()
{
	SDL_Quit();
}

void SDLPlatform::PumpEvents()
{
	using namespace Core;
	SDL_Event sdlEvent;
	while (SDL_PollEvent(&sdlEvent))
	{
		auto executeCallbacks =
			[this]<typename T>(T event)
			{
				for (const auto& callback : m_eventCallbacks[typeid(T).hash_code()])
					callback(event);
			};

		switch (sdlEvent.type)
		{
		case SDL_EVENT_QUIT:
			{
				AppEvents::Quit event;
				executeCallbacks(event);
			}
			break;

		/* Window events */
		case SDL_EVENT_WINDOW_SHOWN:
			{
				AppEvents::WindowShown event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_WINDOW_HIDDEN:
			{
				AppEvents::WindowHidden event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_WINDOW_EXPOSED:
			{
				AppEvents::WindowExposed event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_WINDOW_MOVED:
			{
				AppEvents::WindowMoved event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_WINDOW_RESIZED:
			{
				AppEvents::WindowResized event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			{
				AppEvents::WindowPixelSizeChanged event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_WINDOW_MINIMIZED:
			{
				AppEvents::WindowMinimized event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_WINDOW_MAXIMIZED:
			{
				AppEvents::WindowMaximized event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_WINDOW_RESTORED:
			{
				AppEvents::WindowRestored event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_WINDOW_MOUSE_ENTER:
			{
				AppEvents::WindowMouseEnter event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_WINDOW_MOUSE_LEAVE:
			{
				AppEvents::WindowMouseLeave event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_WINDOW_FOCUS_GAINED:
			{
				AppEvents::WindowFocusGained event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_WINDOW_FOCUS_LOST:
			{
				AppEvents::WindowFocusLost event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			{
				AppEvents::WindowCloseRequested event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_WINDOW_TAKE_FOCUS:
			{
				AppEvents::WindowTakeFocus event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_WINDOW_HIT_TEST:
			{
				AppEvents::WindowHitTest event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_WINDOW_ICCPROF_CHANGED:
			{
				AppEvents::WindowIccProfChanged event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
			{
				AppEvents::WindowDisplayChanged event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
			{
				AppEvents::WindowDisplayScaleChanged event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_WINDOW_OCCLUDED:
			{
				AppEvents::WindowOccluded event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
			{
				AppEvents::WindowEnterFullscreen event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
			{
				AppEvents::WindowLeaveFullscreen event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_WINDOW_DESTROYED:
			{
				AppEvents::WindowDestroyed event;
				executeCallbacks(event);
			}
			break;

		/* Keyboard events */
		case SDL_EVENT_KEY_DOWN:
			{
				AppEvents::KeyDown event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_KEY_UP:
			{
				AppEvents::KeyUp event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_TEXT_EDITING:
			{
				AppEvents::TextEditing event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_TEXT_INPUT:
			{
				AppEvents::TextInput event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_KEYMAP_CHANGED:
			{
				AppEvents::KeymapChanged event;
				executeCallbacks(event);
			}
			break;

		/* Mouse events */
		case SDL_EVENT_MOUSE_MOTION:
			{
				AppEvents::MouseMotion event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			{
				AppEvents::MouseButtonDown event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_MOUSE_BUTTON_UP:
			{
				AppEvents::MouseButtonUp event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_MOUSE_WHEEL:
			{
				AppEvents::MouseWheel event;
				executeCallbacks(event);
			}
			break;

		/* Gamepad events */
		case SDL_EVENT_GAMEPAD_AXIS_MOTION:
			{
				AppEvents::GamepadAxisMotion event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
			{
				AppEvents::GamepadButtonDown event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_GAMEPAD_BUTTON_UP:
			{
				AppEvents::GamepadButtonUp event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_GAMEPAD_ADDED:
			{
				AppEvents::GamepadAdded event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_GAMEPAD_REMOVED:
			{
				AppEvents::GamepadRemoved event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_GAMEPAD_REMAPPED:
			{
				AppEvents::GamepadRemapped event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_GAMEPAD_TOUCHPAD_DOWN:
			{
				AppEvents::GamepadTouchpadDown event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION:
			{
				AppEvents::GamepadTouchpadMotion event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_GAMEPAD_TOUCHPAD_UP:
			{
				AppEvents::GamepadTouchpadUp event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_GAMEPAD_SENSOR_UPDATE:
			{
				AppEvents::GamepadSensorUpdate event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_GAMEPAD_UPDATE_COMPLETE:
			{
				AppEvents::GamepadUpdateComplete event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_GAMEPAD_STEAM_HANDLE_UPDATED:
			{
				AppEvents::GamepadSteamHandleUpdated event;
				executeCallbacks(event);
			}
			break;

		/* Clipboard events */
		case SDL_EVENT_CLIPBOARD_UPDATE:
			{
				AppEvents::ClipboardUpdate event;
				executeCallbacks(event);
			}
			break;
		default:;
			//PE_ASSERT(false, "SDL event type not handled!");
		}
	}
}

Duration SDLPlatform::GetApplicationTime()
{
	return Duration(SDL_GetPerformanceCounter());
}

uint64_t SDLPlatform::GetPerformanceTicksPerSecond()
{
	return SDL_GetPerformanceFrequency();
}
}
