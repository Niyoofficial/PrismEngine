#pragma once
#include "Prism-Core/Render/RenderTypes.h"

namespace Prism::Core
{
class Window;
}

namespace Prism::Render
{
struct SwapchainDesc
{
	struct
	{
		int32_t numerator = 60;
		int32_t denominator = 1;
	} refreshRate;
	TextureFormat format = TextureFormat::RGBA8_UNorm;
	SampleDesc sampleDesc;
	int32_t bufferCount = 3;
};

class Swapchain
{
public:
	static Swapchain* Create(Core::Window* window, SwapchainDesc swapchainDesc);

	virtual void Resize() = 0;
};
}
