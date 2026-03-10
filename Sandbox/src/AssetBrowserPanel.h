#pragma once
#include <filesystem>
#include <stack>

#include "Prism/AssetManagement/TextureAsset.h"
#include "Prism/Utilities/Duration.h"
#include "Prism/UI/PrismImGui.h"

namespace Prism
{
class AssetBrowserPanel
{
public:
	AssetBrowserPanel();

	void UpdateImGui(Duration delta);

private:
	void RenderHeader();
	void RenderSidePanel();
	void RenderBody(bool grid);

	void DrawContextMenuItems(const std::filesystem::path& context, bool isDir);

	std::pair<bool, uint32_t> DirectoryTreeViewRecursive(const std::filesystem::path& path, uint32_t* count, int* selectionMask, ImGuiTreeNodeFlags flags);
	void UpdateDirectoryEntries(const std::filesystem::path& directory);
	void Refresh() { UpdateDirectoryEntries(m_currentDirectory); }

private:
	struct File
	{
		std::string name;
		std::string filepath;
		std::string extension;
		std::fs::directory_entry directoryEntry;
		//Ref<Texture2D> Thumbnail = nullptr;
		bool isDirectory = false;

		//FileType Type;
		//std::string_view fileTypeString;
		//ImVec4 fileTypeIndicatorColor;
	};

	std::filesystem::path m_assetsDirectory;
	std::filesystem::path m_currentDirectory;
	std::stack<std::filesystem::path> m_backStack;
	std::vector<File> m_directoryEntries;
	uint32_t m_currentlyVisibleItemsTreeView = 0;
	float m_thumbnailSize = 128.0f;
	ImGuiTextFilter m_filter;
	float m_elapsedTime = 0.0f;

	Ref<TextureAsset> m_folderIcon;
	Ref<TextureAsset> m_textureIcon;
};
}
