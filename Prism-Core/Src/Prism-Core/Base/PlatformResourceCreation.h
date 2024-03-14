#pragma once
#include "Prism-Core/Base/Window.h"

namespace Prism::Core::Private
{
Window* CreateWindow(const WindowDesc& windowDesc, const Render::SwapchainDesc& swapchainDesc);
class Platform* CreatePlatform();
}
