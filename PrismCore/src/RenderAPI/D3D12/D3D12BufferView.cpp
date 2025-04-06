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

	m_descriptor = D3D12RenderDevice::Get().AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	auto* d3d12Buffer = static_cast<D3D12Buffer*>(m_owningBuffer.Raw());
	if (d3d12Buffer->GetBufferDesc().usage != ResourceUsage::Dynamic)
	{
		switch (m_viewDesc.type)
		{
		case BufferViewType::CBV:
			{
				PE_ASSERT(!m_viewDesc.flags.HasAllFlags(BufferViewFlags::NeedsCounter), "NeedsCounter flag can only be used on UAV");

				auto d3d12ViewDesc = GetD3D12ConstantBufferViewDesc(m_owningBuffer, m_viewDesc);
				D3D12RenderDevice::Get().GetD3D12Device()->CreateConstantBufferView(&d3d12ViewDesc, m_descriptor.GetCPUHandle());
			}
			break;
		case BufferViewType::SRV:
			{
				PE_ASSERT(!m_viewDesc.flags.HasAllFlags(BufferViewFlags::NeedsCounter), "NeedsCounter flag can only be used on UAV");

				auto d3d12ViewDesc = GetD3D12ShaderResourceViewDesc(m_viewDesc);
				D3D12RenderDevice::Get().GetD3D12Device()->CreateShaderResourceView(
					static_cast<D3D12Buffer*>(m_owningBuffer.Raw())->GetD3D12Resource(),
					&d3d12ViewDesc,
					m_descriptor.GetCPUHandle());
			}
			break;
		case BufferViewType::UAV:
			{
				if (m_viewDesc.flags.HasAllFlags(BufferViewFlags::NeedsCounter))
				{
					// Counter buffer
					auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
					auto bufferDesc = CD3DX12_RESOURCE_DESC1::Buffer(
						32,
						D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
					PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateCommittedResource3(
						&heapProps,
						D3D12_HEAP_FLAG_NONE,
						&bufferDesc,
						D3D12_BARRIER_LAYOUT_UNDEFINED,
						nullptr, nullptr,
						0, nullptr,
						IID_PPV_ARGS(&m_counterBuffer)));

					PE_ASSERT_HR(m_counterBuffer->SetName((buffer->GetBufferDesc().bufferName + L"_CounterBuffer").c_str()));
				}

				auto d3d12ViewDesc = GetD3D12UnorderedAccessViewDesc(m_viewDesc);
				D3D12RenderDevice::Get().GetD3D12Device()->CreateUnorderedAccessView(
					static_cast<D3D12Buffer*>(m_owningBuffer.Raw())->GetD3D12Resource(),
					m_counterBuffer.Get(),
					&d3d12ViewDesc,
					m_descriptor.GetCPUHandle());
			}
			break;
		default:
			PE_ASSERT_NO_ENTRY();
		}
	}
}

void D3D12BufferView::BuildDynamicDescriptor(CD3DX12_CPU_DESCRIPTOR_HANDLE d3d12DescriptorHandle)
{
	PE_ASSERT(m_owningBuffer->GetBufferDesc().usage == ResourceUsage::Dynamic);
	auto* d3d12Buffer = static_cast<D3D12Buffer*>(m_owningBuffer.Raw());
	if (d3d12Buffer->GetBufferDesc().usage == ResourceUsage::Dynamic)
	{
		auto allocation = d3d12Buffer->GetDynamicAllocation();

		auto d3d12ViewDesc = GetD3D12ConstantBufferViewDesc(
			allocation.resource,
			{
				.offset = allocation.offset + m_viewDesc.offset,
				.size = m_viewDesc.size
			});
		D3D12RenderDevice::Get().GetD3D12Device()->CreateConstantBufferView(&d3d12ViewDesc, d3d12DescriptorHandle);
	}
}

D3D12_DESCRIPTOR_HEAP_TYPE D3D12BufferView::GetDescriptorHeapType() const
{
	return GetD3D12DescriptorHeapType(m_viewDesc.type);
}
}
