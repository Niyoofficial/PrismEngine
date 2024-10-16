#include "pcpch.h"
#include "D3D12Buffer.h"

#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"


namespace Prism::Render::D3D12
{
D3D12Buffer::D3D12Buffer(const BufferDesc& desc, RawData initData, Flags<ResourceStateFlags> initState)
	: m_originalDesc(desc)
{
	// TODO: Implement staging buffers
	PE_ASSERT(desc.usage != ResourceUsage::Staging);

	// Dynamic buffers don't need to be created here, they will be automatically created when Map is called
	if (desc.usage == ResourceUsage::Default)
	{
		auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(
			m_originalDesc.size,
			GetD3D12ResourceFlags(m_originalDesc.bindFlags));
		PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateCommittedResource(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			GetD3D12ResourceStates(ResourceStateFlags::Common),
			nullptr, IID_PPV_ARGS(&m_resource)));

		PE_ASSERT_HR(m_resource->SetName(m_originalDesc.bufferName.c_str()));
	}

	if (initData.data && initData.sizeInBytes > 0)
	{
		if (desc.usage == ResourceUsage::Dynamic)
		{
			// TODO: Maybe add dynamic readback buffers?
			PE_ASSERT(desc.cpuAccess == CPUAccess::Write, "Dynamic buffers must have only the CPUAccess::Write flag set");

			void* data = D3D12Buffer::Map(CPUAccess::Write);
			memcpy_s(data, m_originalDesc.size, initData.data, initData.sizeInBytes);
			D3D12Buffer::Unmap();
		}
		else if (desc.usage == ResourceUsage::Default)
		{
			Ref context = RenderDevice::Get().AllocateContext();

			context->UpdateBuffer(this, initData);

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
	: m_resource(resource), m_originalDesc(D3D12::GetBufferDesc(resource->GetDesc(), name, usage))
{
}

void* D3D12Buffer::Map(Flags<CPUAccess> access)
{
	PE_ASSERT(
		m_originalDesc.usage == ResourceUsage::Dynamic ||
		m_originalDesc.usage == ResourceUsage::Staging,
		"Buffer must be Dynamic or Staging!");

	if (m_originalDesc.usage == ResourceUsage::Dynamic)
		PE_ASSERT(access == CPUAccess::Write, "Dynamic buffers can only be written to");
	else if (m_originalDesc.usage == ResourceUsage::Staging)
		PE_ASSERT(m_originalDesc.cpuAccess == access, "Map access doesn't match the one buffer was created with");

	PE_ASSERT(!m_isMapped, "Can't map a buffer that is already mapped, unmap it first");

	m_isMapped = true;
	m_dynamicAllocation = D3D12RenderDevice::Get().AllocateDynamicBufferMemory(m_originalDesc.size);

	return m_dynamicAllocation.cpuAddress;
}

void D3D12Buffer::Unmap()
{
	m_isMapped = false;
}

BufferDesc D3D12Buffer::GetBufferDesc() const
{
	if (auto* d3d12Resource = GetD3D12Resource())
	{
		auto desc = D3D12::GetBufferDesc(d3d12Resource->GetDesc(), m_originalDesc.bufferName, m_originalDesc.usage);
		if (m_originalDesc.usage == ResourceUsage::Dynamic)
			desc.size = m_originalDesc.size;

		return desc;
	}

	return m_originalDesc;
}

ID3D12Resource* D3D12Buffer::GetD3D12Resource() const
{
	if (m_originalDesc.usage == ResourceUsage::Dynamic)
		return m_dynamicAllocation.resource;
	else
		return m_resource.Get();
}

int64_t D3D12Buffer::GetDefaultOffset() const
{
	if (m_originalDesc.usage == ResourceUsage::Dynamic)
		return m_dynamicAllocation.offset;
	else
		return 0;
}
}
