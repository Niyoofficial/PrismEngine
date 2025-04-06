#pragma once
#include "Prism/Render/RenderTypes.h"

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

class Swapchain : public RefCounted
{
public:
	static Swapchain* Create(Core::Window* window, SwapchainDesc swapchainDesc);

	virtual void Present() = 0;
	virtual void Resize() = 0;

	virtual SwapchainDesc GetSwapchainDesc() const = 0;

	virtual class TextureView* GetBackBufferRTV(int32_t index) const = 0;
	virtual TextureView* GetCurrentBackBufferRTV() const = 0;
	virtual int32_t GetCurrentBackBufferIndex() const = 0;
};
}
