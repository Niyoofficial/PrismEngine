#include "AssetBrowserPanel.h"

#include <algorithm>
#include <filesystem>

#include "EditorTheme.h"
#include "Sandbox.h"
#include "Prism/AssetManagement/AssetType.h"
#include "Prism/AssetManagement/TextureAsset.h"
#include "Prism/Base/Paths.h"

namespace Prism
{
int AssetBrowserPanel::SelectionWithDeletion::ApplyDeletionPreLoop(ImGuiMultiSelectIO* multiselectIO, int itemsCount)
{
	if (Size == 0)
		return -1;

	// If focused item is not selected...
	const int focused_idx = (int)multiselectIO->NavIdItem;  // Index of currently focused item
	if (multiselectIO->NavIdSelected == false)  // This is merely a shortcut, == Contains(adapter->IndexToStorage(items, focused_idx))
	{
		multiselectIO->RangeSrcReset = true;    // Request to recover RangeSrc from NavId next frame. Would be ok to reset even when NavIdSelected==true, but it would take an extra frame to recover RangeSrc when deleting a selected item.
		return focused_idx;             // Request to focus same item after deletion.
	}

	// If focused item is selected: land on first unselected item after focused item.
	for (int idx = focused_idx + 1; idx < itemsCount; idx++)
		if (!Contains(GetStorageIdFromIndex(idx)))
			return idx;

	// If focused item is selected: otherwise return last unselected item before focused item.
	for (int idx = std::min(focused_idx, itemsCount) - 1; idx >= 0; idx--)
		if (!Contains(GetStorageIdFromIndex(idx)))
			return idx;

	return -1;
}

AssetBrowserPanel::SidePanel::SidePanel(std::fs::path assetsDir, std::function<void()> onSelectionChanged)
	: m_assetsDir(assetsDir), m_onSelectionChanged(onSelectionChanged)
{
	m_selectionStorage.SetItemSelected(0, true);
	m_activeDirs[0] = assetsDir;
}

void AssetBrowserPanel::SidePanel::Render()
{
	for (auto dir : m_dirsToOpen)
		ImGui::GetStateStorage()->SetBool(dir, true);
	m_dirsToOpen.clear();

	ImGuiMultiSelectIO* multiselectIO = ImGui::BeginMultiSelect(ImGuiMultiSelectFlags_None, m_selectionStorage.Size);

	ApplySelectionRequests(multiselectIO);

	ImGuiID id = 0;
	DrawDirectories(m_assetsDir, id);

	multiselectIO = ImGui::EndMultiSelect();

	ApplySelectionRequests(multiselectIO);
}

void AssetBrowserPanel::SidePanel::SetActiveDir(std::fs::path path)
{
	m_activeDirs.clear();
	
	std::function<bool(std::fs::path, ImGuiID&)> forEachDir =
		[&forEachDir, pathToCompare = path, this](std::fs::path path, ImGuiID& id)
		{
			ImGuiID currID = id;
			if (pathToCompare == path)
			{
				m_activeDirs[id] = path;
				m_selectionStorage.Clear();
				m_selectionStorage.SetItemSelected(id, true);
				return true;
			}

			++id;
			for (auto& entry : std::fs::directory_iterator(path))
			{
				if (!entry.is_directory())
					continue;

				if (forEachDir(entry, id))
				{
					m_dirsToOpen.push_back(currID);
					return true;
				}
			}

			return false;
		};

	ImGuiID id = 0;
	forEachDir(m_assetsDir, id);
}

std::vector<std::fs::path> AssetBrowserPanel::SidePanel::GetActiveDirs() const
{
	std::vector<std::fs::path> paths;
	for (auto& path : m_activeDirs | std::views::values)
		paths.push_back(path);
	return paths;
}

void AssetBrowserPanel::SidePanel::ForEachVisibleDir(std::fs::path path, const std::function<void(std::fs::path, ImGuiID)>& func)
{
	std::function<void(std::fs::path, const std::function<void(std::fs::path, ImGuiID)>&, ImGuiID&)> forEachInternal =
		[&forEachInternal, this](std::fs::path path, const std::function<void(std::fs::path, ImGuiID)>& func, ImGuiID& id)
		{
			func(path, id);

			bool open = IsOpen(id);
			++id;
			if (open)
			{
				for (auto& entry : std::fs::directory_iterator(path))
				{
					if (!entry.is_directory())
						continue;

					forEachInternal(entry, func, id);
				}
			}
			else
			{
				for (auto& entry : std::fs::recursive_directory_iterator(path))
				{
					if (entry.is_directory())
						++id;
				}
			}
		};

	ImGuiID id = 0;
	forEachInternal(path, func, id);
}

void AssetBrowserPanel::SidePanel::ApplySelectionRequests(ImGuiMultiSelectIO* multiselectIO)
{
	for (auto& req : multiselectIO->Requests)
	{
		if (req.Type == ImGuiSelectionRequestType_SetAll)
		{
			if (req.Selected)
			{
				ForEachVisibleDir(m_assetsDir,
					[this](std::fs::path path, ImGuiID id)
					{
						m_selectionStorage.SetItemSelected(id, true);
						m_activeDirs[id] = path;
					});
			}
			else
			{
				m_selectionStorage.Clear();
				m_activeDirs.clear();
			}
		}
		else if (req.Type == ImGuiSelectionRequestType_SetRange)
		{
			ImGuiID first = req.RangeFirstItem;
			ImGuiID last = req.RangeLastItem;

			ForEachVisibleDir(m_assetsDir,
				[first, last, &req, this](std::fs::path path, ImGuiID id)
				{
					if (id >= std::min(first, last) && id <= std::max(first, last))
					{
						m_selectionStorage.SetItemSelected(id, req.Selected);
						if (req.Selected)
							m_activeDirs[id] = path;
						else
							m_activeDirs.erase(id);
					}
				});
		}
	}

	if (multiselectIO->Requests.size() > 0)
		m_onSelectionChanged();
}

void AssetBrowserPanel::SidePanel::DrawDirectories(std::fs::path path, ImGuiID& id)
{
	if (!std::fs::is_directory(path))
		return;

	ImGuiTreeNodeFlags treeNodeFlags =
		ImGuiTreeNodeFlags_SpanFullWidth |
		ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_OpenOnDoubleClick |
		ImGuiTreeNodeFlags_NavLeftJumpsBackHere |
		ImGuiTreeNodeFlags_FramePadding;

	bool hasChildren = false;
	for (auto& entry : std::fs::directory_iterator(path))
	{
		if (entry.is_directory())
		{
			hasChildren = true;
			break;
		}
	}
	if (!hasChildren)
		treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;

	bool wasSelected = false;
	if (m_selectionStorage.Contains(id))
	{
		treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
		ImGui::PushStyleColor(ImGuiCol_Header, EditorTheme::s_headerSelectedColor);
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::s_headerSelectedColor);
		wasSelected = true;
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::s_headerHoveredColor);
	}

	ImGui::SetNextItemSelectionUserData(id);
	ImGui::SetNextItemStorageID(id);
	std::string nodeName = std::string(IsOpen(id) ? ICON_MDI_FOLDER_OPEN : ICON_MDI_FOLDER) + " " + path.filename().string();
	bool open = ImGui::TreeNodeEx(path.lexically_normal().string().c_str(), treeNodeFlags, "%s", nodeName.c_str());
	ImGui::PopStyleColor(wasSelected ? 2 : 1);
	if (open)
	{
		++id;
		for (auto& entry : std::fs::directory_iterator(path))
			DrawDirectories(entry, id);
		ImGui::TreePop();
	}
	else
	{
		if (ImGui::IsItemToggledOpen())
		{
			std::function<int32_t(ImGuiID&, int32_t)> closeAndUnselectChildren =
				[&closeAndUnselectChildren, path, this](ImGuiID& id, int32_t depth)
				{
					// Recursive close (the test for depth == 0 is because we call this on a node that was just closed!)
					int unselectedCount = m_selectionStorage.Contains(id) ? 1 : 0;
					if (depth == 0 || IsOpen(id))
					{
						ImGuiID childId = id;
						for (auto& entry : std::fs::directory_iterator(path))
						{
							if (entry.is_directory())
							{
								++childId;
								unselectedCount += closeAndUnselectChildren(childId, depth + 1);
							}
						}
						ImGui::GetStateStorage()->SetBool(id, false);
					}

					// Select root node if any of its child was selected, otherwise unselect
					m_selectionStorage.SetItemSelected(id, (depth == 0 && unselectedCount > 0));
					return unselectedCount;
				};

			ImGuiID idCopy = id;
			closeAndUnselectChildren(idCopy, 0);
		}

		++id;
		for (auto& entry : std::fs::recursive_directory_iterator(path))
		{
			if (entry.is_directory())
				++id;
		}
	}
}

