#include "pcpch.h"
#include "Base.h"

namespace Prism::Core
{
void InitCore()
{
	Log::InitLog();
}

void ShutdownCore()
{
	Log::ShutdownLog();
}
}
