#include "pcpch.h"
#include "Prism/Base/Application.h"
#include "Prism/Base/Base.h"														\


#ifdef PE_PLATFORM_WINDOWS

extern void CreateApplication(int32_t argc, char** argv);

int32_t main(int32_t argc, char** argv)
{
	using namespace Prism::Core;

	InitCore();

	CreateApplication(argc, argv);

	Application::Get().Run();

	Application::Destroy();

	ShutdownCore();

	return 0;
}

#endif
