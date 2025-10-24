#pragma once

#include <variant>
#include <functional>

#include "AppEvents.h"
#include "Prism/Utilities/StaticSingleton.h"

namespace Prism
{
struct Duration;
}

namespace Prism::Core
{
class Window;

struct DisplayInfo
{
	uint32_t displayID;				/**< the display this mode is associated with */
	int32_t width;                  /**< width */
	int32_t height;                 /**< height */
	float pixelDensity;				/**< scale converting size to pixels (e.g. a 1920x1080 mode with 2.0 scale would have 3840x2160 pixels) */
	float refreshRate;				/**< refresh rate (or 0.0f for unspecified) */
	int32_t refreshRateNumerator;   /**< precise refresh rate numerator (or 0 for unspecified) */
	int32_t refreshRateDenominator; /**< precise refresh rate denominator */
};

/*
 * An entry for filters for file dialogs.
 *
 * `name` is a user-readable label for the filter (for example, "Office Document", "3D Mesh").
 *
 * `pattern` is a semicolon-separated list of file extensions (for example,
 * "doc;docx"). File extensions may only contain alphanumeric characters,
 * hyphens, underscores and periods. Alternatively, the whole string can be a
 * single asterisk ("*"), which serves as an "All files" filter.
 */
struct DialogFileFilter
{
	const char* name = nullptr;
	const char* pattern = nullptr;
};

class Platform : public StaticPointerSingleton<Platform>
{
public:
	static void Create();
	static void Destroy();
	static void TryDestroy();
	static Platform& Get();


	virtual ~Platform() = default;

	virtual void PumpEvents() = 0;
	template<typename T>
	void AddAppEventCallback(std::function<void(AppEvent)>&& callback)
	{
		static_assert(std::is_constructible_v<AppEvent, T>, "Provided type is not included in AppEvent variant");
		m_eventCallbacks[typeid(T).hash_code()].push_back(std::forward<std::function<void(AppEvent)>>(callback));
	}

	virtual bool IsKeyPressed(KeyCode keyCode) = 0;

	virtual void SetMouseRelativeMode(Window* window, bool bRelativeMode) = 0;

	virtual Duration GetApplicationTime() = 0;
	virtual uint64_t GetPerformanceTicksPerSecond() = 0;

	virtual DisplayInfo GetDisplayInfo(uint32_t displayID) = 0;
	virtual uint32_t GetPrimaryDisplayID() = 0;

	virtual void OpenFileDialog(const std::function<void(std::vector<std::string>, int32_t)>& callback,
								Window* window = nullptr, const std::vector<DialogFileFilter>& filters = {},
								const std::string& defaultLocation = {}, bool allowMany = false) = 0;


	virtual void InitializeImGuiPlatform(Window* window) = 0;
	virtual void ShutdownImGuiPlatform() = 0;

	virtual void ImGuiNewFrame() = 0;

protected:
	std::unordered_map<size_t, std::vector<std::function<void(AppEvent)>>> m_eventCallbacks;
};
}
