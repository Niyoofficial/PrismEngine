#pragma once

namespace Prism::Core {
class Application;
}

namespace Prism
{
class AssetRegistry;

class DirectoryIterator
{
public:
	DirectoryIterator() = default;
	DirectoryIterator(const AssetRegistry& registry, std::fs::path path);
	DirectoryIterator(const DirectoryIterator&) = default;
	DirectoryIterator(DirectoryIterator&&) = default;
	~DirectoryIterator() = default;

	DirectoryIterator& operator=(const DirectoryIterator&) = default;
	DirectoryIterator& operator=(DirectoryIterator&&) = default;

	std::fs::directory_entry operator*() const;
	DirectoryIterator& operator++();
	bool operator==(const DirectoryIterator& rhs) const;

private:
	std::fs::directory_iterator m_impl;
	const AssetRegistry* m_registry = nullptr;
};

inline DirectoryIterator begin(DirectoryIterator iter) noexcept
{
	return iter;
}

inline DirectoryIterator end(DirectoryIterator) noexcept
{
	return {};
}

class RecursiveDirectoryIterator
{
public:
	RecursiveDirectoryIterator() = default;
	RecursiveDirectoryIterator(const AssetRegistry& registry, std::fs::path path);
	RecursiveDirectoryIterator(const RecursiveDirectoryIterator&) = default;
	RecursiveDirectoryIterator(RecursiveDirectoryIterator&&) = default;
	~RecursiveDirectoryIterator() = default;

	RecursiveDirectoryIterator& operator=(const RecursiveDirectoryIterator&) = default;
	RecursiveDirectoryIterator& operator=(RecursiveDirectoryIterator&&) = default;

	std::fs::directory_entry operator*() const;
	RecursiveDirectoryIterator& operator++();
	bool operator==(const RecursiveDirectoryIterator& rhs) const;

private:
	std::fs::recursive_directory_iterator m_impl;
	const AssetRegistry* m_registry = nullptr;
};

inline RecursiveDirectoryIterator begin(RecursiveDirectoryIterator iter) noexcept
{
	return iter;
}

inline RecursiveDirectoryIterator end(RecursiveDirectoryIterator) noexcept
{
	return {};
}

class AssetRegistry
{
	friend Core::Application;

public:
	static AssetRegistry& Get();

	std::vector<std::fs::path> FetchFilesOfType(class AssetType* type, const std::fs::path& pathToSearch = "") const;

	DirectoryIterator CreateDirectoryIterator(std::fs::path path) const;
	DirectoryIterator CreateRecursiveDirectoryIterator(std::fs::path path) const;

	std::fs::path GetAbsPath(const std::fs::path& path) const;
	std::fs::path GetRelPath(const std::fs::path& path) const;
	bool IsEnginePath(const std::fs::path& path) const;
	bool IsProjectPath(const std::fs::path& path) const;

	bool IsDirectory(const std::fs::directory_entry& entry);
	bool IsDirectory(const std::fs::path& path);

protected:
	AssetRegistry() = default;

private:
	std::fs::path NormalizePath(const std::fs::path& path) const;
};
}
