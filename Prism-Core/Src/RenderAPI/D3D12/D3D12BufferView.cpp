#include "pcpch.h"
#include "D3D12BufferView.h"

#include "RenderAPI/D3D12/D3D12Buffer.h"
#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"

namespace Prism::Render::D3D12
{
D3D12BufferView::D3D12BufferView(const BufferViewDesc& desc, Buffer* buffer)
	: m_viewDesc(desc)
{
	PE_ASSERT(buffer);
	PE_ASSERT(m_viewDesc.size + m_viewDesc.offset <= buffer->GetBufferDesc().size);

	m_owningBuffer = buffer;

	m_descriptor = D3D12RenderDevice::Get().AllocateCPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	auto* d3d12Buffer = static_cast<D3D12Buffer*>(m_owningBuffer.Raw());
	if (d3d12Buffer->GetBufferDesc().usage != ResourceUsage::Dynamic)
	{
		switch (m_viewDesc.type)
		{
		case BufferViewType::CBV:
			{
				auto d3d12ViewDesc = GetD3D12ConstantBufferViewDesc(m_owningBuffer, m_viewDesc);
				D3D12RenderDevice::Get().GetD3D12Device()->CreateConstantBufferView(&d3d12ViewDesc, m_descriptor.GetCPUHandle());
			}
			break;
		case BufferViewType::SRV:
			{
				auto d3d12ViewDesc = GetD3D12ShaderResourceViewDesc(m_viewDesc);
				D3D12RenderDevice::Get().GetD3D12Device()->CreateShaderResourceView(
					static_cast<D3D12Buffer*>(m_owningBuffer.Raw())->GetD3D12Resource(),
					&d3d12ViewDesc,
					m_descriptor.GetCPUHandle());
			}
			break;
		case BufferViewType::UAV:
			{
				auto d3d12ViewDesc = GetD3D12UnorderedAccessViewDesc(m_viewDesc);
				D3D12RenderDevice::Get().GetD3D12Device()->CreateUnorderedAccessView(
					static_cast<D3D12Buffer*>(m_owningBuffer.Raw())->GetD3D12Resource(),
					nullptr,
					&d3d12ViewDesc,
					m_descriptor.GetCPUHandle());
			}
			break;
		default:
			PE_ASSERT_NO_ENTRY();
		}

	}
}

void D3D12BufferView::BuildView()
{
	auto* d3d12Buffer = static_cast<D3D12Buffer*>(m_owningBuffer.Raw());
	if (d3d12Buffer->GetBufferDesc().usage == ResourceUsage::Dynamic)
	{
		const auto& allocation = d3d12Buffer->GetDynamicAllocation();

		auto d3d12ViewDesc = GetD3D12ConstantBufferViewDesc(
			allocation.resource,
			{
				.offset = allocation.offset + m_viewDesc.offset,
				.size = m_viewDesc.size
			});
		D3D12RenderDevice::Get().GetD3D12Device()->CreateConstantBufferView(&d3d12ViewDesc, m_descriptor.GetCPUHandle());
	}
}
}
