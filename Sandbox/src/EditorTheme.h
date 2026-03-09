#pragma once

#include "Prism/UI/PrismImGui.h"

namespace Prism
{
struct EditorTheme
{
	static void Init();

	inline static ImVec4 s_headerSelectedColor;
	inline static ImVec4 s_headerHoveredColor;
	inline static ImVec4 s_windowBgColor;
	inline static ImVec4 s_windowBgAlternativeColor;
	inline static ImVec4 s_assetIconColor;
	inline static ImVec4 s_textColor;
	inline static ImVec4 s_textDisabledColor;

	inline static ImVec2 s_framePadding;
	inline static ImVec2 s_popupItemSpacing;

	static ImFont* s_defaultFont;
	static ImFont* s_smallFont;
	static ImFont* s_boldFont;
};
}
