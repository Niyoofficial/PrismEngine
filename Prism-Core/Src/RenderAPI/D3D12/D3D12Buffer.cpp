#include "pcpch.h"
#include "D3D12Buffer.h"

#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"


namespace Prism::Render::D3D12
{
D3D12Buffer::D3D12Buffer(const BufferDesc& desc, BufferData initData, Flags<ResourceStateFlags> initState)
	: m_originalDesc(desc)
{
	auto actualInitState = desc.usage == ResourceUsage::Default && initData.data ? ResourceStateFlags::CopyDest : initState;
	actualInitState = ResourceStateFlags::Common;

	auto heapProps = CD3DX12_HEAP_PROPERTIES(m_originalDesc.usage == ResourceUsage::Default
												 ? D3D12_HEAP_TYPE_DEFAULT
												 : D3D12_HEAP_TYPE_UPLOAD);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(m_originalDesc.size, GetD3D12ResourceFlags(m_originalDesc.bindFlags));
	PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		GetD3D12ResourceStates(actualInitState),
		nullptr, IID_PPV_ARGS(&m_resource)));

	PE_ASSERT_HR(m_resource->SetName(m_originalDesc.bufferName.c_str()));

	if (initData.data)
	{
		if (desc.usage == ResourceUsage::Dynamic)
		{
			auto data = D3D12Buffer::Map(CPUAccess::Write);
			memcpy_s(data, m_originalDesc.size, initData.data, initData.sizeInBytes);
			D3D12Buffer::Unmap();
		}
		else if (desc.usage == ResourceUsage::Default)
		{
			Ref context = RenderDevice::Get().AllocateContext();

			context->UpdateBuffer(this, initData);

			// TODO
			/*context->Transition({
				.resource = this,
				.oldState = ResourceStateFlags::CopyDest,
				.newState = initState
			});*/

			RenderDevice::Get().SubmitContext(context);

			RenderDevice::Get().FlushCommandQueue();
		}
	}
}

D3D12Buffer::D3D12Buffer(ID3D12Resource* resource, const std::wstring& name, ResourceUsage usage)
	: m_originalDesc(D3D12::GetBufferDesc(resource->GetDesc(), name, usage)), m_resource(resource)
{
}

void* D3D12Buffer::Map(CPUAccess access)
{
	PE_ASSERT(
		m_originalDesc.usage == ResourceUsage::Dynamic ||
		m_originalDesc.usage == ResourceUsage::Staging,
		"Buffer must be Dynamic or Staging!");

	if (m_originalDesc.usage == ResourceUsage::Dynamic)
		PE_ASSERT(access == CPUAccess::Write, "Dynamic buffers can only be written to");
	else if (m_originalDesc.usage == ResourceUsage::Staging)
		PE_ASSERT(m_originalDesc.cpuAccess == access, "Map access doesn't match the one buffer was created with");

	void* mappedData = nullptr;
	PE_ASSERT_HR(m_resource->Map(0, nullptr, &mappedData));

	return mappedData;
}

void D3D12Buffer::Unmap()
{
	m_resource->Unmap(0, nullptr);
}

BufferDesc D3D12Buffer::GetBufferDesc() const
{
	return D3D12::GetBufferDesc(m_resource->GetDesc(), m_originalDesc.bufferName, m_originalDesc.usage);
}
}
