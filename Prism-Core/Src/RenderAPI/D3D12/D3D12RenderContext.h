#pragma once
#include "Prism-Core/Render/RenderContext.h"
#include "Prism-Core/Render/RenderResourceView.h"
#include "RenderAPI/D3D12/D3D12DescriptorHeapManager.h"
#include "RenderAPI/D3D12/D3D12GraphicsPipelineState.h"


namespace Prism::Render::D3D12
{
class D3D12RenderContext : public RenderContext
{
public:
	D3D12RenderContext();

	ID3D12CommandAllocator* GetCommandAllocator() const { return m_commandAllocator.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() const { return m_commandList.Get(); }

	virtual void Draw(DrawCommandDesc desc) override;
	virtual void DrawIndexed(DrawIndexedCommandDesc desc) override;

	virtual void SetPSO(GraphicsPipelineState* pso) override;

	virtual void SetRenderTargets(std::vector<TextureView*> rtvs, TextureView* dsv) override;
	virtual void SetViewports(std::vector<Viewport> viewports) override;
	virtual void SetScissors(std::vector<Scissor> scissors) override;

	virtual void SetVertexBuffer(Buffer* buffer, int32_t vertexSizeInBytes) override;
	virtual void SetIndexBuffer(Buffer* buffer, IndexBufferFormat format) override;

	virtual void SetTexture(TextureView* textureView, const std::wstring& paramName) override;
	virtual void SetCBuffer(BufferView* bufferView, const std::wstring& paramName) override;

	virtual void ClearRenderTargetView(TextureView* rtv, glm::float4* clearColor = nullptr) override;
	virtual void ClearDepthStencilView(TextureView* dsv, Flags<ClearFlags> flags, DepthStencilValue* clearValue = nullptr) override;

	virtual void Transition(StateTransitionDesc desc) override;

	virtual void UpdateBuffer(Buffer* buffer, BufferData data) override;

	virtual void CopyBufferRegion(Buffer* dest, int32_t destOffset, Buffer* src, int32_t srcOffset, int32_t numBytes) override;

	virtual void CloseContext() override;

private:
	void PrepareDraw();

	void SetResource(RenderResourceView* view, const std::wstring& paramName);

	RenderResourceView* GetResourceView(const std::wstring& name);

private:
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	D3D12GraphicsPipelineState* m_currentPSO = nullptr;
	class D3D12RootSignature* m_currentRootSig = nullptr;

	std::unordered_map<std::wstring, RenderResourceView*> m_rootResources;
	std::vector<GPUDescriptorHeapAllocation> m_gpuDescriptors;
};
}
