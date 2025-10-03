#pragma once
#include "Prism/Base/AppEvents.h"
#include "Prism/Base/AppEvents.h"
#include "Prism/Render/RenderCommandList.h"
#include "Prism/Render/RenderResourceView.h"
#include "RenderAPI/D3D12/D3D12Base.h"
#include "RenderAPI/D3D12/D3D12DescriptorHeapManager.h"
#include "RenderAPI/D3D12/D3D12PipelineStateCache.h"

namespace Prism::Render::D3D12
{
class D3D12RenderCommandList : public RenderCommandList
{
public:
	explicit D3D12RenderCommandList();

	virtual void Draw(DrawCommandDesc desc) override;
	virtual void DrawIndexed(DrawIndexedCommandDesc desc) override;
	virtual void Dispatch(glm::int3 threadGroupCount) override;

	virtual void SetPSO(const GraphicsPipelineStateDesc& desc) override;
	virtual void SetPSO(const ComputePipelineStateDesc& desc) override;

	virtual void SetRenderTargets(std::vector<TextureView*> rtvs, TextureView* dsv) override;

	virtual void SetViewports(std::vector<Viewport> viewports) override;
	virtual void SetScissors(std::vector<Scissor> scissors) override;

	virtual void SetVertexBuffer(Buffer* buffer, int64_t vertexSizeInBytes) override;
	virtual void SetIndexBuffer(Buffer* buffer, IndexBufferFormat format) override;

	virtual void SetTexture(TextureView* textureView, const std::wstring& paramName) override;
	virtual void SetBuffer(BufferView* bufferView, const std::wstring& paramName) override;

	virtual void ClearRenderTargetView(TextureView* rtv, glm::float4* clearColor) override;
	virtual void ClearDepthStencilView(TextureView* dsv, Flags<ClearFlags> flags, DepthStencilValue* clearValue) override;

	virtual void Barrier(BufferBarrier barrier) override;
	virtual void Barrier(TextureBarrier barrier) override;

	virtual void UpdateBuffer(Buffer* buffer, RawData data) override;
	virtual void UpdateTexture(Texture* texture, RawData data, int32_t subresourceIndex) override;

	virtual void CopyBufferRegion(Buffer* dest, int64_t destOffset, Buffer* src, int64_t srcOffset, int64_t numBytes) override;
	virtual void CopyBufferRegion(Texture* dest, glm::int3 destLoc, int32_t destSubresourceIndex, Buffer* src, int64_t srcOffset) override;
	virtual void CopyTextureRegion(Buffer* dest, int64_t destOffset,
								   Texture* src, int32_t srcSubresourceIndex, Box3I srcBox) override;
	virtual void CopyTextureRegion(Texture* dest, glm::int3 destLoc, int32_t destSubresourceIndex,
								   Texture* src, int32_t srcSubresourceIndex, Box3I srcBox) override;

	virtual void RenderImGui() override;

	virtual void SetMarker(glm::float3 color, std::wstring string) override;
	virtual void BeginEvent(glm::float3 color, std::wstring string) override;
	virtual void EndEvent() override;

	virtual void Close() override;

	ID3D12GraphicsCommandList7* GetD3D12CommandList() const { return m_commandList.Get(); }

private:
	void SetupDrawOrDispatch(PipelineStateType type);

private:
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList7> m_commandList;

	GraphicsPipelineStateDesc m_currentGraphicsPSO = {};
	ComputePipelineStateDesc m_currentComputePSO = {};

	std::vector<Ref<TextureView>> m_renderTargetViews;
	Ref<TextureView> m_depthStencilView;

	std::unordered_map<std::wstring, Ref<RenderResourceView>> m_rootResources;
	std::vector<Ref<RenderResourceView>> m_overriddenRootResources;
	std::vector<DescriptorHeapAllocation> m_dynamicDescriptors;
};
}
