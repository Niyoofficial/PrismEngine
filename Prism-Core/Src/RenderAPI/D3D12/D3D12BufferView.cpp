#include "pcpch.h"
#include "D3D12BufferView.h"

#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"

namespace Prism::Render::D3D12
{
D3D12BufferView::D3D12BufferView(const BufferViewDesc& desc, Buffer* buffer)
	: m_owningBuffer(buffer)
{
	auto d3d12ViewDesc = GetD3D12ConstantBufferViewDesc(buffer, desc);

	m_descriptor = D3D12RenderDevice::Get().AllocateCPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	D3D12RenderDevice::Get().GetD3D12Device()->CreateConstantBufferView(&d3d12ViewDesc, m_descriptor.GetCPUHandle());
}

Buffer* D3D12BufferView::GetBuffer() const
{
	return m_owningBuffer;
}
}
