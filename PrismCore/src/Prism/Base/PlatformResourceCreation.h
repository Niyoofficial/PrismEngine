#pragma once
#include "Prism/Base/Window.h"

namespace Prism::Core::Private
{
Ref<Window> CreateWindow(const WindowDesc& windowDesc, const Render::SwapchainDesc& swapchainDesc);
class Platform* CreatePlatform();
}
