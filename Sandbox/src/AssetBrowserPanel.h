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
	class SidePanel
	{
	public:
		explicit SidePanel(std::fs::path assetsDir, std::function<void()> onSelectionChanged);

		void Render();
		void SetActiveDir(std::fs::path path);
		std::vector<std::fs::path> GetActiveDirs() const;

	private:
		void ForEachVisibleDir(std::fs::path path, const std::function<void(std::fs::path, ImGuiID)>& func);
		void ApplySelectionRequests(ImGuiMultiSelectIO* multiselectIO);
		void DrawDirectories(std::fs::path path, ImGuiID& id);
		bool IsOpen(ImGuiID id) const;

	private:
		std::fs::path m_assetsDir;
		ImGuiSelectionBasicStorage m_selectionStorage;
		std::unordered_map<ImGuiID, std::fs::path> m_activeDirs;
		std::function<void()> m_onSelectionChanged;
		std::vector<ImGuiID> m_dirsToOpen;
	};

public:
	AssetBrowserPanel();

	void UpdateImGui(Duration delta);

private:
	void RenderHeader();
	void RenderBody();

	ImGuiID UpdateDirectoryEntries(std::fs::path dir, ImGuiID id);
	void DrawContextMenuItems(const std::filesystem::path& context, bool isDir);

	void RenderSidePanelDirectory(const std::filesystem::path& path, int& currentIndex);
	std::pair<bool, uint32_t> DirectoryTreeViewRecursive(const std::filesystem::path& path, uint32_t* count, uint64_t* selectionMask, ImGuiTreeNodeFlags flags);

private:
	struct File
	{
		std::fs::path filename;
		std::fs::path filepath;
		std::fs::path extension;
		std::fs::directory_entry directoryEntry;
		bool isDirectory = false;
		bool selected = false;

		class AssetType* assetType = nullptr;
	};

	std::filesystem::path m_assetsDirectory;
	std::vector<File> m_directoryEntries;
	uint32_t m_currentlyVisibleItemsTreeView = 0;
	ImGuiTextFilter m_filter;
	float m_elapsedTime = 0.0f;
	std::fs::path m_selectedFile;
	std::vector<bool> m_sidePanelSelection;
	int32_t m_sidePanelLastSelected;
	ImGuiSelectionBasicStorage m_selection;

	Ref<TextureAsset> m_folderIcon;
	Ref<TextureAsset> m_textureIcon;

	SidePanel m_sidePanel;
	// Selected directories to display, copied over from the side panel each frame
	std::vector<std::fs::path> m_activeDirs;
	// All currently visible files and dirs
	std::map<ImGuiID, std::fs::path> m_displayedPaths;
};
}
