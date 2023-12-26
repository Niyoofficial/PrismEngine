#include "pcpch.h"
#include "Base.h"

namespace Prism::Core
{
void InitializeCore()
{
	Log::InitLog();
}

void ShutdownCore()
{
	Log::ShutdownLog();
}
}
