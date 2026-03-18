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
	struct SelectionWithDeletion : public ImGuiSelectionBasicStorage
	{
		int ApplyDeletionPreLoop(ImGuiMultiSelectIO* multiselectIO, int itemsCount);

		template<typename ITEM_TYPE>
		void ApplyDeletionPostLoop(ImGuiMultiSelectIO* multiselectIO, ImVector<ITEM_TYPE>& items, int itemCurrIdxToSelect)
		{
			// Rewrite item list (delete items) + convert old selection index (before deletion) to new selection index (after selection).
			// If NavId was not part of selection, we will stay on same item.
			ImVector<ITEM_TYPE> newItems;
			newItems.reserve(items.Size - Size);
			int item_next_idx_to_select = -1;
			for (int idx = 0; idx < items.Size; idx++)
			{
				if (!Contains(GetStorageIdFromIndex(idx)))
					newItems.push_back(items[idx]);
				if (itemCurrIdxToSelect == idx)
					item_next_idx_to_select = newItems.Size - 1;
			}
			items.swap(newItems);

			// Update selection
			Clear();
			if (item_next_idx_to_select != -1 && multiselectIO->NavIdSelected)
				SetItemSelected(GetStorageIdFromIndex(item_next_idx_to_select), true);
		}
	};

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
	//void UpdateDirectoryEntries(const std::filesystem::path& directory);
	//void Refresh() { UpdateDirectoryEntries(m_currentDirectory); }

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
	std::filesystem::path m_currentDirectory;
	std::stack<std::filesystem::path> m_backStack;
	std::vector<File> m_directoryEntries;
	uint32_t m_currentlyVisibleItemsTreeView = 0;
	ImGuiTextFilter m_filter;
	float m_elapsedTime = 0.0f;
	std::fs::path m_selectedFile;
	std::vector<bool> m_sidePanelSelection;
	int32_t m_sidePanelLastSelected;
	SelectionWithDeletion m_selection;

	Ref<TextureAsset> m_folderIcon;
	Ref<TextureAsset> m_textureIcon;

	SidePanel m_sidePanel;
	std::vector<std::fs::path> m_activeDirs;
	std::map<ImGuiID, std::fs::path> m_activePaths;
};
}
