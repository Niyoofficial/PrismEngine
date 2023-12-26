#pragma once

#include "Prism-Core/Base/Window.h"

namespace Prism::Core
{
class Application
{
public:
	void Run();

protected:
	void Init();

protected:
	std::unique_ptr<class Window> m_window;
};
}
