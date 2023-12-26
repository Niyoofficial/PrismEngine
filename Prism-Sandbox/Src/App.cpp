#include <cstdint>

#include "Prism-Core/Base/Application.h"

Prism::Core::Application* CreateApplication(int32_t argc, char** argv)
{
	return new Prism::Core::Application;
}
