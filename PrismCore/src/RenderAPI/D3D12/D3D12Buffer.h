#pragma once
#include "Prism/Render/Buffer.h"
#include "RenderAPI/D3D12/D3D12Base.h"
#include "RenderAPI/D3D12/D3D12DynamicGPURingBuffer.h"

namespace Prism::Render::D3D12
{
class D3D12Buffer : public Buffer
{
public:
	D3D12Buffer(const BufferDesc& desc);
	D3D12Buffer(ID3D12Resource* resource, const std::wstring& name, ResourceUsage usage, CPUAccess cpuAccess);

	virtual void* Map(Flags<CPUAccess> access) override;
	virtual void Unmap() override;

	virtual BufferDesc GetBufferDesc() const override;

	ID3D12Resource* GetD3D12Resource() const;
	// Since dynamic resources are allocated from a different resource,
	// this will return the offset where this buffers actually starts in the resource,
	// this will return 0 for everything else
	int64_t GetDefaultOffset() const;

	DynamicGPURingBuffer::DynamicAllocation GetDynamicAllocation() const { return m_dynamicAllocation; }

private:
	ComPtr<ID3D12Resource> m_resource;
	BufferDesc m_originalDesc;

	bool m_isMapped = false;
	DynamicGPURingBuffer::DynamicAllocation m_dynamicAllocation;
};
}
