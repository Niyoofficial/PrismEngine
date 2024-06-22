#pragma once

#define PE_ENABLE_ASSERTS

// Enable asserts break in debug build only
#ifdef PE_BUILD_DEBUG
	#define PE_ENABLE_ASSERTS_BREAK
#else
	#define PE_ASSERT_BREAK_INSTRUCTION
#endif

#ifdef PE_PLATFORM_WINDOWS
	#define PE_DEBUG_BREAK_INSTRUCTION do { __debugbreak(); } while (false)
#else
	// TODO
	#define PE_DEBUG_BREAK_INSTRUCTION
#endif

#ifdef PE_ENABLE_ASSERTS_BREAK
	#define PE_ASSERT_BREAK_INSTRUCTION PE_DEBUG_BREAK_INSTRUCTION
#else
	#define PE_ASSERT_BREAK_INSTRUCTION
#endif

#ifdef PE_ENABLE_ASSERTS
	#define PE_ASSERT(expr, ...) \
		do { if (!(expr)) { Prism::Log::PrintAssertMessage(__FILE__, __LINE__, SPDLOG_FUNCTION __VA_OPT__(, __VA_ARGS__)); PE_ASSERT_BREAK_INSTRUCTION; }} while (0)
	#define PE_ASSERT_NO_ENTRY(...) PE_ASSERT(false, "No entry assert failed!" __VA_OPT__(" - {}", __VA_ARGS__))
#else
	#define PE_ASSERT(expr, ...) do { expr } while (0)
	#define PE_ASSERT_NO_ENTRY()
#endif
