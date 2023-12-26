#include "pcpch.h"
#include "Prism-Core/Base/Application.h"
#include "Prism-Core/Base/Base.h"


#ifdef PE_PLATFORM_WINDOWS

extern Prism::Core::Application* CreateApplication(int32_t argc, char** argv);

int32_t main(int32_t argc, char** argv)
{
	using namespace Prism::Core;

	InitializeCore();

	Application* app = CreateApplication(argc, argv);
	app->Run();

	delete app;

	ShutdownCore();

	return 0;
}

#endif
