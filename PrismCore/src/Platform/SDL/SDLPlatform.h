#pragma once
#include "Prism/Base/Platform.h"


namespace Prism::SDL
{
class SDLPlatform : public Core::Platform
{
public:
	SDLPlatform();
	virtual ~SDLPlatform() override;

	virtual void PumpEvents() override;

	virtual bool IsKeyPressed(KeyCode keyCode) override;

	virtual void SetMouseRelativeMode(Core::Window* window, bool bRelativeMode) override;

	virtual Duration GetApplicationTime() override;
	virtual uint64_t GetPerformanceTicksPerSecond() override;

	virtual Core::DisplayInfo GetDisplayInfo(uint32_t displayID) override;
	virtual uint32_t GetPrimaryDisplayID() override;

	virtual void OpenFileDialog(const std::function<void(std::vector<std::string>, int32_t)>& callback,
								Core::Window* window, const std::vector<Core::DialogFileFilter>& filters,
								const std::string& defaultLocation, bool allowMany) override;


	virtual void InitializeImGuiPlatform(Core::Window* window) override;
	virtual void ShutdownImGuiPlatform() override;

	virtual void ImGuiNewFrame() override;

private:
	uint64_t m_ticksStart = 0;

	bool initializedImGui = false;
};
}
