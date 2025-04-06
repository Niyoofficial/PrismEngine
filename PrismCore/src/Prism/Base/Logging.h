#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "Prism/Utilities/StringUtils.h"

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

	template<typename T, typename... Args>
	void Log(LogVerbosity verbosity, T&& string, Args&&... args);

protected:
	explicit LogCategory(const std::string& logName);

	static spdlog::level::level_enum PrismVerbosityToSpdlog(LogVerbosity verbosity);

private:
	std::string m_name;
	std::unique_ptr<spdlog::logger> m_logger;
};

class ErrorLogger
{
public:
	ErrorLogger();

	void Init();

	template<typename T, typename... Args>
	void Log(const char* filename, int32_t line, const char* function,
			 T&& string, Args&&... args);

private:
	std::unique_ptr<spdlog::logger> m_logger;
};

class LogsRegistry final
{
public:
	static LogsRegistry& Get();
	static void DestroyRegistry();

	LogsRegistry();

	void AddCategoryToRegistry(LogCategory* category);
	const std::vector<LogCategory*>& GetRegisteredCategories() { return m_categories; }
	void RegisterErrorLogger(ErrorLogger* errorLogger);
	ErrorLogger* GetErrorLogger() const;

	std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> GetStdOutSink() const { return m_stdOutSink; }
	std::shared_ptr<spdlog::sinks::stderr_color_sink_mt> GetStdErrSink() const { return m_stdErrSink; }
	std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> GetFileSink() const { return m_fileSink; }

private:
	std::vector<LogCategory*> m_categories;
	ErrorLogger* m_errorLogger = nullptr;

	// Sinks
	std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> m_stdOutSink;
	std::shared_ptr<spdlog::sinks::stderr_color_sink_mt> m_stdErrSink;
	std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> m_fileSink;
};

template<typename T>
auto ConvertToString(T&& arg)
{
	return std::forward<T>(arg);
}

std::string ConvertToString(const wchar_t* arg);
std::string ConvertToString(const std::wstring& arg);
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
// TODO: Maybe remove fmt::runtime's if possible

template<typename T, typename... Args>
void LogCategory::Log(LogVerbosity verbosity, T&& string, Args&&... args)
{
	m_logger->log(PrismVerbosityToSpdlog(verbosity),
				  fmt::runtime(ConvertToString(std::forward<T>(string))),
				  ConvertToString(std::forward<Args>(args))...);
}

template<typename T, typename... Args>
void ErrorLogger::Log(const char* filename, int32_t line, const char* function,
					  T&& string, Args&&... args)
{
	spdlog::source_loc sourceLoc(filename, line, function);
	m_logger->log(sourceLoc, spdlog::level::critical,
				  fmt::runtime(ConvertToString(std::forward<T>(string))),
				  ConvertToString(std::forward<Args>(args))...);
}

extern ErrorLogger g_errorLogger;

void PrintAssertMessage(const char* filename, int32_t line, const char* function);

template<typename T, typename... Args>
void PrintAssertMessage(const char* filename, int32_t line, const char* function,
						T&& string, Args&&... args)
{
	LogsRegistry::Get().GetErrorLogger()->Log(filename, line, function, "Assertion Failed: {0}",
											  fmt::format(fmt::runtime(ConvertToString(std::forward<T>(string))),
														  ConvertToString(std::forward<Args>(args))...));
}
}
