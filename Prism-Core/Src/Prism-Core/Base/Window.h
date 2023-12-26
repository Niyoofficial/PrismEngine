#pragma once

#include "Prism-Core/Base/Base.h"

namespace Prism::Core
{
struct WindowParameters
{
public:
	std::string windowTitle;
	glm::float2 windowSize;
	bool fullscreen = false;
};

class Window
{
public:
	static Window* Create();

	virtual ~Window() = default;

	virtual void Init(const WindowParameters& params) = 0;
};
}
