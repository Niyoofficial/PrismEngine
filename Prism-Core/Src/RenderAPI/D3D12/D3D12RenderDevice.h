﻿#pragma once

#include <span>

#include "Prism-Core/Render/RenderDevice.h"

#include "RenderAPI/D3D12/D3D12DescriptorHeapManager.h"
#include "RenderAPI/D3D12/D3D12RootSignatureCache.h"
#include "RenderAPI/D3D12/D3D12ShaderCompiler.h"

#include "RenderAPI/D3D12/D3D12Base.h"
#include "RenderAPI/D3D12/D3D12DynamicBufferAllocator.h"


namespace Prism::Render::D3D12
{
class D3D12RenderCommandQueue;

class D3D12RenderDevice : public RenderDevice
{
public:
	static D3D12RenderDevice& Get();
	static D3D12RenderDevice* TryGet();


	explicit D3D12RenderDevice(RenderDeviceParams params);
	virtual ~D3D12RenderDevice() override;

	virtual void ImGuiNewFrame() override;

	RenderCommandQueue* GetRenderQueue() const override;

	virtual void ReleaseStaleResources() override;

	virtual int64_t GetTotalSizeInBytes(TextureDesc texDesc, int32_t firstSubresource, int32_t numSubresources) const override;
	virtual int64_t GetTotalSizeInBytes(BufferDesc buffDesc) const override;
	virtual SubresourceFootprint GetSubresourceFootprint(TextureDesc texDesc, int32_t subresourceIndex) const override;
	virtual int64_t GetTexturePitchAlignment() const override;

	ID3D12Device10* GetD3D12Device() const;
	IDXGIFactory2* GetDXGIFactory() const;
	ID3D12CommandQueue* GetD3D12CommandQueue() const;

	D3D12ShaderCompiler& GetShaderCompiler() { return m_shaderCompiler; }
	const D3D12ShaderCompiler& GetShaderCompiler() const { return m_shaderCompiler; }

	CPUDescriptorHeapAllocation AllocateCPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, int32_t count = 1);
	GPUDescriptorHeapAllocation CopyToGPUHeap(const CPUDescriptorHeapAllocation& cpuAllocation);
	// All descriptors from the span will be grouped together when copied to GPU
	// All cpu descriptors must be of the same heap type
	GPUDescriptorHeapAllocation CopyToGPUHeap(std::span<const CPUDescriptorHeapAllocation*> cpuAllocations);

	std::array<ID3D12DescriptorHeap*, 2> GetGPUDescriptorHeaps() const;

	uint32_t GetDescriptorHandleSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const;

	const D3D12RootSignatureCache& GetRootSignatureCache() const { return m_rootSignatureCache; }
	D3D12RootSignatureCache& GetRootSignatureCache() { return m_rootSignatureCache; }

	DynamicGPURingBuffer::DynamicAllocation AllocateDynamicBufferMemory(int64_t size);

	virtual void InitializeImGui(Core::Window* window) override;
	virtual void ShutdownImGui() override;

	ComPtr<ID3D12DescriptorHeap> GetImGuiDescriptorHeap() const { return m_imGuiHeap; }

private:
#if USE_PIX
	HMODULE m_pixGpuCaptureModule = {};
	HMODULE m_pixTimingCaptureModule = {};
#endif

	ComPtr<IDXGIFactory2> m_dxgiFactory;
	ComPtr<ID3D12Device10> m_d3dDevice;

	D3D12ShaderCompiler m_shaderCompiler;

	std::unordered_map<D3D12_DESCRIPTOR_HEAP_TYPE, uint32_t> m_descriptorHandleSizes;

	std::unordered_map<D3D12_DESCRIPTOR_HEAP_TYPE, CPUDescriptorHeapManager> m_cpuDescriptorHeapManagers;
	std::unordered_map<D3D12_DESCRIPTOR_HEAP_TYPE, GPUDescriptorHeapManager> m_gpuDescriptorHeapManagers;

	Ref<D3D12RenderCommandQueue> m_commandQueue;

	D3D12RootSignatureCache m_rootSignatureCache;

	DynamicBufferAllocator m_dynamicBufferAllocator;

	bool initializedImGui = false;
	ComPtr<ID3D12DescriptorHeap> m_imGuiHeap;
};
}
