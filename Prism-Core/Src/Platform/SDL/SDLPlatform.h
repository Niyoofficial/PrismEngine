#pragma once
#include "Prism-Core/Base/Platform.h"


namespace Prism::SDL
{
class SDLPlatform : public Core::Platform
{
public:
	SDLPlatform();
	virtual ~SDLPlatform() override;

	virtual void PumpEvents() override;

	virtual bool IsKeyPressed(KeyCode keyCode) override;

	virtual void SetMouseRelativeMode(bool bRelativeMode) override;

	virtual Duration GetApplicationTime() override;
	virtual uint64_t GetPerformanceTicksPerSecond() override;
};
}
