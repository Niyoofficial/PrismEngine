#pragma once

#include <vulkan/vulkan_core.h>
#include "Prism/Render/RenderCommandList.h"

namespace Prism::Render::Vulkan
{
struct PendingColorClear
{
	Ref<TextureView> view;
	VkClearValue clearValue;
};

struct PendingDepthClear
{
	Ref<TextureView> view;
	VkClearValue clearValue;
	Flags<ClearFlags> flags;
};

class VulkanRenderCommandList : public RenderCommandList
{
public:
	explicit VulkanRenderCommandList();
	~VulkanRenderCommandList() override;

	void Draw(DrawCommandDesc desc) override;
	void DrawIndexed(DrawIndexedCommandDesc desc) override;

	void Dispatch(glm::int3 threadGroupCount) override;

	void SetPSO(const GraphicsPipelineStateDesc& desc) override;
	void SetPSO(const ComputePipelineStateDesc& desc) override;

	void SetStencilRef(uint32_t ref) override;

	void SetRenderTargets(std::vector<Ref<TextureView>> rtvs, const Ref<TextureView>& dsv) override;
	void SetViewports(std::vector<Viewport> viewports) override;
	void SetScissors(std::vector<Scissor> scissors) override;

	void SetVertexBuffer(const Ref<Buffer>& buffer, int64_t vertexSizeInBytes) override;
	void SetIndexBuffer(const Ref<Buffer>& buffer, IndexBufferFormat format) override;

	void SetTextures(const std::vector<Ref<TextureView>>& textureViews, const std::wstring& paramName) override;
	void SetBuffers(const std::vector<Ref<BufferView>>& bufferViews, const std::vector<std::any> dynamicAllocations,
	                const std::wstring& paramName) override;

	void ClearRenderTargetView(const Ref<TextureView>& rtv, glm::float4* clearColor) override;
	void ClearDepthStencilView(const Ref<TextureView>& dsv, Flags<ClearFlags> flags, DepthStencilValue* clearValue) override;

	void ClearUnorderedAccessView(const Ref<TextureView>& uav, glm::float4 values) override;
	void ClearUnorderedAccessView(const Ref<TextureView>& uav, glm::uint4 values) override;

	void Barrier(BufferBarrier barrier) override;
	void Barrier(TextureBarrier barrier) override;

	void UpdateBuffer(const Ref<Buffer>& buffer, RawData data) override;
	void UpdateTexture(const Ref<Texture>& texture, RawData data, int32_t subresourceIndex) override;

	void CopyBufferRegion(const Ref<Buffer>& dest, int64_t destOffset, const Ref<Buffer>& src, int64_t srcOffset,
	                      int64_t numBytes) override;
	void CopyBufferRegion(const Ref<Texture>& dest, glm::int3 destLoc, int32_t destSubresourceIndex, const Ref<Buffer>& src,
	                      int64_t srcOffset) override;
	void CopyTextureRegion(const Ref<Buffer>& dest, int64_t destOffset, const Ref<Texture>& src, int32_t srcSubresourceIndex,
	                       Box3I srcBox) override;
	void CopyTextureRegion(const Ref<Texture>& dest, glm::int3 destLoc, int32_t destSubresourceIndex, const Ref<Texture>& src,
	                       int32_t srcSubresourceIndex, Box3I srcBox) override;

	void RenderImGui(Swapchain* swapchain, int32_t backbufferIndex, ImDrawData* drawData) override;

	void SetMarker(glm::float3 color, std::wstring string) override;
	void BeginEvent(glm::float3 color, std::wstring string) override;
	void EndEvent() override;

	[[nodiscard]] VkCommandBuffer GetVkCommandBuffer() const { return m_commandBuffer; }

private:
	void BindDescriptorSets(PipelineStateType type);

	void SetupDrawOrDispatch(PipelineStateType type);

	void BeginDynamicRendering();

	void EndDynamicRendering();

	VkCommandPool m_commandPool = VK_NULL_HANDLE;
	VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;

	GraphicsPipelineStateDesc m_currentGraphicsPSO;
	ComputePipelineStateDesc m_currentComputePSO;

	std::vector<Ref<TextureView>> m_renderTargetViews;
	Ref<TextureView> m_depthStencilView;

	std::vector<PendingColorClear> m_pendingColorClears;
	std::optional<PendingDepthClear> m_pendingDepthClear;

	bool m_renderingActive = false;

	std::unordered_map<std::wstring, std::vector<Ref<RenderResourceView>>> m_boundResources;
};
} // namespace Prism::Render::Vulkan
