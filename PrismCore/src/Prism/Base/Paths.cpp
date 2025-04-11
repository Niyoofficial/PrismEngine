#include "Paths.h"
#include <filesystem>

#include "Prism/Utilities/LazySingleton.h"

namespace Prism::Core
{
Paths& Paths::Get()
{
	return LazySingleton<Paths>::Get();
}

std::wstring Paths::GetEngineDir() const
{
	return PREPROCESSOR_TO_WIDE_STRING(PE_ENGINE_DIR);
}

std::wstring Paths::GetProjectDir() const
{
	return std::filesystem::current_path().generic_wstring();
}

std::wstring Paths::GetIntermediateDir() const
{
	return GetProjectDir() + L"/int";
}

std::wstring Paths::GetLogsDir() const
{
	return GetProjectDir() + L"/logs";
}

std::wstring Paths::MakePathRelative(std::wstring absPath, std::wstring basePath) const
{
	return std::filesystem::relative(absPath, basePath).generic_wstring();
}
}
