#pragma once
#include "Prism-Core/Render/RenderAPI.h"

#include "D3D12Base.h"
#include "RenderAPI/D3D12/D3D12DescriptorHeapManager.h"
#include "RenderAPI/D3D12/D3D12RootSignatureCache.h"
#include "RenderAPI/D3D12/D3D12ShaderCompiler.h"

namespace Prism::Render
{
class Renderer;
}

namespace Prism::D3D12
{
class D3D12RenderAPI : public Render::RenderAPI
{
public:
	static D3D12RenderAPI* Get();

	D3D12RenderAPI();

	ID3D12Device* GetD3DDevice() const;
	IDXGIFactory* GetDXGIFactory() const;
	ID3D12CommandQueue* GetCommandQueue() const;

	D3D12ShaderCompiler& GetShaderCompiler() { return m_shaderCompiler; }
	const D3D12ShaderCompiler& GetShaderCompiler() const { return m_shaderCompiler; }

	CPUDescriptorHeapManager& GetCPUDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type);
	const CPUDescriptorHeapManager& GetCPUDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type) const;
	GPUDescriptorHeapManager& GetGPUDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type);
	const GPUDescriptorHeapManager& GetGPUDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type) const;
	uint32_t GetDescriptorHandleSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const;

	const D3D12RootSignatureCache& GetRootSignatureCache() const { return m_rootSignatureCache; }
	D3D12RootSignatureCache& GetRootSignatureCache() { return m_rootSignatureCache; }


	virtual void Begin() override;
	virtual void End() override;

	virtual void Draw(Render::DrawCommandDesc desc) override;
	virtual void DrawIndexed(Render::DrawIndexedCommandDesc desc) override;

	virtual void SetPSO(Render::GraphicsPipelineState* pso) override;

private:
	ComPtr<IDXGIFactory> m_dxgiFactory;
	ComPtr<ID3D12Device> m_d3dDevice;

	ComPtr<ID3D12CommandQueue> m_commandQueue;

	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	D3D12ShaderCompiler m_shaderCompiler;

	std::unordered_map<D3D12_DESCRIPTOR_HEAP_TYPE, uint32_t> m_descriptorHandleSizes;

	std::unordered_map<D3D12_DESCRIPTOR_HEAP_TYPE, CPUDescriptorHeapManager> m_cpuDescriptorHeapManagers;
	std::unordered_map<D3D12_DESCRIPTOR_HEAP_TYPE, GPUDescriptorHeapManager> m_gpuDescriptorHeapManagers;

	D3D12RootSignatureCache m_rootSignatureCache;
};
}
