#include "Instrumentation.h"

#if ENABLE_SUPERLUMINAL_API
#include "Superluminal/PerformanceAPI.h"
#include "Superluminal/PerformanceAPI_capi.h"
#endif

namespace Prism::Instrumentation
{
void BeginEvent(const char* name, const char* data, uint32_t color)
{
#if ENABLE_SUPERLUMINAL_API
	PerformanceAPI_BeginEvent(name, data, color);
#endif
}

void EndEvent()
{
#if ENABLE_SUPERLUMINAL_API
	PerformanceAPI_EndEvent();
#endif
}

InstrumentationScope::InstrumentationScope(const char* name, const char* data, uint32_t color)
{
	BeginEvent(name, data, color);
}

InstrumentationScope::~InstrumentationScope()
{
	EndEvent();
}
}
