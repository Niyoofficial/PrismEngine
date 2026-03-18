#pragma once

#include "Prism/UI/PrismImGui.h"

namespace Prism
{
struct EditorTheme
{
	static void Init();

	inline static glm::float4 s_headerSelectedColor;
	inline static glm::float4 s_headerHoveredColor;
	inline static glm::float4 s_windowBgColor;
	inline static glm::float4 s_windowBgAlternativeColor;
	inline static glm::float4 s_assetIconColor;
	inline static glm::float4 s_textColor;
	inline static glm::float4 s_textDisabledColor;

	inline static glm::float2 s_framePadding;
	inline static glm::float2 s_popupItemSpacing;

	static ImFont* s_defaultFont;
	static ImFont* s_smallFont;
	static ImFont* s_boldFont;
};
}
