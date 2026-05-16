#include "pcpch.h"
#include "AssetRegistry.h"

#include <utility>

#include "Prism/AssetManagement/AssetType.h"
#include "Prism/Base/Application.h"
#include "Prism/Base/Paths.h"

namespace Prism
{
DirectoryIterator::DirectoryIterator(const AssetRegistry& registry, std::fs::path path)
	: m_impl(registry.GetAbsPath(path)), m_registry(&registry)
{
}

std::fs::directory_entry DirectoryIterator::operator*() const
{
	return std::fs::directory_entry(m_registry->GetRelPath(*m_impl));
}

DirectoryIterator& DirectoryIterator::operator++()
{
	++m_impl;
	return *this;
}

bool DirectoryIterator::operator==(const DirectoryIterator& rhs) const
{
	return m_impl == rhs.m_impl;
}

RecursiveDirectoryIterator::RecursiveDirectoryIterator(const AssetRegistry& registry, std::fs::path path)
	: m_impl(registry.GetAbsPath(path)), m_registry(&registry)
{
}

std::fs::directory_entry RecursiveDirectoryIterator::operator*() const
{
	return std::fs::directory_entry(m_registry->GetRelPath(*m_impl));
}

RecursiveDirectoryIterator& RecursiveDirectoryIterator::operator++()
{
	++m_impl;
	return *this;
}

bool RecursiveDirectoryIterator::operator==(const RecursiveDirectoryIterator& rhs) const
{
	return m_impl == rhs.m_impl;
}

AssetRegistry& AssetRegistry::Get()
{
	return Core::Application::Get().GetAssetRegistry();
}

std::vector<std::fs::path> AssetRegistry::FetchFilesOfType(AssetType* type, const std::fs::path& pathToSearch) const
{
	PE_ASSERT(type);

	std::fs::path path;
	if (pathToSearch.empty())
		path = Core::Paths::Get().GetProjectAssetsDir();
	else if (pathToSearch.is_relative())
		path = GetAbsPath(pathToSearch);
	else
		path = pathToSearch;

	std::vector<std::fs::path> output;
	auto extensions = type->GetAssociatedExtensions();

	for (auto& entry : std::fs::recursive_directory_iterator(path))
	{
		if (extensions.contains(entry.path().extension()))
			output.emplace_back(GetRelPath(entry));
	}

	return output;
}

DirectoryIterator AssetRegistry::CreateDirectoryIterator(std::fs::path path) const
{
	return {*this, std::move(path)};
}

DirectoryIterator AssetRegistry::CreateRecursiveDirectoryIterator(std::fs::path path) const
{
	return {*this, std::move(path)};
}

std::fs::path AssetRegistry::GetAbsPath(const std::fs::path& path) const
{
	auto normPath = NormalizePath(path);

	auto pathString = normPath.string();
	bool startsWithSlash = pathString[0] == (char)std::fs::path::preferred_separator;
	if (pathString.compare(startsWithSlash ? 1 : 0, 6, "engine") == 0)
		return Core::Paths::Get().GetEngineAssetsDir() / pathString.substr(startsWithSlash ? 7 : 6);
	else
		return Core::Paths::Get().GetProjectAssetsDir() / normPath;
}

std::fs::path AssetRegistry::GetRelPath(const std::fs::path& path) const
{
	std::string normPath = path.lexically_normal().string();
	if (auto projPath = Core::Paths::Get().GetProjectAssetsDir().string(); normPath.starts_with(projPath))
		return normPath.substr(projPath.size() + 1);
	else if (auto enginePath = Core::Paths::Get().GetEngineAssetsDir().string(); normPath.starts_with(enginePath))
		return std::fs::path("engine") / normPath.substr(enginePath.size());
	else
		PE_ASSERT_NO_ENTRY();

	return {};
}

bool AssetRegistry::IsEnginePath(const std::fs::path& path) const
{
	if (path.empty())
		return false;

	auto pathString = path.string();
	return pathString.compare(pathString[0] == '\\' || pathString[0] == '/' ? 1 : 0, 6, "engine") == 0;
}

bool AssetRegistry::IsProjectPath(const std::fs::path& path) const
{
	return !IsEnginePath(path);
}

bool AssetRegistry::IsDirectory(const std::fs::directory_entry& entry)
{
	return std::fs::is_directory(GetAbsPath(entry));
}

bool AssetRegistry::IsDirectory(const std::fs::path& path)
{
	return std::fs::is_directory(GetAbsPath(path));
}

std::fs::path AssetRegistry::NormalizePath(const std::fs::path& path) const
{
	if (path.is_absolute())
	{
		PE_ASSERT(false, "Path has to be relative, you can use engine/ prefix to refer to engine assets");
		return {};
	}

	return path.lexically_normal();
}
}
