#include "EditorTheme.h"

namespace Prism
{
static ImVec4 Darken(ImVec4 c, float p) { return { glm::max(0.f, c.x - 1.0f * p), glm::max(0.f, c.y - 1.0f * p), glm::max(0.f, c.z - 1.0f * p), c.w }; }
static ImVec4 Lighten(ImVec4 c, float p) { return { glm::max(0.f, c.x + 1.0f * p), glm::max(0.f, c.y + 1.0f * p), glm::max(0.f, c.z + 1.0f * p), c.w }; }

static ImVec4 Disabled(ImVec4 c) { return Darken(c, 0.6f); }
static ImVec4 Hovered(ImVec4 c) { return Lighten(c, 0.2f); }
static ImVec4 Active(ImVec4 c) { return Lighten(ImVec4(c.x, c.y, c.z, 1.0f), 0.1f); }
static ImVec4 Collapsed(ImVec4 c) { return Darken(c, 0.2f); }

ImFont* EditorTheme::s_defaultFont = nullptr;
ImFont* EditorTheme::s_smallFont = nullptr;
ImFont* EditorTheme::s_boldFont = nullptr;

void EditorTheme::Init()
{
	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	// Primary background
	colors[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);  // #131318
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f); // #131318

	colors[ImGuiCol_PopupBg] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);

	// Headers
	colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.30f, 0.40f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.25f, 0.25f, 0.35f, 1.00f);

	// Buttons
	colors[ImGuiCol_Button] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.32f, 0.40f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.38f, 0.50f, 1.00f);

	// Frame BG
	colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.27f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);

	// Tabs
	colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.35f, 0.35f, 0.50f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.25f, 0.25f, 0.38f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.13f, 0.13f, 0.17f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);

	// Title
	colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.20f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);

	// Borders
	colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.25f, 0.50f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	// Text
	colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.95f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.55f, 1.00f);

	// Highlights
	colors[ImGuiCol_CheckMark] = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.60f, 0.80f, 1.00f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.50f, 0.70f, 1.00f, 0.50f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.60f, 0.80f, 1.00f, 0.75f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.70f, 0.90f, 1.00f, 1.00f);

	// Scrollbar
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.35f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.50f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.45f, 0.45f, 0.55f, 1.00f);

	// Table
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.f, 1.f, 1.f, 0.024f);

	// Style tweaks
	style.WindowRounding = 5.0f;
	style.FrameRounding = 5.0f;
	style.GrabRounding = 5.0f;
	style.TabRounding = 5.0f;
	style.PopupRounding = 5.0f;
	style.ScrollbarRounding = 5.0f;
	style.WindowPadding = ImVec2(10, 10);
	style.FramePadding = ImVec2(6, 4);
	style.ItemSpacing = ImVec2(8, 6);
	style.PopupBorderSize = 0.f;

	s_headerSelectedColor = ImVec4(0.19f, 0.53f, 0.78f, 1.00f);
	s_headerHoveredColor = Lighten(colors[ImGuiCol_HeaderActive], 0.1f);
	s_windowBgColor = colors[ImGuiCol_WindowBg];
	s_windowBgAlternativeColor = Lighten(s_windowBgColor, 0.04f);
	s_assetIconColor = Lighten(s_headerSelectedColor, 0.9f);
	s_textColor = colors[ImGuiCol_Text];
	s_textDisabledColor = colors[ImGuiCol_TextDisabled];

	s_framePadding = ImVec2(4.0f, 2.0f);
	s_popupItemSpacing = ImVec2(6.0f, 8.0f);

	// Font loading
	auto loadIconFont =
		[&](float fontSize)
		{
			static constexpr ImWchar iconRanges[] = { ICON_MIN_MDI, ICON_MAX_MDI, 0 };
			ImFontConfig iconsConfig;
			iconsConfig.MergeMode = true;
			iconsConfig.PixelSnapH = true;
			iconsConfig.GlyphOffset.y = 1.0f;
			iconsConfig.OversampleH = iconsConfig.OversampleV = 1;
			iconsConfig.GlyphMinAdvanceX = 4.0f;
			iconsConfig.SizePixels = 12.0f;
			// TODO: Fix this path
			io.Fonts->AddFontFromFileTTF("../PrismCore/assets/materialdesignicons-webfont.ttf", fontSize, &iconsConfig, iconRanges);
		};

	ImFontConfig fontConfig;
	fontConfig.MergeMode = false;
	fontConfig.PixelSnapH = true;
	fontConfig.OversampleH = fontConfig.OversampleV = 1;
	fontConfig.GlyphMinAdvanceX = 4.0f;
	fontConfig.SizePixels = 12.0f;
	// TODO: Fix this path
	s_defaultFont = io.Fonts->AddFontFromFileTTF("../PrismCore/assets/JetBrainsMono-Regular.ttf", 16.f, &fontConfig);
	loadIconFont(16.f);
	s_smallFont = io.Fonts->AddFontFromFileTTF("../PrismCore/assets/JetBrainsMono-Regular.ttf", 12.f, &fontConfig);
	loadIconFont(12.f);
	s_boldFont = io.Fonts->AddFontFromFileTTF("../PrismCore/assets/JetBrainsMono-Bold.ttf", 16.f, &fontConfig);
	loadIconFont(16.f);

	io.Fonts->Build();
}
}
