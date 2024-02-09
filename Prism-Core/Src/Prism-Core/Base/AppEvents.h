#pragma once

namespace Prism::Core
{
namespace AppEvents
{
	struct Quit
	{
		
	};

	/* Window events */
	struct WindowShown {};
	struct WindowHidden {};
	struct WindowExposed {};
	struct WindowMoved {};
	struct WindowResized {};
	struct WindowPixelSizeChanged {};
	struct WindowMinimized {};
	struct WindowMaximized {};
	struct WindowRestored {};
	struct WindowMouseEnter {};
	struct WindowMouseLeave {};
	struct WindowFocusGained {};
	struct WindowFocusLost {};
	struct WindowCloseRequested {};
	struct WindowTakeFocus {};
	struct WindowHitTest {};
	struct WindowIccProfChanged {};
	struct WindowDisplayChanged {};
	struct WindowDisplayScaleChanged {};
	struct WindowOccluded {};
	struct WindowEnterFullscreen {};
	struct WindowLeaveFullscreen {};
	struct WindowDestroyed {};

	/* Keyboard events */
	struct KeyDown {};
	struct KeyUp {};
	struct TextEditing {};
	struct TextInput {};
	struct KeymapChanged {};

	/* Mouse events */
	struct MouseMotion {};
	struct MouseButtonDown {};
	struct MouseButtonUp {};
	struct MouseWheel {};

	/* Gamepad events */
	struct GamepadAxisMotion {};
	struct GamepadButtonDown {};
	struct GamepadButtonUp {};
	struct GamepadAdded {};
	struct GamepadRemoved {};
	struct GamepadRemapped {};
	struct GamepadTouchpadDown {};
	struct GamepadTouchpadMotion {};
	struct GamepadTouchpadUp {};
	struct GamepadSensorUpdate {};
	struct GamepadUpdateComplete {};
	struct GamepadSteamHandleUpdated {};

	/* Clipboard events */
	struct ClipboardUpdate {};
}

using AppEvent = std::variant<
	AppEvents::Quit,

	/* Window events */
	AppEvents::WindowShown,
	AppEvents::WindowHidden,
	AppEvents::WindowExposed,
	AppEvents::WindowMoved,
	AppEvents::WindowResized,
	AppEvents::WindowPixelSizeChanged,
	AppEvents::WindowMinimized,
	AppEvents::WindowMaximized,
	AppEvents::WindowRestored,
	AppEvents::WindowMouseEnter,
	AppEvents::WindowMouseLeave,
	AppEvents::WindowFocusGained,
	AppEvents::WindowFocusLost,
	AppEvents::WindowCloseRequested,
	AppEvents::WindowTakeFocus,
	AppEvents::WindowHitTest,
	AppEvents::WindowIccProfChanged,
	AppEvents::WindowDisplayChanged,
	AppEvents::WindowDisplayScaleChanged,
	AppEvents::WindowOccluded,
	AppEvents::WindowEnterFullscreen,
	AppEvents::WindowLeaveFullscreen,
	AppEvents::WindowDestroyed,

	/* Keyboard events */
	AppEvents::KeyDown,
	AppEvents::KeyUp,
	AppEvents::TextEditing,
	AppEvents::TextInput,
	AppEvents::KeymapChanged,

	/* Mouse events */
	AppEvents::MouseMotion,
	AppEvents::MouseButtonDown,
	AppEvents::MouseButtonUp,
	AppEvents::MouseWheel,

	/* Gamepad events */
	AppEvents::GamepadAxisMotion,
	AppEvents::GamepadButtonDown,
	AppEvents::GamepadButtonUp,
	AppEvents::GamepadAdded,
	AppEvents::GamepadRemoved,
	AppEvents::GamepadRemapped,
	AppEvents::GamepadTouchpadDown,
	AppEvents::GamepadTouchpadMotion,
	AppEvents::GamepadTouchpadUp,
	AppEvents::GamepadSensorUpdate,
	AppEvents::GamepadUpdateComplete,
	AppEvents::GamepadSteamHandleUpdated,

	/* Clipboard events */
	AppEvents::ClipboardUpdate
	>;
}
