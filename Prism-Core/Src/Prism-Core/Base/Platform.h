#pragma once

#include <variant>
#include <functional>

#include "AppEvents.h"
#include "Prism-Core/Utilities/StaticSingleton.h"

namespace Prism
{
struct Duration;
}

namespace Prism::Core
{
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

	virtual void SetMouseRelativeMode(bool bRelativeMode) = 0;

	virtual Duration GetApplicationTime() = 0;
	virtual uint64_t GetPerformanceTicksPerSecond() = 0;

protected:
	std::unordered_map<size_t, std::vector<std::function<void(AppEvent)>>> m_eventCallbacks;
};
}
