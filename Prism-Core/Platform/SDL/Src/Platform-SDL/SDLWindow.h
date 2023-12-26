#pragma once
#include "Prism-Core/Base/Window.h"

namespace Prism::Platform::SDL
{
class SDLWindow : public Core::Window
{
public:
	virtual void Init(const Core::WindowParameters& params) override;
};
}
