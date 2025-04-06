#include "pcpch.h"
#include "SDLPlatform.h"

#include "Platform/SDL/SDLTypeConversions.h"
#include "Prism/Utilities/Duration.h"
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_timer.h"

#include "imgui_impl_sdl3.h"
#include "Prism/Base/Window.h"


namespace Prism::SDL
{
SDLPlatform::SDLPlatform()
{
	bool success = SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD);
	PE_ASSERT(success, SDL_GetError());

	m_ticksStart = SDL_GetPerformanceCounter();
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
		if (initializedImGui)
			ImGui_ImplSDL3_ProcessEvent(&sdlEvent);

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
				// TODO: Pass window handle to the event, use SDL_GetWindowFromID
				AppEvents::WindowResized event = {
					.newSize = {sdlEvent.window.data1, sdlEvent.window.data2}
				};
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
				if (ImGui::GetIO().WantCaptureMouse)
					break;

				AppEvents::WindowMouseEnter event;
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_WINDOW_MOUSE_LEAVE:
			{
				if (ImGui::GetIO().WantCaptureMouse)
					break;

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
				if (ImGui::GetIO().WantCaptureKeyboard)
					break;

				AppEvents::KeyDown event = {
					.scanCode = (ScanCode)sdlEvent.key.scancode,
					.keyCode = (KeyCode)sdlEvent.key.key,
					.repeat = (bool)sdlEvent.key.repeat
				};
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_KEY_UP:
			{
				if (ImGui::GetIO().WantCaptureKeyboard)
					break;

				AppEvents::KeyUp event = {
					.scanCode = (ScanCode)sdlEvent.key.scancode,
					.keyCode = (KeyCode)sdlEvent.key.key,
					.repeat = (bool)sdlEvent.key.repeat
				};
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
				if (ImGui::GetIO().WantCaptureMouse)
					break;

				AppEvents::MouseMotion event = {
					.position = {sdlEvent.motion.x, sdlEvent.motion.y},
					.relPosition = {sdlEvent.motion.xrel, sdlEvent.motion.yrel}
				};
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			{
				if (ImGui::GetIO().WantCaptureMouse)
					break;

				AppEvents::MouseButtonDown event = {
					.keyCode = GetPrismKeyCodeFromMouseButton(sdlEvent.button.button),
					.position = {sdlEvent.button.x, sdlEvent.button.y},
					.doubleClick = sdlEvent.button.clicks == 2
				};
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_MOUSE_BUTTON_UP:
			{
				if (ImGui::GetIO().WantCaptureMouse)
					break;

				AppEvents::MouseButtonUp event = {
					.keyCode = GetPrismKeyCodeFromMouseButton(sdlEvent.button.button),
					.position = {sdlEvent.button.x, sdlEvent.button.y}
				};
				executeCallbacks(event);
			}
			break;
		case SDL_EVENT_MOUSE_WHEEL:
			{
				if (ImGui::GetIO().WantCaptureMouse)
					break;

				AppEvents::MouseWheel event = {
					.x = sdlEvent.wheel.x,
					.y = sdlEvent.wheel.y
				};
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

bool SDLPlatform::IsKeyPressed(KeyCode keyCode)
{
	int32_t numKeys = 0;
	const bool* keyboardState = SDL_GetKeyboardState(&numKeys);
	SDL_Scancode scanCode = SDL_GetScancodeFromKey(GetSDLKeyCode(keyCode), nullptr);

	PE_ASSERT(scanCode >= 0 && scanCode < numKeys);

	constexpr int32_t LAST_KEYBOARD_KEY = 290;
	if (scanCode > LAST_KEYBOARD_KEY)
		return SDL_GetMouseState(nullptr, nullptr) & (1 << (scanCode - (LAST_KEYBOARD_KEY + 1)));
	else
		return keyboardState[scanCode];
}

void SDLPlatform::SetMouseRelativeMode(Core::Window* window, bool bRelativeMode)
{
	SDL_SetWindowRelativeMouseMode(std::any_cast<SDL_Window*>(window->GetNativeWindow()), bRelativeMode);
}

Duration SDLPlatform::GetApplicationTime()
{
	return Duration(SDL_GetPerformanceCounter() - m_ticksStart);
}

uint64_t SDLPlatform::GetPerformanceTicksPerSecond()
{
	return SDL_GetPerformanceFrequency();
}

void SDLPlatform::InitializeImGuiPlatform(Core::Window* window)
{
	ImGui_ImplSDL3_InitForD3D(std::any_cast<SDL_Window*>(window->GetNativeWindow()));
	initializedImGui = true;
}

void SDLPlatform::ShutdownImGuiPlatform()
{
	if (initializedImGui)
	{
		ImGui_ImplSDL3_Shutdown();
		initializedImGui = false;
	}
}

void SDLPlatform::ImGuiNewFrame()
{
	ImGui_ImplSDL3_NewFrame();
}
}
