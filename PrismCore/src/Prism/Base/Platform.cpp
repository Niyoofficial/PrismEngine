#include "pcpch.h"
#include "Platform.h"

#include "Prism/Base/PlatformResourceCreation.h"

namespace Prism::Core
{
Platform& Platform::Get()
{
	return StaticPointerSingleton<Platform>::Get();
}

void Platform::Create()
{
	StaticPointerSingleton<Platform>::Create(Private::CreatePlatform());
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