bool AssetBrowserPanel::SidePanel::IsOpen(ImGuiID id) const
{
	return ImGui::GetStateStorage()->GetBool(id);
}

AssetBrowserPanel::AssetBrowserPanel()
	: m_assetsDirectory(Core::Paths::Get().GetProjectAssetsDir()), m_currentDirectory(m_assetsDirectory),
	  m_folderIcon(EditorApplication::Get().GetAssetManager().LoadAsset<TextureAsset>("engine/folder_icon.png")),
	  m_textureIcon(EditorApplication::Get().GetAssetManager().LoadAsset<TextureAsset>("engine/texture_icon.png")),
	  m_sidePanel(m_assetsDirectory, [this](){m_selection.Clear();})	
{
}

void AssetBrowserPanel::UpdateImGui(Duration delta)
{
	if (ImGui::Begin("Asset Browser", nullptr, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar))
	{
		RenderHeader();

		ImGui::Separator();

		ImVec2 availableRegion = ImGui::GetContentRegionAvail();
		if (ImGui::BeginTable("MainViewTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_ContextMenuInBody, availableRegion))
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			m_sidePanel.Render();
			ImGui::TableNextColumn();
			RenderBody();

			ImGui::EndTable();
		}
	}
	ImGui::End();
}

void AssetBrowserPanel::RenderHeader()
{
	float cursorPosX = ImGui::GetCursorPosX();
	m_filter.Draw("##asset_filter", ImGui::GetContentRegionAvail().x);
	if (!m_filter.IsActive())
	{
		ImGui::SameLine();
		ImGui::SetCursorPosX(cursorPosX + ImGui::GetFontSize() * 0.5f);
		static const char* searchString = ICON_MDI_MAGNIFY " Search...";
		static const char* searchStringEnd = searchString + strlen(searchString);
		ImGui::TextUnformatted(searchString, searchStringEnd);
	}

	ImGui::Spacing();
	ImGui::Spacing();

	// Back button
	{
		bool disabledBackButton = false;
		if (m_currentDirectory == m_assetsDirectory)
			disabledBackButton = true;

		if (disabledBackButton)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (ImGui::Button(ICON_MDI_ARROW_LEFT_CIRCLE))
		{
			m_backStack.push(m_currentDirectory);
			//UpdateDirectoryEntries(m_currentDirectory.parent_path(), TODO);
		}

		if (disabledBackButton)
		{
			ImGui::PopStyleVar();
		}
	}

	ImGui::SameLine();

	// Front button
	{
		bool disabledFrontButton = false;
		if (m_backStack.empty())
			disabledFrontButton = true;

		if (disabledFrontButton)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		if (ImGui::Button(ICON_MDI_ARROW_RIGHT_CIRCLE))
		{
			const auto& top = m_backStack.top();
			//UpdateDirectoryEntries(top, TODO);
			m_backStack.pop();
		}

		if (disabledFrontButton)
		{
			ImGui::PopStyleVar();
		}
	}

	ImGui::SameLine();

	constexpr const char* folderIcon = ICON_MDI_FOLDER;
	ImGui::TextUnformatted(folderIcon);

	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.0f, 0.0f, 0.0f, 0.0f });

	std::filesystem::path projectDir = m_assetsDirectory.parent_path();
	static std::filesystem::path absProjectDir = std::filesystem::absolute(m_assetsDirectory.parent_path());
	static std::string absProjectDirString = absProjectDir.string();
	static size_t absProjectDirLength = absProjectDirString.length();

	std::string currentDir = m_currentDirectory.string();
	const char* p = &currentDir[absProjectDirLength + 1];
	const std::filesystem::path currentDirectory = p;

	std::filesystem::path directoryToOpen;
	for (auto& path : currentDirectory)
	{
		projectDir /= path;
		ImGui::SameLine();
		if (ImGui::Button(path.filename().string().c_str()))
			directoryToOpen = projectDir;

		if (m_currentDirectory != projectDir)
		{
			ImGui::SameLine();
			constexpr const char* delimeter = "/";
			ImGui::TextUnformatted(delimeter, delimeter + 1);
		}
	}
	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar();

	//if (!directoryToOpen.empty())
	//	UpdateDirectoryEntries(m_assetsDirectory / directoryToOpen, TODO);
}

