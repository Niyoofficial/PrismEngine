#include "pcpch.h"

#include "imgui.h"
#include "Prism/Render/RenderDevice.h"
#include "RenderAPI/D3D12/D3D12TextureView.h"

using namespace Prism;

namespace ImGui
{
void Image(Ref<Render::TextureView> texture_view, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
{
	Render::RenderDevice::Get().AddResourceToReleaseQueueWhenFrameEnds(texture_view);
	Image((ImTextureID)static_cast<Render::D3D12::D3D12TextureView*>(texture_view.Raw())->GetDescriptor().GetGPUHandle().ptr, image_size, uv0, uv1, tint_col, border_col);
}

bool ImageButton(const char* str_id, Ref<Render::TextureView> texture_view, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& bg_col, const ImVec4& tint_col)
{
	Render::RenderDevice::Get().AddResourceToReleaseQueueWhenFrameEnds(texture_view);
	return ImageButton(str_id, (ImTextureID)static_cast<Render::D3D12::D3D12TextureView*>(texture_view.Raw())->GetDescriptor().GetGPUHandle().ptr, image_size, uv0, uv1, bg_col, tint_col);
}
}
