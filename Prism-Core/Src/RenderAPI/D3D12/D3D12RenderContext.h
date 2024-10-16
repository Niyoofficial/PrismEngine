#pragma once
#include "Prism-Core/Render/RenderContext.h"
#include "Prism-Core/Render/RenderResourceView.h"
#include "RenderAPI/D3D12/D3D12DescriptorHeapManager.h"
#include "RenderAPI/D3D12/D3D12PipelineState.h"


namespace Prism::Render::D3D12
{
enum class PipelineStateType
{
	Graphics,
	Compute
};

class D3D12RenderContext : public RenderContext
{
public:
	D3D12RenderContext();

	ID3D12CommandAllocator* GetCommandAllocator() const { return m_commandAllocator.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() const { return m_commandList.Get(); }

	virtual void Draw(DrawCommandDesc desc) override;
	virtual void DrawIndexed(DrawIndexedCommandDesc desc) override;

	virtual void Dispatch(int32_t threadGroupCountX, int32_t threadGroupCountY, int32_t threadGroupCountZ) override;

	virtual void SetPSO(GraphicsPipelineState* pso) override;
	virtual void SetPSO(ComputePipelineState* pso) override;

	virtual void SetRenderTargets(std::vector<TextureView*> rtvs, TextureView* dsv) override;
	virtual void SetViewports(std::vector<Viewport> viewports) override;
	virtual void SetScissors(std::vector<Scissor> scissors) override;

	virtual void SetVertexBuffer(Buffer* buffer, int64_t vertexSizeInBytes) override;
	virtual void SetIndexBuffer(Buffer* buffer, IndexBufferFormat format) override;

	virtual void SetTexture(TextureView* textureView, const std::wstring& paramName, int32_t index = 0) override;
	virtual void SetCBuffer(BufferView* bufferView, const std::wstring& paramName) override;

	virtual void ClearRenderTargetView(TextureView* rtv, glm::float4* clearColor = nullptr) override;
	virtual void ClearDepthStencilView(TextureView* dsv, Flags<ClearFlags> flags, DepthStencilValue* clearValue = nullptr) override;

	virtual void Transition(StateTransitionDesc desc) override;

	virtual void UpdateBuffer(Buffer* buffer, RawData data) override;
	virtual void UpdateTexture(Texture* texture, RawData data, int32_t subresourceIndex) override;

	virtual void CopyBufferRegion(Buffer* dest, int32_t destOffset, Buffer* src, int32_t srcOffset, int32_t numBytes) override;
	virtual void CopyTextureRegion(Texture* dest, int32_t x, int32_t y, int32_t z, int32_t subresourceIndex, Buffer* src, int64_t srcOffset = 0) override;

	virtual void CloseContext() override;

private:
	void PrepareDrawOrDispatch(PipelineStateType type);

	CPUDescriptorHeapAllocation GetNullDescriptorForRootParam(class D3D12RootSignature* rootSignature, int32_t rootParamIndex);

	void SetRootParam(int32_t rootIndex, const CPUDescriptorHeapAllocation& cpuAllocation, PipelineStateType type);
	void SetRootParam(int32_t rootIndex, std::span<const CPUDescriptorHeapAllocation*> cpuAllocations, PipelineStateType type);

private:
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	D3D12RootSignature* m_currentRootSig = nullptr;

	std::unordered_map<std::wstring, std::unordered_map<int32_t, RenderResourceView*>> m_rootResources;
	std::vector<GPUDescriptorHeapAllocation> m_gpuDescriptors;
};
}