void AssetBrowserPanel::RenderSidePanelDirectory(const std::filesystem::path& path, int& currentIndex)
{
    // Only show directories
    if (!std::filesystem::is_directory(path))
        return;

    ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_FramePadding;

    // Selection state for this node
    bool isSelected = m_sidePanelSelection[currentIndex];

    ImGuiTreeNodeFlags nodeFlags = baseFlags;
    if (isSelected)
        nodeFlags |= ImGuiTreeNodeFlags_Selected;

    // Check if this node has children
    bool hasChild = false;
    for (auto& entry : std::filesystem::directory_iterator(path))
    {
        if (entry.is_directory())
        {
	        hasChild = true;
        	break;
        }
    }
    if (!hasChild)
        nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    // Render node
    std::string label = path.filename().string();
    if (label.empty())
		label = "Assets";
    bool open = ImGui::TreeNodeEx((void*)(intptr_t)currentIndex, nodeFlags, "%s", label.c_str());

    if (ImGui::IsItemClicked())
    {
        bool ctrl = ImGui::GetIO().KeyCtrl;
        bool shift = ImGui::GetIO().KeyShift;
        if (!ctrl && !shift)
        {
            // Single select
            std::fill(m_sidePanelSelection.begin(), m_sidePanelSelection.end(), false);
            m_sidePanelSelection[currentIndex] = true;
            m_sidePanelLastSelected = currentIndex;
        }
        else if (shift && m_sidePanelLastSelected >= 0)
        {
            // Range select
            int begin = std::min(m_sidePanelLastSelected, currentIndex);
            int end = std::max(m_sidePanelLastSelected, currentIndex);
            for (int i = begin; i <= end; ++i)
                m_sidePanelSelection[i] = true;
        }
        else if (ctrl)
        {
            // Toggle select
            m_sidePanelSelection[currentIndex] = !m_sidePanelSelection[currentIndex];
            m_sidePanelLastSelected = currentIndex;
        }
    }

    ++currentIndex;

    if (open && hasChild)
    {
        for (auto& entry : std::filesystem::directory_iterator(path))
        {
            if (entry.is_directory())
                RenderSidePanelDirectory(entry.path(), currentIndex);
        }
        ImGui::TreePop();
    }
}

