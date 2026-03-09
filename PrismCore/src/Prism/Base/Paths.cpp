#include "Paths.h"
#include <filesystem>

#include "Prism/Utilities/LazySingleton.h"

namespace Prism::Core
{
Paths& Paths::Get()
{
	return LazySingleton<Paths>::Get();
}

std::fs::path Paths::GetEngineDir() const
{
	return PREPROCESSOR_TO_STRING(PE_ENGINE_DIR);
}

std::fs::path Paths::GetProjectDir() const
{
	return std::filesystem::current_path();
}

std::fs::path Paths::GetEngineAssetsDir() const
{
	return GetEngineDir() / "assets";
}

std::fs::path Paths::GetProjectAssetsDir() const
{
	return GetProjectDir() / "assets";
}

std::fs::path Paths::GetIntermediateDir() const
{
	return GetProjectDir() / "int";
}

std::fs::path Paths::GetLogsDir() const
{
	return GetProjectDir() / "logs";
}
}
