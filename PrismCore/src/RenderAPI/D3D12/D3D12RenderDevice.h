#pragma once
#define USE_PIX 1

#include "Prism/Render/RenderDevice.h"

#include "RenderAPI/D3D12/D3D12DescriptorHeapManager.h"
#include "RenderAPI/D3D12/D3D12ShaderCompiler.h"

#include "RenderAPI/D3D12/D3D12Base.h"
#include "RenderAPI/D3D12/D3D12DynamicBufferAllocator.h"


namespace Prism::Render::D3D12
{
class D3D12RootSignature;
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

	DescriptorHeapAllocation AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, int32_t count = 1);

	std::array<ID3D12DescriptorHeap*, 2> GetGPUDescriptorHeaps() const;

	uint32_t GetDescriptorHandleSize(D3D12_DESCRIPTOR_HEAP_TYPE type) const;

	D3D12RootSignature* GetGlobalRootSignature(PipelineStateType type) const;

	DynamicGPURingBuffer::DynamicAllocation AllocateDynamicBufferMemory(int64_t size);

	virtual void InitializeImGui(Core::Window* window, TextureFormat depthFormat) override;
	virtual void ShutdownImGui() override;

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

	Ref<D3D12RootSignature> m_graphicsRootSignature;
	Ref<D3D12RootSignature> m_computeRootSignature;

	DynamicBufferAllocator m_dynamicBufferAllocator;

	bool initializedImGui = false;
	std::vector<DescriptorHeapAllocation> m_imGuiAllocations;
};
}