void AssetBrowserPanel::RenderBody()
{
	if (ImGui::BeginChild("Assets", {}, ImGuiChildFlags_None, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
	{
		m_activeDirs = m_sidePanel.GetActiveDirs();
		m_activePaths.clear();

		UpdateDirectoryEntries(m_assetsDirectory, 0);

		std::vector<ImGuiID> listOfPathIDs;
		listOfPathIDs.reserve(m_activePaths.size());
		for (const auto& id : m_activePaths | std::views::keys)
			listOfPathIDs.push_back(id);

		constexpr float layoutItemSpacing = 10.f;
		constexpr int32_t iconHitSpacing = 10;
		constexpr float originalThumbnailSize = 140.f;

		float scaledThumbnailSize = originalThumbnailSize * ImGui::GetIO().FontGlobalScale;
		float scaledThumbnailSizeX = scaledThumbnailSize * 0.55f;

		float availWidth = ImGui::GetContentRegionAvail().x;

		availWidth += floorf(layoutItemSpacing * 0.5f);

		// Layout: calculate number of icon per line and number of lines
		constexpr float ITEM_PADDING = 4.f;
		glm::float2 layoutItemSize = { scaledThumbnailSizeX + ITEM_PADDING * 2, scaledThumbnailSize + ITEM_PADDING * 2 };
		int32_t layoutColumnCount = std::max((int)(availWidth / (layoutItemSize.x + layoutItemSpacing)), 1);
		int32_t layoutLineCount = (m_activePaths.size() + layoutColumnCount - 1) / layoutColumnCount;

		glm::float2 layoutItemStep = { layoutItemSize.x + layoutItemSpacing, layoutItemSize.y + layoutItemSpacing };
		float layoutSelectableSpacing = std::max(floorf(layoutItemSpacing) - iconHitSpacing, 0.0f);
		float layoutOuterPadding = floorf(layoutItemSpacing * 0.5f);

		glm::float2 startPos = ImGui::GetCursorScreenPos();
		startPos = glm::float2(startPos.x + layoutOuterPadding, startPos.y + layoutOuterPadding);
		ImGui::SetCursorScreenPos(startPos);

		ImGuiMultiSelectIO* multiselectIO = ImGui::BeginMultiSelect(ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_ClearOnClickVoid, m_selection.Size, m_activePaths.size());

		m_selection.UserData = &listOfPathIDs;
		m_selection.AdapterIndexToStorageId = 
			[](ImGuiSelectionBasicStorage* self_, int idx)
			{
				return (*(std::vector<ImGuiID>*)self_->UserData)[idx];
			};

		m_selection.ApplyRequests(multiselectIO);

		//TODO: const bool want_delete = (ImGui::Shortcut(ImGuiKey_Delete, ImGuiInputFlags_Repeat) && (m_selection.Size > 0)) || RequestDelete;
		const int item_curr_idx_to_focus = -1; //TODO: want_delete ? m_selection.ApplyDeletionPreLoop(multiselectIO, Items.Size) : -1;
		//TODO: RequestDelete = false;

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(layoutSelectableSpacing, layoutSelectableSpacing));

		const int columnCount = layoutColumnCount;
		ImGuiListClipper clipper;
		clipper.Begin(layoutLineCount, layoutItemStep.y);
		if (item_curr_idx_to_focus != -1)
			clipper.IncludeItemByIndex(item_curr_idx_to_focus / columnCount); // Ensure focused item line is not clipped.
		if (multiselectIO->RangeSrcItem != -1)
			clipper.IncludeItemByIndex((int)multiselectIO->RangeSrcItem / columnCount); // Ensure RangeSrc item line is not clipped.
		while (clipper.Step())
		{
			for (int lineIdx = clipper.DisplayStart; lineIdx < clipper.DisplayEnd; lineIdx++)
			{
				const int itemMinIdxForCurrentLine = lineIdx * columnCount;
				const int itemMaxIdxForCurrentLine = std::min((lineIdx + 1) * columnCount, (int)m_activePaths.size());
				for (int item_idx = itemMinIdxForCurrentLine; item_idx < itemMaxIdxForCurrentLine; ++item_idx)
				{
					ImGuiID item = listOfPathIDs[item_idx];
					std::fs::path currPath = m_activePaths[item];
					ImGui::PushID(item);

					// Position item
					glm::float2 itemPos = { startPos.x + (item_idx % columnCount) * layoutItemStep.x, startPos.y + lineIdx * layoutItemStep.y };
					ImGui::SetCursorScreenPos(itemPos);

					ImGui::SetNextItemSelectionUserData(item_idx);
					bool itemIsSelected = m_selection.Contains((ImGuiID)item);
					bool itemIsVisible = ImGui::IsRectVisible(layoutItemSize);
					ImGui::Selectable("", itemIsSelected, ImGuiSelectableFlags_None, layoutItemSize);

					if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
					{
						if (std::fs::is_directory(currPath))
							m_sidePanel.SetActiveDir(currPath);
					}

					glm::float2 rectMin = ImGui::GetItemRectMin();
					glm::float2 rectMax = ImGui::GetItemRectMax();

					// Update our selection state immediately (without waiting for EndMultiSelect() requests)
					// because we use this to alter the color of our text/icon.
					if (ImGui::IsItemToggledSelection())
						itemIsSelected = !itemIsSelected;

					// Focus (for after deletion)
					if (item_curr_idx_to_focus == item_idx)
						ImGui::SetKeyboardFocusHere(-1);

					// Drag and drop
					if (ImGui::BeginDragDropSource())
					{
						// Create payload with full selection OR single unselected item.
						// (the later is only possible when using ImGuiMultiSelectFlags_SelectOnClickRelease)
						if (ImGui::GetDragDropPayload() == NULL)
						{
							ImVector<ImGuiID> payload_items;
							void* it = NULL;
							ImGuiID id = 0;
							if (!itemIsSelected)
								payload_items.push_back(item);
							else
								while (m_selection.GetNextSelectedItem(&it, &id))
									payload_items.push_back(id);
							ImGui::SetDragDropPayload("ASSETS_BROWSER_ITEMS", payload_items.Data, (size_t)payload_items.size_in_bytes());
						}

						// Display payload content in tooltip, by extracting it from the payload data
						// (we could read from selection, but it is more correct and reusable to read from payload)
						const ImGuiPayload* payload = ImGui::GetDragDropPayload();
						const int payload_count = (int)payload->DataSize / (int)sizeof(ImGuiID);
						ImGui::Text("%d assets", payload_count);

						ImGui::EndDragDropSource();
					}

					if (itemIsVisible)
					{
						bool isDirectory = std::fs::is_directory(currPath);
						AssetType* assetType = AssetTypeRegistry::Get().GetAssetTypeForExtension(currPath.extension());

						// Background Image
						{
							ImGui::GetWindowDrawList()->AddRectFilled(rectMin, rectMax, ImGui::GetColorU32(EditorTheme::s_windowBgAlternativeColor));
						}

						if (itemIsSelected)
						{
							ImU32 selectColor = ImGui::GetColorU32({ 0.2f, 0.5f, 0.9f, 0.18f });
							ImGui::GetWindowDrawList()->AddRectFilled(rectMin, rectMax, selectColor);
						}
						else if (ImGui::IsItemHovered())
						{
							ImU32 hoverColor = ImGui::GetColorU32({ 0.2f, 0.5f, 0.9f, 0.12f });
							ImGui::GetWindowDrawList()->AddRectFilled(rectMin, rectMax, hoverColor);
						}

						// Icon
						{
							constexpr float ICON_PADDING = 4.f;
							ImGui::SetCursorScreenPos(rectMin + glm::float2{ICON_PADDING, 0.f});
							ImGui::SetNextItemAllowOverlap();
							auto size = glm::compMin(rectMax - rectMin - 2 * ICON_PADDING);
							ImGui::Image(isDirectory ? m_folderIcon->GetRenderResource() : m_textureIcon->GetRenderResource(), { size, size });
						}

						// Indicator line
						constexpr float INDICATOR_LINE_WIDTH = 3.f;
						{
							glm::float4 color = isDirectory ? glm::float4{} : (assetType ? assetType->GetAssetIndicatorColor() : glm::float4{0.3f, 0.3f, 0.3f, 1.f});
							ImGui::GetWindowDrawList()->AddRectFilled(
								{ ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y },
								{ ImGui::GetItemRectMax().x, ImGui::GetItemRectMax().y + INDICATOR_LINE_WIDTH },
								ImGui::GetColorU32(color));
						}

						constexpr glm::float2 FILE_TYPE_PADDING = {8.f, 4.f};
						float fileTypeTextHeight = 2 * FILE_TYPE_PADDING.y + EditorTheme::s_smallFont->FontSize;

						// Filename
						{
							constexpr float FILENAME_PADDING = 8.f;
							ImGui::SetCursorScreenPos({ ImGui::GetItemRectMin().x + FILENAME_PADDING, ImGui::GetItemRectMax().y + INDICATOR_LINE_WIDTH + FILENAME_PADDING });
							
							float filenameTextHeight = itemPos.y + layoutItemSize.y - fileTypeTextHeight - ImGui::GetCursorScreenPos().y;
							const ImRect clipRect = ImRect(
								{ ImGui::GetCursorScreenPos() },
								{ ImGui::GetCursorScreenPos().x + scaledThumbnailSizeX, filenameTextHeight + ImGui::GetCursorScreenPos().y + (isDirectory ? fileTypeTextHeight : 0.f) });
							ImGui::PushClipRect(clipRect.Min, clipRect.Max, true);
							auto filename = currPath.filename().string();
							ImGui::PushTextWrapPos(itemPos.x - ImGui::GetCurrentWindowRead()->Pos.x + scaledThumbnailSizeX - FILENAME_PADDING);
							ImGui::TextWrapped("%s", filename.c_str());
							ImGui::PopTextWrapPos();
							ImGui::PopClipRect();
						}

						// File Type
						if (!isDirectory)
						{
							ImGui::SetCursorScreenPos({ itemPos.x + FILE_TYPE_PADDING.x, itemPos.y + (rectMax - rectMin).y - fileTypeTextHeight });
							ImGui::BeginDisabled();
							ImGui::PushFont(EditorTheme::s_smallFont);
							std::string fileTypeName = assetType ? assetType->GetFileTypeName() : "Unknown";
							ImGui::Text("%s", fileTypeName.c_str());
							ImGui::PopFont();
							ImGui::EndDisabled();
						}
					}

					ImGui::PopID();
				}
			}
		}
		clipper.End();
		ImGui::PopStyleVar(); // ImGuiStyleVar_ItemSpacing

		multiselectIO = ImGui::EndMultiSelect();
		m_selection.ApplyRequests(multiselectIO);
		// TODO
		//if (want_delete)
		//	Selection.ApplyDeletionPostLoop(ms_io, Items, item_curr_idx_to_focus);
	}
	ImGui::EndChild();

	


	/*std::filesystem::path directoryToOpen;
	std::filesystem::path directoryToDelete;

	constexpr float padding = 4.0f;
	float scaledThumbnailSize = m_thumbnailSize * ImGui::GetIO().FontGlobalScale;
	float scaledThumbnailSizeX = scaledThumbnailSize * 0.55f;
	float cellSize = scaledThumbnailSizeX + 2 * padding + scaledThumbnailSizeX * 0.1f;

	constexpr float overlayPaddingY = 6.0f * padding;
	constexpr float thumbnailPadding = overlayPaddingY * 0.5f;
	float thumbnailSize = scaledThumbnailSizeX - thumbnailPadding;

	ImVec2 backgroundThumbnailSize = { scaledThumbnailSizeX + padding * 2, scaledThumbnailSize + padding * 2 };

	float panelWidth = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ScrollbarSize;
	int32_t columnCount = (int32_t)(panelWidth / cellSize);
	columnCount = std::max(columnCount, 1);

	float lineHeight = ImGui::GetTextLineHeight();
	int flags = ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_ScrollY;

	if (!grid)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, {0, 0});
		columnCount = 1;
		flags |= ImGuiTableFlags_RowBg
			| ImGuiTableFlags_NoPadOuterX
			| ImGuiTableFlags_NoPadInnerX
			| ImGuiTableFlags_SizingStretchSame;
	}
	else
	{
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, {scaledThumbnailSizeX * 0.05f, scaledThumbnailSizeX * 0.05f});
		flags |= ImGuiTableFlags_PadOuterX | ImGuiTableFlags_SizingFixedFit;
	}

	ImVec2 cursorPos = ImGui::GetCursorPos();
	const ImVec2 region = ImGui::GetContentRegionAvail();
	ImGui::InvisibleButton("##DragDropTargetAssetPanelBody", region);

	ImGui::SetNextItemAllowOverlap();
	ImGui::SetCursorPos(cursorPos);

	if (ImGui::BeginTable("BodyTable", columnCount, flags))
	{
		int i = 0;
		for (auto& file : m_directoryEntries)
		{
			ImGui::PushID(i);

			Ref<Render::Texture> texture;
			if (file.isDirectory)
				texture = m_folderIcon->GetRenderResource();
			else
				texture = m_textureIcon->GetRenderResource();
			
			if (!texture)
				texture = EditorApplication::Get().GetBuiltinResources().blackTexture;

			ImGui::TableNextColumn();

			const auto& path = file.directoryEntry.path();

			if (grid)
			{
				cursorPos = ImGui::GetCursorPos();

				// Background button
				std::string id = "##" + std::to_string(i);
				//ImGui::InvisibleButton(id.c_str(), backgroundThumbnailSize);
				ImGui::PushID(id.c_str());
				ImGui::Selectable("", &file.selected, 0, backgroundThumbnailSize);
				ImGui::PopID();

				/*ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, EditorTheme::s_popupItemSpacing);
				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::MenuItemEx("Delete", ICON_MDI_DELETE))
					{
						directoryToDelete = path;
						ImGui::CloseCurrentPopup();
					}
					if (ImGui::MenuItemEx("Rename", ICON_MDI_RENAME))
					{
						ImGui::CloseCurrentPopup();
					}

					ImGui::Separator();

					DrawContextMenuItems(path, isDir);
					ImGui::EndPopup();
				}
				ImGui::PopStyleVar();#1#

				/*if (ImGui::BeginDragDropTarget())
				{
					//if (isDir)
					//	DragDropTarget(file.filepath);

					ImGui::EndDragDropTarget();
				}
				if (ImGui::BeginDragDropSource())
				{
					//DragDropFrom(file.filepath, file.name);
				}#1#

				if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0))
				{
					m_selectedFile = file.filepath;
				}
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
				{
					if (file.isDirectory)
					{
						directoryToOpen = path;
					}
					else
					{
						/*OpenFile(path);#1#
					}
				}

				// Background Image
				/*{
					ImVec2 rectMin = { cursorPos.x, cursorPos.y };
					ImVec2 rectMax = { rectMin.x + backgroundThumbnailSize.x, rectMin.y + backgroundThumbnailSize.y };
					ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetCurrentWindowRead()->Pos + rectMin, ImGui::GetCurrentWindowRead()->Pos + rectMax, ImGui::GetColorU32(EditorTheme::s_windowBgAlternativeColor), 6.0f);
				}

				if (m_selectedFile == file.filepath)
				{
					ImU32 hoverColor = ImGui::GetColorU32({ 0.2f, 0.5f, 0.9f, 0.18f });
					ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), hoverColor, 6.0f);
					ImGui::GetStyle().
				}
				else if (ImGui::IsItemHovered())
				{
					ImU32 hoverColor = ImGui::GetColorU32({0.2f, 0.5f, 0.9f, 0.18f});
					ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), hoverColor, 6.0f);
				}#1#

				// Thumbnail Image
				ImGui::SetCursorPos({ cursorPos.x + thumbnailPadding * 0.75f, cursorPos.y + thumbnailPadding });
				ImGui::SetNextItemAllowOverlap();
				ImGui::Image(texture, {thumbnailSize, thumbnailSize});

				// Type Color frame
				const ImVec2 typeColorFrameSize = { scaledThumbnailSizeX, scaledThumbnailSizeX * 0.03f };
				ImGui::SetCursorPosX(cursorPos.x + padding);
				ImGui::Image(EditorApplication::Get().GetBuiltinResources().whiteTexture, typeColorFrameSize, { 0, 0 }, { 1, 1 },
					file.isDirectory ? glm::float4(0.0f, 0.0f, 0.0f, 0.0f) : (file.assetType ? file.assetType->GetAssetIndicatorColor() : glm::float4{0.3f, 0.3f, 0.3f, 1.f}));

				ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2{padding * 2.f, padding * 2.f});
				float fileTypeTextY = cursorPos.y + backgroundThumbnailSize.y - EditorTheme::s_smallFont->FontSize - padding * 4.0f;
				const ImRect clipRect = ImRect({ ImGui::GetCursorScreenPos() },
											   { ImGui::GetCursorScreenPos().x + scaledThumbnailSizeX, fileTypeTextY + ImGui::GetCurrentWindowRead()->Pos.y - padding });
				ImGui::PushClipRect(clipRect.Min, clipRect.Max, true);
				auto filename = file.filename.string();
				ImGui::PushTextWrapPos(cursorPos.x + scaledThumbnailSizeX - padding * 2.f);
				ImGui::TextWrapped("%s", filename.c_str());
				ImGui::PopTextWrapPos();
				ImGui::PopClipRect();

				if (!file.isDirectory)
				{
					ImGui::SetCursorPos({ cursorPos.x + padding * 2.0f, fileTypeTextY });
					ImGui::BeginDisabled();
					ImGui::PushFont(EditorTheme::s_smallFont);
					std::string fileTypeName = file.assetType ? file.assetType->GetFileTypeName() : "Unknown";
					ImGui::Text("%s", fileTypeName.c_str());
					ImGui::PopFont();
					ImGui::EndDisabled();
				}
			}
			else
			{
				constexpr ImGuiTreeNodeFlags teeNodeFlags = ImGuiTreeNodeFlags_FramePadding
					| ImGuiTreeNodeFlags_SpanFullWidth
					| ImGuiTreeNodeFlags_Leaf;

				auto filename = file.filename.string();

				const bool opened = ImGui::TreeNodeEx(filename.c_str(), teeNodeFlags, "");

				if (file.isDirectory && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
					directoryToOpen = path;

				if (ImGui::BeginDragDropSource())
				{
					//TODO DragDropFrom(file.filepath, file.name);
				}

				ImGui::SameLine();
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() - lineHeight);
				ImGui::Image(texture, { lineHeight, lineHeight });
				ImGui::SameLine();
				ImGui::TextUnformatted(filename.c_str());

				if (opened)
					ImGui::TreePop();
			}

			ImGui::PopID();
			++i;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, EditorTheme::s_popupItemSpacing);
		// TODO
		/*if (ImGui::BeginPopupContextWindow("AssetPanelHierarchyContextWindow", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			EditorLayer::GetInstance()->ResetContext();
			DrawContextMenuItems(m_currentDirectory, true);
			ImGui::EndPopup();
		}#1#
		ImGui::PopStyleVar();

		ImGui::EndTable();

		//if (!anyItemHovered && ImGui::IsItemClicked())
		//	EditorLayer::GetInstance()->ResetContext();
	}

	ImGui::PopStyleVar();

	if (!directoryToDelete.empty())
	{
		std::filesystem::remove_all(directoryToDelete);
		//EditorLayer::GetInstance()->ResetContext();
	}

	if (!directoryToOpen.empty())
		UpdateDirectoryEntries(directoryToOpen);*/
}

ImGuiID AssetBrowserPanel::UpdateDirectoryEntries(std::fs::path dir, ImGuiID id)
{
	bool activeDir = std::ranges::find(m_activeDirs, dir) != m_activeDirs.end();
	if (activeDir)
		m_activePaths.erase(id);
	for (auto& entry : std::fs::directory_iterator(dir))
	{
		++id;
		if (activeDir)
			m_activePaths[id] = entry;
		if (entry.is_directory())
			id = UpdateDirectoryEntries(entry, id);
	}

	return id;
}

void AssetBrowserPanel::DrawContextMenuItems(const std::filesystem::path& context, bool isDir)
{
	if (isDir)
	{
		if (ImGui::BeginMenu("Create"))
		{
			if (ImGui::MenuItemEx("Folder", ICON_MDI_FOLDER_PLUS))
			{
				int i = 0;
				bool created = false;
				std::string newFolderPath;
				while (!created)
				{
					std::string folderName = "New Folder" + (i == 0 ? "" : std::format(" ({})", i));
					newFolderPath = (context / folderName).string();
					created = std::filesystem::create_directory(newFolderPath);
					++i;
				}
				//TODO EditorLayer::GetInstance()->SetContext(EditorContextType::File, newFolderPath.c_str(), sizeof(char) * (newFolderPath.length() + 1));
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndMenu();
		}
	}
	/*if (ImGui::MenuItemEx("Show in Explorer", ARC_ICON_ARROW_TOP_RIGHT))
	{
		std::string path = context.string();
		FileDialogs::OpenFolderAndSelectItem(path.c_str());
		ImGui::CloseCurrentPopup();
	}
	if (ImGui::MenuItemEx("Open", ARC_ICON_OPEN_IN_APP))
	{
		std::string path = context.string();
		FileDialogs::OpenFileWithProgram(path.c_str());
		ImGui::CloseCurrentPopup();
	}
	if (ImGui::MenuItem("Copy Path"))
	{
		std::string path = context.string();
		ImGui::SetClipboardText(path.c_str());
		ImGui::CloseCurrentPopup();
	}*/

	if (isDir)
	{
		if (ImGui::MenuItemEx("Refresh", ICON_MDI_REFRESH))
		{
			//Refresh();
			ImGui::CloseCurrentPopup();
		}
	}
}

std::pair<bool, uint32_t> AssetBrowserPanel::DirectoryTreeViewRecursive(const std::filesystem::path& path, uint32_t* count, uint64_t* selectionMask, ImGuiTreeNodeFlags flags)
{
	bool anyNodeClicked = false;
	uint32_t nodeClicked = 0;

	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		ImGuiTreeNodeFlags nodeFlags = flags;

		auto& entryPath = entry.path();

		if (!std::filesystem::is_directory(entryPath))
			continue;

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		bool selected = (*selectionMask & 1 << *count) != 0;
		if (selected)
		{
			nodeFlags |= ImGuiTreeNodeFlags_Selected;
			ImGui::PushStyleColor(ImGuiCol_Header, EditorTheme::s_headerSelectedColor);
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::s_headerSelectedColor);
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::s_headerHoveredColor);
		}

		const uint64_t id = *count;
		bool open = ImGui::TreeNodeEx(reinterpret_cast<void*>(id), nodeFlags, "");
		ImGui::PopStyleColor(selected ? 2 : 1);

		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
		{
			//UpdateDirectoryEntries(entryPath, TODO);

			nodeClicked = *count;
			anyNodeClicked = true;
		}

		std::string name = entryPath.filename().string();
		//TODO
		/*if (ImGui::BeginDragDropTarget())
		{
			if (!entryIsFile)
				DragDropTarget(filepath.c_str());

			ImGui::EndDragDropTarget();
		}
		if (ImGui::BeginDragDropSource())
		{
			DragDropFrom(filepath.c_str(), name);
		}*/

		const char* folderIcon = open ? ICON_MDI_FOLDER_OPEN : ICON_MDI_FOLDER;

		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::s_assetIconColor);
		ImGui::TextUnformatted(folderIcon);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		ImGui::TextUnformatted(name.c_str());
		m_currentlyVisibleItemsTreeView++;

		(*count)--;

		if (open)
		{
			auto [isClicked, clickedNode] = DirectoryTreeViewRecursive(entryPath, count, selectionMask, flags);

			if (!anyNodeClicked)
			{
				anyNodeClicked = isClicked;
				nodeClicked = clickedNode;
			}

			ImGui::TreePop();
		}
		else
		{
			for (auto& e : std::filesystem::recursive_directory_iterator(entryPath))
				(*count)--;
		}
	}

	return { anyNodeClicked, nodeClicked };
}
}
