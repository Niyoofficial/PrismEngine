#pragma once
#include "Prism-Core/Render/RenderDevice.h"

#include "RenderAPI/D3D12/D3D12DescriptorHeapManager.h"
#include "RenderAPI/D3D12/D3D12RootSignatureCache.h"
#include "RenderAPI/D3D12/D3D12ShaderCompiler.h"

#include "RenderAPI/D3D12/D3D12Base.h"


namespace Prism::Render::D3D12
{
class D3D12RenderDevice : public RenderDevice
{
public:
	static D3D12RenderDevice& Get();
	static D3D12RenderDevice* TryGet();


	explicit D3D12RenderDevice(RenderDeviceParams params);
	virtual ~D3D12RenderDevice();

	virtual void SubmitContext(RenderContext* context) override;

	virtual void FlushCommandQueue() override;

	virtual uint64_t GetCompletedCommandListFenceValue() const override;

	ID3D12Device* GetD3D12Device() const;
	IDXGIFactory2* GetDXGIFactory() const;
	ID3D12CommandQueue* GetD3D12CommandQueue() const;

	D3D12ShaderCompiler& GetShaderCompiler() { return m_shaderCompiler; }
	const D3D12ShaderCompiler& GetShaderCompiler() const { return m_shaderCompiler; }

	CPUDescriptorHeapAllocation AllocateCPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, int32_t count = 1);
	GPUDescriptorHeapAllocation CopyToGPUHeap(const CPUDescriptorHeapAllocation& cpuAllocation);

	std::array<ID3D12DescriptorHeap*, 2> GetGPUDescriptorHeaps() const;

	uint32_t GetDescriptorHandleSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const;

	const D3D12RootSignatureCache& GetRootSignatureCache() const { return m_rootSignatureCache; }
	D3D12RootSignatureCache& GetRootSignatureCache() { return m_rootSignatureCache; }

private:
	HMODULE m_pixGpuCaptureModule = {};
	HMODULE m_pixTimingCaptureModule = {};

	ComPtr<ID3D12CommandQueue> m_commandQueue;

	ComPtr<IDXGIFactory2> m_dxgiFactory;
	ComPtr<ID3D12Device> m_d3dDevice;

	D3D12ShaderCompiler m_shaderCompiler;

	std::unordered_map<D3D12_DESCRIPTOR_HEAP_TYPE, uint32_t> m_descriptorHandleSizes;

	std::unordered_map<D3D12_DESCRIPTOR_HEAP_TYPE, CPUDescriptorHeapManager> m_cpuDescriptorHeapManagers;
	std::unordered_map<D3D12_DESCRIPTOR_HEAP_TYPE, GPUDescriptorHeapManager> m_gpuDescriptorHeapManagers;

	D3D12RootSignatureCache m_rootSignatureCache;

	uint64_t m_mainFenceValue = 0;
	ComPtr<ID3D12Fence> m_mainFence;

	ReleaseQueue<Ref<RenderContext>> m_contextReleaseQueue;
};
}
