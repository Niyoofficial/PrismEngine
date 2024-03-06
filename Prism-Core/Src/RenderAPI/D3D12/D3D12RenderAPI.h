#pragma once
#include "Prism-Core/Render/RenderAPI.h"

#include "RenderAPI/D3D12/D3D12DescriptorHeapManager.h"
#include "RenderAPI/D3D12/D3D12RootSignatureCache.h"
#include "RenderAPI/D3D12/D3D12ShaderCompiler.h"

namespace Prism::Render::D3D12
{
class D3D12RenderAPI : public RenderAPI
{
public:
	static D3D12RenderAPI* Get();

	D3D12RenderAPI();

	ID3D12Device* GetD3DDevice() const;
	IDXGIFactory2* GetDXGIFactory() const;
	ID3D12CommandQueue* GetCommandQueue() const;

	D3D12ShaderCompiler& GetShaderCompiler() { return m_shaderCompiler; }
	const D3D12ShaderCompiler& GetShaderCompiler() const { return m_shaderCompiler; }

	DescriptorHeapAllocation AllocateCPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, int32_t count = 1);

	uint32_t GetDescriptorHandleSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const;

	const D3D12RootSignatureCache& GetRootSignatureCache() const { return m_rootSignatureCache; }
	D3D12RootSignatureCache& GetRootSignatureCache() { return m_rootSignatureCache; }


	virtual void Begin() override;
	virtual void End() override;

	virtual void FlushCommandQueue() override;

	virtual void Draw(DrawCommandDesc desc) override;
	virtual void DrawIndexed(DrawIndexedCommandDesc desc) override;

	virtual void SetPSO(GraphicsPipelineState* pso) override;
	virtual void SetRenderTargets(std::vector<TextureView*> rtvs, TextureView* dsv) override;
	virtual void SetViewports(std::vector<Viewport> viewports) override;
	virtual void SetScissors(std::vector<Scissor> scissors) override;

	virtual void ClearRenderTargetView(TextureView* rtv, glm::float4* clearColor = nullptr) override;
	virtual void ClearDepthStencilView(TextureView* dsv, Flags<ClearFlags> flags, DepthStencilValue* clearValue = nullptr) override;

	virtual void Transition(StateTransitionDesc desc) override;

private:
	ComPtr<IDXGIFactory2> m_dxgiFactory;
	ComPtr<ID3D12Device> m_d3dDevice;

	ComPtr<ID3D12CommandQueue> m_commandQueue;

	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	uint64_t m_mainFenceValue = 0;
	ComPtr<ID3D12Fence> m_mainFence;

	D3D12ShaderCompiler m_shaderCompiler;

	std::unordered_map<D3D12_DESCRIPTOR_HEAP_TYPE, uint32_t> m_descriptorHandleSizes;

	std::unordered_map<D3D12_DESCRIPTOR_HEAP_TYPE, CPUDescriptorHeapManager> m_cpuDescriptorHeapManagers;
	std::unordered_map<D3D12_DESCRIPTOR_HEAP_TYPE, GPUDescriptorHeapManager> m_gpuDescriptorHeapManagers;

	D3D12RootSignatureCache m_rootSignatureCache;
};
}
