#include "pcpch.h"

#include "imgui.h"
#include "Prism/Render/RenderDevice.h"
#include "RenderAPI/D3D12/D3D12TextureView.h"

using namespace Prism;

namespace ImGui
{
void Image(Prism::Ref<Prism::Render::TextureView> texture_view, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
{
	Image(texture_view.Raw(), image_size, uv0, uv1, tint_col, border_col);
}

void Image(Render::TextureView* texture_view, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
{
	Ref texture_view_ref = texture_view;
	Render::RenderDevice::Get().AddResourceToReleaseQueueWhenFrameEnds(texture_view_ref);
	Image((ImTextureID)static_cast<Render::D3D12::D3D12TextureView*>(texture_view)->GetDescriptor().GetGPUHandle().ptr, image_size, uv0, uv1, tint_col, border_col);
}

bool ImageButton(const char* str_id, Prism::Ref<Prism::Render::TextureView> texture_view, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& bg_col, const ImVec4& tint_col)
{
	return ImageButton(str_id, texture_view.Raw(), image_size, uv0, uv1, bg_col, tint_col);
}

bool ImageButton(const char* str_id, Render::TextureView* texture_view, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& bg_col, const ImVec4& tint_col)
{
	Ref texture_view_ref = texture_view;
	Render::RenderDevice::Get().AddResourceToReleaseQueueWhenFrameEnds(texture_view_ref);
	return ImageButton(str_id, (ImTextureID)static_cast<Render::D3D12::D3D12TextureView*>(texture_view)->GetDescriptor().GetGPUHandle().ptr, image_size, uv0, uv1, bg_col, tint_col);
}
}
