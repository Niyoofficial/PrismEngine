#include "pcpch.h"
#include "Logging.h"

#include <spdlog/sinks/stdout_color_sinks.h>

#include "Prism-Core/Utilities/LazySingleton.h"

namespace Prism::Log
{
ErrorLogger g_errorLogger;

void InitLog()
{
	g_errorLogger.Init();

	for (LogCategory* category : LogCategoryRegistry::Get().GetRegisteredCategories())
		category->Init();
}

void ShutdownLog()
{
	LogCategoryRegistry::DestroyRegistry();
}

LogCategoryRegistry& LogCategoryRegistry::Get()
{
	return LazySingleton<LogCategoryRegistry>::Get();
}

void LogCategoryRegistry::DestroyRegistry()
{
	LazySingleton<LogCategoryRegistry>::Destroy();
}

void LogCategoryRegistry::AddCategoryToRegistry(LogCategory* category)
{
	m_categories.push_back(category);
}

ErrorLogger::ErrorLogger()
	: m_logger("ErrorLog", std::make_shared<spdlog::sinks::stderr_color_sink_mt>())
{
	m_logger.set_pattern("%^[%T] %v [File: %g] [Line: %#] [Func: %!()]%$");
	m_logger.set_level(spdlog::level::critical);
}

void ErrorLogger::Init()
{
	//m_logger = std::make_unique<spdlog::logger>("Assert", std::make_shared<spdlog::sinks::stderr_color_sink_mt>());
}

void LogCategory::Init()
{
	m_logger = std::make_unique<spdlog::logger>(m_name, std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
	m_logger->set_pattern("%^[%T] [%l] %n: %v%$");
	m_logger->set_level(spdlog::level::trace);
}

LogCategory::LogCategory(const std::string& logName)
	: m_name(logName)
{
	LogCategoryRegistry::Get().AddCategoryToRegistry(this);
}

spdlog::level::level_enum LogCategory::PrismVerbosityToSpdlog(LogVerbosity verbosity)
{
	spdlog::level::level_enum spdlogVerbosity = spdlog::level::level_enum::trace;
	switch (verbosity)
	{
	case LogVerbosity::Trace:
		spdlogVerbosity = spdlog::level::level_enum::trace;
		break;
	case LogVerbosity::Info:
		spdlogVerbosity = spdlog::level::level_enum::info;
		break;
	case LogVerbosity::Warn:
		spdlogVerbosity = spdlog::level::level_enum::warn;
		break;
	case LogVerbosity::Error:
		spdlogVerbosity = spdlog::level::level_enum::err;
		break;
	case LogVerbosity::Critical:
		spdlogVerbosity = spdlog::level::level_enum::critical;
		break;
	}

	return spdlogVerbosity;
}

void PrintAssertMessage(const char* filename, int32_t line, const char* function)
{
	g_errorLogger.Log(filename, line, function, "Assertion Failed!");
}
}
