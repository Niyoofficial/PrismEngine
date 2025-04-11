#pragma once

namespace Prism::Core
{
class Paths
{
public:
	static Paths& Get();

	std::wstring GetEngineDir() const;
	std::wstring GetProjectDir() const;

	std::wstring GetIntermediateDir() const;
	std::wstring GetLogsDir() const;

	std::wstring MakePathRelative(std::wstring absPath, std::wstring basePath) const;
};
}
