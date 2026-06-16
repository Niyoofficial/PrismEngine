#include "AssetBrowserPanel.h"

#include <algorithm>
#include <filesystem>

#include "EditorApplication.h"
#include "EditorTheme.h"
#include "Sandbox.h"
#include "xxhash.h"
#include "Prism/AssetManagement/AssetType.h"
#include "Prism/AssetManagement/TextureAsset.h"
#include "Prism/Base/Paths.h"

namespace Prism
{
namespace
{
ImGuiID GetPathHash(std::fs::path path)
{
	auto stringPath = path.lexically_normal().string();
	return XXH32(stringPath.c_str(), stringPath.size(), 0);
}
}

AssetBrowserPanel::SidePanel::SidePanel(std::function<void()> onSelectionChanged)
	: m_onSelectionChanged(onSelectionChanged)
{
	ImGuiID pathID = GetPathHash(Core::Paths::Get().GetProjectAssetsDir());
	m_selectionStorage.SetItemSelected(pathID, true);
	m_activeDirs[pathID] = Core::Paths::Get().GetProjectAssetsDir();
}

void AssetBrowserPanel::SidePanel::Render()
{
	for (auto dir : m_dirsToOpen)
		ImGui::GetStateStorage()->SetBool(dir, true);
	m_dirsToOpen.clear();

	ImGuiMultiSelectIO* multiselectIO = ImGui::BeginMultiSelect(ImGuiMultiSelectFlags_None, m_selectionStorage.Size);

	ApplySelectionRequests(multiselectIO);

	DrawDirectories(Core::Paths::Get().GetProjectAssetsDir());

	multiselectIO = ImGui::EndMultiSelect();

	ApplySelectionRequests(multiselectIO);
}

void AssetBrowserPanel::SidePanel::SetActiveDir(std::fs::path path)
{
	m_activeDirs.clear();

	auto pathID = GetPathHash(path);
	m_activeDirs[pathID] = path.lexically_normal();
	m_selectionStorage.Clear();
	m_selectionStorage.SetItemSelected(pathID, true);

	while ((path = path.parent_path()) != Core::Paths::Get().GetProjectAssetsDir().parent_path())
		m_dirsToOpen.push_back(GetPathHash(path));
}

std::vector<std::fs::path> AssetBrowserPanel::SidePanel::GetActiveDirs() const
{
	std::vector<std::fs::path> paths;
	for (auto& path : m_activeDirs | std::views::values)
		paths.push_back(path);
	return paths;
}

void AssetBrowserPanel::SidePanel::ForEachVisibleDir(std::fs::path path, const std::function<bool(std::fs::path)>& func)
{
	std::function<bool(std::fs::path, const std::function<bool(std::fs::path)>&)> forEachInternal =
		[&forEachInternal, this](std::fs::path path, const std::function<bool(std::fs::path)>& func)
		{
			bool toContinue = false;
			toContinue = func(path);
			if (toContinue)
			{
				bool open = IsOpen(GetPathHash(path));
				if (open)
				{
					for (const auto& entry : std::fs::directory_iterator(path))
					{
						if (!entry.is_directory())
							continue;

						if (!forEachInternal(entry, func))
							return false;
					}
				}
			}

			return toContinue;
		};

	forEachInternal(path, func);
}

void AssetBrowserPanel::SidePanel::ApplySelectionRequests(ImGuiMultiSelectIO* multiselectIO)
{
	for (auto& req : multiselectIO->Requests)
	{
		if (req.Type == ImGuiSelectionRequestType_SetAll)
		{
			if (req.Selected)
			{
				ForEachVisibleDir(Core::Paths::Get().GetProjectAssetsDir(),
								  [this](std::fs::path path)
								  {
									  auto hash = GetPathHash(path);
									  m_selectionStorage.SetItemSelected(hash, true);
									  m_activeDirs[hash] = path;

									  return true;
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

			bool found = false;
			ForEachVisibleDir(Core::Paths::Get().GetProjectAssetsDir(),
							  [&found, first, last, &req, this](std::fs::path path)
							  {
								  auto hash = GetPathHash(path);
								  bool toContinue = true;
								  if (hash == first || hash == last)
								  {
									  if (found)
										  toContinue = false;
									  found = !found;
								  }

								  if (found || !toContinue)
								  {
									  m_selectionStorage.SetItemSelected(hash, req.Selected);
									  if (req.Selected)
										  m_activeDirs[hash] = path;
									  else
										  m_activeDirs.erase(hash);
								  }

								  if (found && first == last)
									  return false;

								  return toContinue;
							  });
		}
	}

	if (!multiselectIO->Requests.empty())
		m_onSelectionChanged();
}

void AssetBrowserPanel::SidePanel::DrawDirectories(std::fs::path path)
{
	path = path.lexically_normal();

	ImGuiTreeNodeFlags treeNodeFlags =
		ImGuiTreeNodeFlags_SpanFullWidth |
		ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_OpenOnDoubleClick |
		ImGuiTreeNodeFlags_NavLeftJumpsBackHere |
		ImGuiTreeNodeFlags_FramePadding;

	bool hasChildren = false;
	for (const auto& entry : std::fs::directory_iterator(path))
	{
		if (entry.is_directory())
		{
			hasChildren = true;
			break;
		}
	}
	if (!hasChildren)
		treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;

	ImGuiID hash = GetPathHash(path);

	bool wasSelected = false;
	if (m_selectionStorage.Contains(hash))
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

	ImGui::SetNextItemSelectionUserData(hash);
	ImGui::SetNextItemStorageID(hash);
	std::string nodeName = std::string(IsOpen(hash) ? ICON_MDI_FOLDER_OPEN : ICON_MDI_FOLDER) + " " + path.filename().string();
	bool open = ImGui::TreeNodeEx(path.string().c_str(), treeNodeFlags, "%s", nodeName.c_str());
	ImGui::PopStyleColor(wasSelected ? 2 : 1);
	if (open)
	{
		for (const auto& entry : std::fs::directory_iterator(path))
		{
			// TODO: Stupid hack, because directories usually don't have extension, it avoids expensive is_directory call
			if (!entry.path().has_extension() && entry.is_directory())
				DrawDirectories(entry);
		}
		ImGui::TreePop();
	}
	else
	{
		if (ImGui::IsItemToggledOpen())
		{
			std::function<bool(const std::fs::path&)> closeAndUnselectChildren =
				[&closeAndUnselectChildren, this](const std::fs::path& path)
				{
					ImGui::GetStateStorage()->SetBool(GetPathHash(path), false);

					bool wasSelected = false;
					for (const auto& entry : std::fs::directory_iterator(path))
					{
						if (entry.is_directory())
						{
							auto childID = GetPathHash(entry);
							wasSelected = m_selectionStorage.Contains(childID);
							m_selectionStorage.SetItemSelected(childID, false);
							if (IsOpen(childID))
								wasSelected |= closeAndUnselectChildren(entry);
						}
					}
					return wasSelected;
				};

			if (closeAndUnselectChildren(path))
				m_selectionStorage.SetItemSelected(hash, true);
		}
	}
}

bool AssetBrowserPanel::SidePanel::IsOpen(ImGuiID id) const
{
	return ImGui::GetStateStorage()->GetBool(id);
}

AssetBrowserPanel::AssetBrowserPanel()
	: m_folderIcon(EditorApplication::Get().GetAssetManager().LoadAsset<TextureAsset>("engine/folder_icon.png")),
	  m_textureIcon(EditorApplication::Get().GetAssetManager().LoadAsset<TextureAsset>("engine/texture_icon.png")),
	  m_sidePanel([this](){m_selection.Clear();})
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

	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
	ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.0f, 0.0f, 0.0f, 0.0f });

	if (m_activeDirs.size() == 1)
	{
		std::filesystem::path projectDir = Core::Paths::Get().GetProjectDir();
		static size_t absProjectDirLength = projectDir.string().length();

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

		if (!directoryToOpen.empty() && std::fs::is_directory(directoryToOpen))
			m_sidePanel.SetActiveDir(directoryToOpen);
	}
	else
	{
		ImGui::SameLine();
		ImGui::PushID("multiple_button");
		ImGui::Button("<multiple>");
		ImGui::PopID();
	}

	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar();
}

void AssetBrowserPanel::RenderBody()
{
	if (ImGui::BeginChild("Assets", {}, ImGuiChildFlags_None, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
	{
		m_activeDirs = m_sidePanel.GetActiveDirs();
		m_displayedPaths.clear();

		UpdateDirectoryEntries(Core::Paths::Get().GetProjectAssetsDir(), 0);

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

		bool pressedDelete = (ImGui::IsWindowFocused() && ImGui::Shortcut(ImGuiKey_Delete, ImGuiInputFlags_Repeat) && (m_selection.Size > 0));
		bool confirmedDelete = false;

		if (ImGui::BeginPopupModal("CONFIRM_DELETE_POPUP", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar))
		{
			ImGui::Text("Are you sure you want to delete?");
			if (ImGui::Button("Yes"))
			{
				confirmedDelete = true;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
 			if (ImGui::Button("No"))
 			{
				ImGui::CloseCurrentPopup();
 			}

			ImGui::EndPopup();
		}

		if (pressedDelete)
			ImGui::OpenPopup("CONFIRM_DELETE_POPUP");

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

					// Drag and drop
					if (ImGui::BeginDragDropSource())
					{
						// Create payload with full selection OR single unselected item.
						// (the latter is only possible when using ImGuiMultiSelectFlags_SelectOnClickRelease)
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

								out << AssetRegistry::Get().GetRelPath(path).string();

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

		if (confirmedDelete)
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
		if (activeDir && entry.path().extension() != ".meta")
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
}
