#pragma once

namespace Prism::Core
{
class Paths
{
public:
	static Paths& Get();

	std::fs::path GetEngineDir() const;
	std::fs::path GetProjectDir() const;

	std::fs::path GetEngineAssetsDir() const;
	std::fs::path GetProjectAssetsDir() const;

	std::fs::path GetIntermediateDir() const;
	std::fs::path GetLogsDir() const;
};
}
