#pragma once
#include "Prism/Render/BufferView.h"
#include "RenderAPI/D3D12/D3D12DescriptorHeapManager.h"
#include "RenderAPI/D3D12/D3D12DynamicGPURingBuffer.h"

namespace Prism::Render::D3D12
{
class D3D12BufferView : public BufferView
{
public:
	D3D12BufferView(const BufferViewDesc& desc, Buffer* buffer);

	virtual BufferViewDesc GetViewDesc() const override { return m_viewDesc; }

	void BuildDynamicDescriptor(CD3DX12_CPU_DESCRIPTOR_HANDLE d3d12DescriptorHandle);

	const DescriptorHeapAllocation& GetDescriptor() const { return m_descriptor; }

	D3D12_DESCRIPTOR_HEAP_TYPE GetDescriptorHeapType() const;

private:
	BufferViewDesc m_viewDesc;
	DescriptorHeapAllocation m_descriptor;

	ComPtr<ID3D12Resource> m_counterBuffer;
};
}
