#pragma once

#define MAKE_COLOR(R, G, B) ((((uint32_t)(R)) << 24) | (((uint32_t)(G)) << 16) | (((uint32_t)(B)) << 8) | (uint32_t)0xFF)

namespace Prism::Instrumentation
{
inline void BeginEvent(const char* name, const char* data = nullptr, uint32_t color = 0xFFFFFFFF);
inline void EndEvent();

struct InstrumentationScope
{
	explicit InstrumentationScope(const char* name, const char* data = nullptr, uint32_t color = 0xFFFFFFFF);
	~InstrumentationScope();

	InstrumentationScope(const InstrumentationScope&) = delete;
	InstrumentationScope(InstrumentationScope&&) = delete;
	bool operator=(const InstrumentationScope&) const = delete;
	bool operator=(InstrumentationScope&&) const = delete;
};
}

/**
 * 
 * @param name The name of the instrumentation event
 * @param data [optional] Additional runtime data for this event
 * @param color [optional] Color to be shown in the profiling tool
 */
#define SCOPED_INSTRUMENTATION(name, ...) const auto PREPROCESSOR_JOIN(instrumentationScope_, __LINE__) = ::Prism::Instrumentation::InstrumentationScope(name __VA_OPT__(,) __VA_ARGS__)
 /**
  * Scoped instrumentation event with __FUNCTION__ used as a name
  *
  * @param data [optional] Additional runtime data for this event
  * @param color [optional] Color to be shown in the profiling tool
  */
#define SCOPED_INSTRUMENTATION_FUNC(...) const auto PREPROCESSOR_JOIN(instrumentationScope_, __LINE__) = ::Prism::Instrumentation::InstrumentationScope(__FUNCTION__ __VA_OPT__(,) __VA_ARGS__)
