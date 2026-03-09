#include "AssetBrowserPanel.h"

#include <algorithm>
#include <filesystem>

#include "EditorTheme.h"
#include "imgui_internal.h"
#include "Sandbox.h"
#include "Prism/Base/Paths.h"

namespace Prism
{
AssetBrowserPanel::AssetBrowserPanel()
	: m_assetsDirectory(Core::Paths::Get().GetProjectAssetsDir()), m_currentDirectory(m_assetsDirectory)
{
}

void AssetBrowserPanel::UpdateImGui(Duration delta)
{
	if (ImGui::Begin("Asset Browser", nullptr, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar))
	{
		RenderHeader();

		ImGui::Separator();

		const ImVec2 availableRegion = ImGui::GetContentRegionAvail();
		if (ImGui::BeginTable("MainViewTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_ContextMenuInBody, availableRegion))
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();

			RenderSidePanel();
			ImGui::TableNextColumn();
			RenderBody(m_thumbnailSize >= 96.0f);

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
			UpdateDirectoryEntries(m_currentDirectory.parent_path());
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
			UpdateDirectoryEntries(top);
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

	if (!directoryToOpen.empty())
		UpdateDirectoryEntries(m_assetsDirectory / directoryToOpen);
}

void AssetBrowserPanel::RenderSidePanel()
{
	static int s_selectionMask = 0;

	constexpr ImGuiTableFlags tableFlags = ImGuiTableFlags_RowBg
		| ImGuiTableFlags_NoPadInnerX
		| ImGuiTableFlags_NoPadOuterX
		| ImGuiTableFlags_ContextMenuInBody
		| ImGuiTableFlags_ScrollY;

	constexpr ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow
		| ImGuiTreeNodeFlags_FramePadding
		| ImGuiTreeNodeFlags_SpanFullWidth;

	ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, { 0, 0 });
	if (ImGui::BeginTable("SideViewTable", 1, tableFlags))
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGuiTreeNodeFlags nodeFlags = treeNodeFlags;
		const bool selected = m_currentDirectory == m_assetsDirectory && s_selectionMask == 0;
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

		const bool opened = ImGui::TreeNodeEx("AssetsDir", nodeFlags, "");
		ImGui::PopStyleColor(selected ? 2 : 1);

		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
		{
			UpdateDirectoryEntries(m_assetsDirectory);
			s_selectionMask = 0;
		}
		const char* folderIcon = opened ? ICON_MDI_FOLDER_OPEN : ICON_MDI_FOLDER;
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::s_assetIconColor);
		ImGui::TextUnformatted(folderIcon);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		constexpr char rootDirName[] = "Assets";
		ImGui::TextUnformatted(rootDirName, rootDirName + sizeof(rootDirName) - 1);

		if (opened)
		{
			uint32_t count = 0;
			for (auto& entry : std::fs::recursive_directory_iterator(m_assetsDirectory))
				count++;

			auto [isClicked, clickedNode] = DirectoryTreeViewRecursive(m_assetsDirectory, &count, &s_selectionMask, treeNodeFlags);

			if (isClicked)
			{
				// (process outside of tree loop to avoid visual inconsistencies during the clicking frame)
				if (ImGui::GetIO().KeyCtrl)
					s_selectionMask ^= 1 << clickedNode; // CTRL+click to toggle
				else
					s_selectionMask = 1 << clickedNode; // Click to single-select
			}

			ImGui::TreePop();
		}

		ImGui::EndTable();
	}

	ImGui::PopStyleVar();
}

void AssetBrowserPanel::RenderBody(bool grid)
{
	std::filesystem::path directoryToOpen;
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
		bool anyItemHovered = false;
		bool textureCreated = false;

		int i = 0;
		for (auto& file : m_directoryEntries)
		{
			ImGui::PushID(i);

			bool isDir = file.isDirectory;
			char* filename = file.name.data();
			char* filenameEnd = filename + file.name.size();

			Ref<Render::Texture> texture;// m_directoryIcon->GetRendererID();
			if (!isDir)
			{
				/*if (file.Type == FileType::Texture)
				{
					if (file.Thumbnail)
					{
						textureId = file.Thumbnail->GetRendererID();
					}
					else if (!textureCreated)
					{
						textureCreated = true;
						file.Thumbnail = AssetManager::GetTexture2D(file.filepath.c_str());
						textureId = file.Thumbnail->GetRendererID();
					}
					else
					{
						textureId = 0;
					}
				}
				else
				{
					textureId = m_FileIcon->GetRendererID();
				}*/
			}

			if (!texture)
				texture = EditorApplication::Get().GetBuiltinResources().blackTexture;

			ImGui::TableNextColumn();

			const auto& path = file.directoryEntry.path();

			if (grid)
			{
				cursorPos = ImGui::GetCursorPos();

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
				ImGui::PopStyleVar();*/

				/*if (ImGui::BeginDragDropTarget())
				{
					//if (isDir)
					//	DragDropTarget(file.filepath);

					ImGui::EndDragDropTarget();
				}
				if (ImGui::BeginDragDropSource())
				{
					//DragDropFrom(file.filepath, file.name);
				}*/

				if (ImGui::IsItemHovered())
					anyItemHovered = true;

				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
				{
					/*if (isDir)
						directoryToOpen = path;
					else
						OpenFile(path);*/
				}

				//const ImTextureID whiteTexId = reinterpret_cast<ImTextureID>(AssetManager::WhiteTexture()->GetRendererID());

				// Foreground Image
				//ImGui::SetCursorPos({ cursorPos.x + padding, cursorPos.y + padding });
				//ImGui::SetNextItemAllowOverlap();
				//ImGui::Image(whiteTexId, { backgroundThumbnailSize.x - padding * 2.0f, backgroundThumbnailSize.y - padding * 2.0f }, { 0, 0 }, { 1, 1 }, EditorTheme::s_windowBgAlternativeColor);

				// Thumbnail Image
				//ImGui::SetCursorPos({ cursorPos.x + thumbnailPadding * 0.75f, cursorPos.y + thumbnailPadding });
				//ImGui::SetNextItemAllowOverlap();
				ImGui::Image(texture->CreateDefaultSRV(), {thumbnailSize, thumbnailSize});

				// Type Color frame
				const ImVec2 typeColorFrameSize = { scaledThumbnailSizeX, scaledThumbnailSizeX * 0.03f };
				//ImGui::SetCursorPosX(cursorPos.x + padding);
				//ImGui::Image(whiteTexId, typeColorFrameSize, { 0, 0 }, { 1, 1 }, isDir ? ImVec4(0.0f, 0.0f, 0.0f, 0.0f) : file.fileTypeIndicatorColor);

				const ImVec2 rectMin = ImGui::GetItemRectMin();
				const ImVec2 rectSize = ImGui::GetItemRectSize();
				const ImRect clipRect = ImRect({ rectMin.x + padding * 1.0f, rectMin.y + padding * 2.0f },
											   { rectMin.x + rectSize.x, rectMin.y + scaledThumbnailSizeX - EditorTheme::s_smallFont->FontSize - padding * 4.0f });
				//TODO: UI::ClippedText(clipRect.Min, clipRect.Max, filename, filenameEnd, nullptr, { 0, 0 }, nullptr, clipRect.GetSize().x);

				if (!isDir)
				{
					ImGui::SetCursorPos({ cursorPos.x + padding * 2.0f, cursorPos.y + backgroundThumbnailSize.y - EditorTheme::s_smallFont->FontSize - padding * 2.0f });
					ImGui::BeginDisabled();
					ImGui::PushFont(EditorTheme::s_smallFont);
					// TODO
					//ImGui::TextUnformatted(file.fileTypeString._Unchecked_begin(), file.fileTypeString._Unchecked_end());
					ImGui::Text("File type");
					ImGui::PopFont();
					ImGui::EndDisabled();
				}
			}
			else
			{
				constexpr ImGuiTreeNodeFlags teeNodeFlags = ImGuiTreeNodeFlags_FramePadding
					| ImGuiTreeNodeFlags_SpanFullWidth
					| ImGuiTreeNodeFlags_Leaf;

				const bool opened = ImGui::TreeNodeEx(file.name.c_str(), teeNodeFlags, "");

				if (ImGui::IsItemHovered())
					anyItemHovered = true;

				if (isDir && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
					directoryToOpen = path;

				if (ImGui::BeginDragDropSource())
				{
					//TODO DragDropFrom(file.filepath, file.name);
				}

				//ImGui::SameLine();
				//ImGui::SetCursorPosX(ImGui::GetCursorPosX() - lineHeight);
				//ImGui::Image(reinterpret_cast<ImTextureID>(textureId), { lineHeight, lineHeight });
				//ImGui::SameLine();
				//ImGui::TextUnformatted(filename, filenameEnd);

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
		}*/
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
		UpdateDirectoryEntries(directoryToOpen);
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
			Refresh();
			ImGui::CloseCurrentPopup();
		}
	}
}

std::pair<bool, uint32_t> AssetBrowserPanel::DirectoryTreeViewRecursive(const std::filesystem::path& path, uint32_t* count, int* selectionMask, ImGuiTreeNodeFlags flags)
{
	bool anyNodeClicked = false;
	uint32_t nodeClicked = 0;

	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		ImGuiTreeNodeFlags nodeFlags = flags;

		auto& entryPath = entry.path();

		const bool entryIsFile = !std::filesystem::is_directory(entryPath);
		if (entryIsFile)
			nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		const bool selected = (*selectionMask & 1 << *count) != 0;
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
			if (!entryIsFile)
				UpdateDirectoryEntries(entryPath);

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

		const char* folderIcon = ICON_MDI_FILE;
		if (entryIsFile)
		{
			/*auto fileType = FileType::Unknown;
			auto entryPathStr = entryPath.extension().string();
			const auto& fileTypeIt = s_FileTypes.find_as(entryPathStr.c_str());
			if (fileTypeIt != s_FileTypes.end())
				fileType = fileTypeIt->second;

			const auto& fileTypeIconIt = s_FileTypesToIcon.find(fileType);
			if (fileTypeIconIt != s_FileTypesToIcon.end())
				folderIcon = fileTypeIconIt->second;*/
			folderIcon = ICON_MDI_MARKER;
		}
		else
		{
			folderIcon = open ? ICON_MDI_FOLDER_OPEN : ICON_MDI_FOLDER;
		}

		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::s_assetIconColor);
		ImGui::TextUnformatted(folderIcon);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		ImGui::TextUnformatted(name.c_str());
		m_currentlyVisibleItemsTreeView++;

		(*count)--;

		if (!entryIsFile)
		{
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
	}

	return { anyNodeClicked, nodeClicked };
}

void AssetBrowserPanel::UpdateDirectoryEntries(const std::filesystem::path& directory)
{
	m_currentDirectory = directory;
	m_directoryEntries.clear();
	auto directoryIt = std::filesystem::directory_iterator(directory);
	for (auto& directoryEntry : directoryIt)
	{
		auto& path = directoryEntry.path();
		auto relativePath = std::filesystem::relative(path, m_assetsDirectory);
		std::string filename = relativePath.filename().string();
		std::string filepath = path.string();
		std::string extension = relativePath.extension().string();

		m_directoryEntries.push_back({
			.name = filename,
			.filepath = filepath,
			.extension = extension,
			.directoryEntry = directoryEntry,
			.isDirectory = directoryEntry.is_directory()
		});
	}
}
}
