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

	virtual Duration GetApplicationTime() override;
	virtual uint64_t GetPerformanceTicksPerSecond() override;
};
}
