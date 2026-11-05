#include "Prism/Render/RenderDevice.h"
#include "RenderAPI/Vulkan/VulkanTextureView.h"
#include "imgui.h"

using namespace Prism;

namespace ImGui
{
void Image(Ref<Render::TextureView> texture_view, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1,
           const ImVec4& tint_col, const ImVec4& border_col)
{
	Render::RenderDevice::Get().AddResourceToReleaseQueueWhenFrameEnds(texture_view);

	auto* vkTextureView = static_cast<Render::Vulkan::VulkanTextureView*>(texture_view.Raw());

	Image(reinterpret_cast<ImTextureID>(vkTextureView->GetImGuiDescriptorSet()), image_size, uv0, uv1, tint_col, border_col);
}

void Image(Ref<Render::Texture> texture, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col,
           const ImVec4& border_col)
{
	Render::RenderDevice::Get().AddResourceToReleaseQueueWhenFrameEnds(texture);
	Image(texture->CreateDefaultSRV(), image_size, uv0, uv1, tint_col, border_col);
}

bool ImageButton(const char* str_id, Ref<Render::TextureView> texture_view, const ImVec2& image_size, const ImVec2& uv0,
                 const ImVec2& uv1, const ImVec4& bg_col, const ImVec4& tint_col)
{
	Render::RenderDevice::Get().AddResourceToReleaseQueueWhenFrameEnds(texture_view);

	auto* vkTextureView = static_cast<Render::Vulkan::VulkanTextureView*>(texture_view.Raw());

	return ImageButton(str_id, reinterpret_cast<ImTextureID>(vkTextureView->GetImGuiDescriptorSet()), image_size, uv0, uv1,
	                   bg_col, tint_col);
}

bool ImageButton(const char* str_id, Ref<Render::Texture> texture, const ImVec2& image_size, const ImVec2& uv0, const ImVec2& uv1,
                 const ImVec4& bg_col, const ImVec4& tint_col)
{
	Render::RenderDevice::Get().AddResourceToReleaseQueueWhenFrameEnds(texture);
	return ImageButton(str_id, texture->CreateDefaultSRV(), image_size, uv0, uv1, bg_col, tint_col);
}
} // namespace ImGui
