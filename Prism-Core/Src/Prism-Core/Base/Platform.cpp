#include "pcpch.h"
#include "Platform.h"

namespace Prism::Core
{
Platform& Platform::Get()
{
	return StaticPointerSingleton<Platform>::Get();
}

void Platform::Destroy()
{
	StaticPointerSingleton<Platform>::Destroy();
}

void Platform::TryDestroy()
{
	StaticPointerSingleton<Platform>::TryDestroy();
}
}
