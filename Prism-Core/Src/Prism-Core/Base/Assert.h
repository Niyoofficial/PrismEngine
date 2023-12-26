#pragma once

#ifdef PE_PLATFORM_WINDOWS
#define PE_DEBUG_BREAK do { __debugbreak(); } while (false)
#else
// TODO
#define PE_DEBUG_BREAK
#endif

#ifdef PE_BUILD_DEBUG
	#define PE_ASSERT_BREAK PE_DEBUG_BREAK
#else
	#define PE_ASSERT_BREAK
#endif

#define PE_ENABLE_ASSERTS

#ifdef PE_ENABLE_ASSERTS
	#define PE_ASSERT(condition, ...) \
		do { if (!condition) { Prism::Log::PrintAssertMessage(__FILE__, __LINE__, SPDLOG_FUNCTION __VA_OPT__(, __VA_ARGS__)); PE_ASSERT_BREAK; }} while (false)
#else
	#define PE_ASSERT(condition, ...)
#endif
