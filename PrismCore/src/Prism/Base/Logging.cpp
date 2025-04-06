#include "pcpch.h"
#include "Logging.h"

#include "Prism/Utilities/LazySingleton.h"


namespace Prism::Log
{
ErrorLogger g_errorLogger;

void InitLog()
{
	g_errorLogger.Init();

	for (LogCategory* category : LogsRegistry::Get().GetRegisteredCategories())
		category->Init();
}

void ShutdownLog()
{
	LogsRegistry::DestroyRegistry();
}

LogsRegistry& LogsRegistry::Get()
{
	return LazySingleton<LogsRegistry>::Get();
}

void LogsRegistry::DestroyRegistry()
{
	LazySingleton<LogsRegistry>::Destroy();
}

LogsRegistry::LogsRegistry()
	: m_stdOutSink(std::make_shared<spdlog::sinks::stdout_color_sink_mt>()),
	  m_stdErrSink(std::make_shared<spdlog::sinks::stderr_color_sink_mt>()),
	  m_fileSink(std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/PrismLog.log", -1, 10, true))
{
	/*auto tp = std::chrono::system_clock::now();
	time_t tnow = std::chrono::system_clock::to_time_t(tp);
	tm tm;
	localtime_s(&tm, &tnow);
	auto logname = fmt::format(SPDLOG_FMT_STRING(SPDLOG_FILENAME_T("{}_{:04d}-{:02d}-{:02d}{}")),
							   "Logs/PrismLog", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, ".log");

	m_fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logname, -1, 10, true);*/
}

void LogsRegistry::AddCategoryToRegistry(LogCategory* category)
{
	m_categories.push_back(category);
}

void LogsRegistry::RegisterErrorLogger(ErrorLogger* errorLogger)
{
	PE_ASSERT(errorLogger);
	m_errorLogger = errorLogger;
}

ErrorLogger* LogsRegistry::GetErrorLogger() const
{
	PE_ASSERT(m_errorLogger);
	return m_errorLogger;
}

std::string ConvertToString(const wchar_t* arg)
{
	return WStringToString(arg);
}

std::string ConvertToString(const std::wstring& arg)
{
	return ConvertToString(arg.c_str());
}

ErrorLogger::ErrorLogger()
{
	LogsRegistry::Get().RegisterErrorLogger(this);
}

void ErrorLogger::Init()
{
	std::array<spdlog::sink_ptr, 3> sinks = {
		LogsRegistry::Get().GetStdErrSink(),
		LogsRegistry::Get().GetStdOutSink(),
		LogsRegistry::Get().GetFileSink()
	};
	m_logger = std::make_unique<spdlog::logger>("ErrorLog", sinks.begin(), sinks.end());
	m_logger->set_pattern("%^[%T] %v [File: %g] [Line: %#] [Func: %!()]%$");
	m_logger->set_level(spdlog::level::critical);
}

void LogCategory::Init()
{
	std::array<spdlog::sink_ptr, 2> sinks = {
		LogsRegistry::Get().GetStdOutSink(),
		LogsRegistry::Get().GetFileSink()
	};
	m_logger = std::make_unique<spdlog::logger>(m_name, sinks.begin(), sinks.end());
	m_logger->set_pattern("%^[%T] [%l] %n: %v%$");
	m_logger->set_level(spdlog::level::trace);
}

LogCategory::LogCategory(const std::string& logName)
	: m_name(logName)
{
	LogsRegistry::Get().AddCategoryToRegistry(this);
}

spdlog::level::level_enum LogCategory::PrismVerbosityToSpdlog(LogVerbosity verbosity)
{
	spdlog::level::level_enum spdlogVerbosity = spdlog::level::trace;
	switch (verbosity)
	{
	case LogVerbosity::Trace:
		spdlogVerbosity = spdlog::level::trace;
		break;
	case LogVerbosity::Info:
		spdlogVerbosity = spdlog::level::info;
		break;
	case LogVerbosity::Warn:
		spdlogVerbosity = spdlog::level::warn;
		break;
	case LogVerbosity::Error:
		spdlogVerbosity = spdlog::level::err;
		break;
	case LogVerbosity::Critical:
		spdlogVerbosity = spdlog::level::critical;
		break;
	}

	return spdlogVerbosity;
}

void PrintAssertMessage(const char* filename, int32_t line, const char* function)
{
	g_errorLogger.Log(filename, line, function, "Assertion Failed!");
}
}
