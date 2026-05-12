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

	if (!multiselectIO->Requests.empty())
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
	: m_assetsDirectory(Core::Paths::Get().GetProjectAssetsDir()),
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
	// TODO: Filtering
	/*m_filter.Draw("##asset_filter", ImGui::GetContentRegionAvail().x);
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

	ImGui::SameLine();
	*/

	constexpr const char* folderIcon = ICON_MDI_FOLDER;
	ImGui::TextUnformatted(folderIcon);

	if (m_activeDirs.size() == 1)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.0f, 0.0f, 0.0f, 0.0f });

		std::filesystem::path projectDir = m_assetsDirectory.parent_path();
		static std::filesystem::path absProjectDir = std::filesystem::absolute(m_assetsDirectory.parent_path());
		static std::string absProjectDirString = absProjectDir.string();
		static size_t absProjectDirLength = absProjectDirString.length();

		std::string currentDir = m_activeDirs[0].string();
		const char* p = &currentDir[absProjectDirLength + 1];
		const std::filesystem::path currentDirectory = p;

		std::filesystem::path directoryToOpen;
		for (auto& path : currentDirectory)
		{
			projectDir /= path;
			ImGui::SameLine();
			ImGui::PushID(projectDir.c_str());
			ImGui::Button(path.filename().string().c_str());
			ImGui::PopID();

			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				directoryToOpen = projectDir;

			if (m_activeDirs[0] != projectDir)
			{
				ImGui::SameLine();
				constexpr const char* delimeter = "/";
				ImGui::TextUnformatted(delimeter, delimeter + 1);
			}
		}
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar();

		if (!directoryToOpen.empty() && std::fs::is_directory(directoryToOpen))
			m_sidePanel.SetActiveDir(directoryToOpen);
	}
	else
	{
		ImGui::SameLine();
		ImGui::TextUnformatted("<multiple>");
	}
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
		m_displayedPaths.clear();

		UpdateDirectoryEntries(m_assetsDirectory, 0);

		std::vector<ImGuiID> listOfPathIDs;
		listOfPathIDs.reserve(m_displayedPaths.size());
		for (const auto& id : m_displayedPaths | std::views::keys)
			listOfPathIDs.push_back(id);

		constexpr float layoutItemSpacing = 10.f;
		constexpr int32_t iconHitSpacing = 10;
		constexpr float originalThumbnailSize = 140.f;

		float scaledThumbnailSize = originalThumbnailSize * ImGui::GetIO().FontGlobalScale;
		float scaledThumbnailSizeX = scaledThumbnailSize * 0.55f;

		float availWidth = ImGui::GetContentRegionAvail().x;

		availWidth += floorf(layoutItemSpacing * 0.5f);

		// Layout: calculate number of icons per line and number of lines
		constexpr float ITEM_PADDING = 4.f;
		glm::float2 layoutItemSize = { scaledThumbnailSizeX + ITEM_PADDING * 2, scaledThumbnailSize + ITEM_PADDING * 2 };
		int32_t layoutColumnCount = std::max((int)(availWidth / (layoutItemSize.x + layoutItemSpacing)), 1);
		int32_t layoutLineCount = (m_displayedPaths.size() + layoutColumnCount - 1) / layoutColumnCount;

		glm::float2 layoutItemStep = { layoutItemSize.x + layoutItemSpacing, layoutItemSize.y + layoutItemSpacing };
		float layoutSelectableSpacing = std::max(floorf(layoutItemSpacing) - iconHitSpacing, 0.0f);
		float layoutOuterPadding = floorf(layoutItemSpacing * 0.5f);

		glm::float2 startPos = ImGui::GetCursorScreenPos();
		startPos = glm::float2(startPos.x + layoutOuterPadding, startPos.y + layoutOuterPadding);
		ImGui::SetCursorScreenPos(startPos);

		ImGuiMultiSelectIO* multiselectIO = ImGui::BeginMultiSelect(ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_ClearOnClickVoid, m_selection.Size, m_displayedPaths.size());

		m_selection.UserData = &listOfPathIDs;
		m_selection.AdapterIndexToStorageId = 
			[](ImGuiSelectionBasicStorage* self_, int idx)
			{
				return (*(std::vector<ImGuiID>*)self_->UserData)[idx];
			};

		m_selection.ApplyRequests(multiselectIO);

		bool wantDelete = (ImGui::Shortcut(ImGuiKey_Delete, ImGuiInputFlags_Repeat) && (m_selection.Size > 0)); // TODO: || RequestDelete;
		//TODO: RequestDelete = false;

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(layoutSelectableSpacing, layoutSelectableSpacing));

		int32_t columnCount = layoutColumnCount;
		ImGuiListClipper clipper;
		clipper.Begin(layoutLineCount, layoutItemStep.y);
		if (multiselectIO->RangeSrcItem != -1)
			clipper.IncludeItemByIndex((int)multiselectIO->RangeSrcItem / columnCount); // Ensure RangeSrc item line is not clipped.
		while (clipper.Step())
		{
			for (int lineIdx = clipper.DisplayStart; lineIdx < clipper.DisplayEnd; lineIdx++)
			{
				const int itemMinIdxForCurrentLine = lineIdx * columnCount;
				const int itemMaxIdxForCurrentLine = std::min((lineIdx + 1) * columnCount, (int)m_displayedPaths.size());
				for (int item_idx = itemMinIdxForCurrentLine; item_idx < itemMaxIdxForCurrentLine; ++item_idx)
				{
					ImGuiID item = listOfPathIDs[item_idx];
					std::fs::path currPath = m_displayedPaths[item];
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

					if (wantDelete)
						ImGui::SetKeyboardFocusHere(-1);

					// Drag and drop
					if (ImGui::BeginDragDropSource())
					{
						// Create payload with full selection OR single unselected item.
						// (the later is only possible when using ImGuiMultiSelectFlags_SelectOnClickRelease)
						YAML::Emitter out;
						out << YAML::BeginSeq;

						AssetType* assetType = nullptr;
						bool mixedTypes = false;
						int32_t itemsCount = 0;

						auto serializePath =
							[&out, &assetType, &mixedTypes, &itemsCount](const std::fs::path& path)
							{
								auto currAssetType = AssetTypeRegistry::Get().GetAssetTypeForExtension(path.extension());

								if (assetType && assetType != currAssetType)
									mixedTypes = true;
								else if (!assetType)
									assetType = currAssetType;

								if (auto relEnginePath = path.lexically_relative(Core::Paths::Get().GetEngineAssetsDir()); !relEnginePath.string().starts_with(".."))
									out << ("engine" / relEnginePath).string();
								else if (auto relProjectPath = path.lexically_relative(Core::Paths::Get().GetProjectAssetsDir()); !relProjectPath.string().starts_with(".."))
									out << relProjectPath.string();

								++itemsCount;
							};

						if (!itemIsSelected)
						{
							serializePath(currPath);
						}
						else
						{
							void* it = nullptr;
							ImGuiID id = 0;
							while (m_selection.GetNextSelectedItem(&it, &id))
								serializePath(m_displayedPaths[id]);
						}
						out << YAML::EndSeq;

						std::string payloadType = "ASSET_BROWSER_ITEMS_";
						if (mixedTypes)
							payloadType += "Mixed";
						else if (assetType)
							payloadType += assetType->GetFileTypeName();
						else
							payloadType += "Unknown";

						ImGui::SetDragDropPayload(payloadType.c_str(), out.c_str(), out.size());

						// Display payload content in tooltip, by extracting it from the payload data
						// (we could read from selection, but it is more correct and reusable to read from payload)
						ImGui::Text("%d assets", itemsCount);

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

		// TODO: Add confirmation monad
		if (wantDelete)
		{
			int32_t idx = 0;
			for (const auto& [id, path] : m_displayedPaths)
			{
				if (m_selection.Contains(m_selection.GetStorageIdFromIndex(idx)))
					std::fs::remove(path);

				++idx;
			}

			m_selection.Clear();
		}
	}
	ImGui::EndChild();
}

ImGuiID AssetBrowserPanel::UpdateDirectoryEntries(std::fs::path dir, ImGuiID id)
{
	bool activeDir = std::ranges::find(m_activeDirs, dir) != m_activeDirs.end();
	if (activeDir)
		m_displayedPaths.erase(id);
	for (auto& entry : std::fs::directory_iterator(dir))
	{
		++id;
		if (activeDir)
			m_displayedPaths[id] = entry;
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
