#pragma once

#define SPDLOG_COMPILED_LIB
#include "spdlog/spdlog.h"

namespace Prism::Log
{
void InitLog();
void ShutdownLog();

enum class LogVerbosity
{
	Trace,
	Info,
	Warn,
	Error,
	Critical
};

class LogCategory
{
public:
	LogCategory() = delete;

	void Init();

	template<typename... Args>
	void Log(LogVerbosity verbosity, spdlog::format_string_t<Args...> string, Args&&... args);

protected:
	explicit LogCategory(const std::string& logName);

	static spdlog::level::level_enum PrismVerbosityToSpdlog(LogVerbosity verbosity);

private:
	std::string m_name;
	std::unique_ptr<spdlog::logger> m_logger;
};

class LogCategoryRegistry final
{
public:
	static LogCategoryRegistry& Get();
	static void DestroyRegistry();

	void AddCategoryToRegistry(LogCategory* category);
	const std::vector<LogCategory*>& GetRegisteredCategories() { return m_categories; }

private:
	std::vector<LogCategory*> m_categories;
};

class ErrorLogger
{
public:
	ErrorLogger();

	void Init();

	template<typename... Args>
	void Log(const char* filename, int32_t line, const char* function,
			 spdlog::format_string_t<Args...> string, Args&&... args);

private:
	spdlog::logger m_logger;
};
}

#define DECLARE_LOG_CATEGORY(categoryName, displayedName)			\
	namespace Prism::Log::Generated									\
	{																\
	inline class categoryName##LogClass final : public LogCategory	\
	{																\
	public:															\
		categoryName##LogClass() : LogCategory(displayedName) {}	\
	} g_##categoryName##Log;										\
	} static_assert(true)

#define PE_LOG(logName, verbosity, ...)																		\
	do																										\
	{																										\
		::Prism::Log::Generated::g_##logName##Log.Log(::Prism::Log::LogVerbosity::verbosity, __VA_ARGS__);	\
	}																										\
	while (false)

// Template implementations
namespace Prism::Log
{
template<typename... Args>
void LogCategory::Log(LogVerbosity verbosity, spdlog::format_string_t<Args...> string, Args&&... args)
{
	m_logger->log(PrismVerbosityToSpdlog(verbosity), string, std::forward<Args>(args)...);
}

template<typename... Args>
void ErrorLogger::Log(const char* filename, int32_t line, const char* function,
					  spdlog::format_string_t<Args...> string, Args&&... args)
{
	spdlog::source_loc sourceLoc(filename, line, function);
	m_logger.log(sourceLoc, spdlog::level::level_enum::critical, string, std::forward<Args>(args)...);
}

extern ErrorLogger g_errorLogger;

template<typename... Args>
void PrintAssertMessage(const char* filename, int32_t line, const char* function)
{
	g_errorLogger.Log(filename, line, function, "Assertion Failed!");
}

template<typename... Args>
void PrintAssertMessage(const char* filename, int32_t line, const char* function,
						spdlog::format_string_t<Args...> string, Args&&... args)
{
	g_errorLogger.Log(filename, line, function, "Assertion Failed: {0}", fmt::format(string, std::forward<Args>(args)...));
}
}
