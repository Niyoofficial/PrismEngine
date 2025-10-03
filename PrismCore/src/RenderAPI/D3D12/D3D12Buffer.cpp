#include "pcpch.h"
#include "D3D12Buffer.h"

#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"


namespace Prism::Render::D3D12
{
D3D12Buffer::D3D12Buffer(const BufferDesc& desc)
	: m_originalDesc(desc)
{
	DISABLE_DESTRUCTION_SCOPE_GUARD(this);

	if (m_originalDesc.bindFlags.HasAllFlags(BindFlags::UniformBuffer))
		m_originalDesc.size = Align(m_originalDesc.size, Constants::UNIFORM_BUFFER_ALIGNMENT);

	if (desc.usage == ResourceUsage::Default)
	{
		auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto bufferDesc = CD3DX12_RESOURCE_DESC1::Buffer(
			m_originalDesc.size,
			GetD3D12ResourceFlags(m_originalDesc.bindFlags));
		PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateCommittedResource3(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_BARRIER_LAYOUT_UNDEFINED,
			nullptr, nullptr,
			0, nullptr,
			IID_PPV_ARGS(&m_resource)));

		PE_ASSERT_HR(m_resource->SetName(m_originalDesc.bufferName.c_str()));
	}
	else if (desc.usage == ResourceUsage::Staging)
	{
		PE_ASSERT(desc.cpuAccess != CPUAccess::None);

		auto heapType = desc.cpuAccess == CPUAccess::Write ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_READBACK;
		auto heapProps = CD3DX12_HEAP_PROPERTIES(heapType);
		auto bufferDesc = CD3DX12_RESOURCE_DESC1::Buffer(
			m_originalDesc.size,
			GetD3D12ResourceFlags(m_originalDesc.bindFlags));
		PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateCommittedResource3(
			&heapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_BARRIER_LAYOUT_UNDEFINED,
			nullptr, nullptr,
			0, nullptr,
			IID_PPV_ARGS(&m_resource)));

		PE_ASSERT_HR(m_resource->SetName(m_originalDesc.bufferName.c_str()));
	}
	else if (desc.usage == ResourceUsage::Dynamic)
	{
		// Dynamic buffers don't need to be created here, they will be automatically created when Map is called
		PE_ASSERT(desc.cpuAccess == CPUAccess::Write, "Dynamic buffers must have only the CPUAccess::Write flag set");
	}
}

D3D12Buffer::D3D12Buffer(ID3D12Resource* resource, const std::wstring& name, ResourceUsage usage, CPUAccess cpuAccess)
	: m_resource(resource), m_originalDesc({.bufferName = name, .usage = usage, .cpuAccess = cpuAccess})
{
	m_originalDesc = D3D12Buffer::GetBufferDesc();
}

void* D3D12Buffer::Map(Flags<CPUAccess> access)
{
	PE_ASSERT(
		m_originalDesc.usage == ResourceUsage::Dynamic ||
		m_originalDesc.usage == ResourceUsage::Staging,
		"Buffer must be Dynamic or Staging to be mapped!");

	if (m_originalDesc.usage == ResourceUsage::Dynamic)
		// TODO: Maybe add dynamic readback buffers?
		PE_ASSERT(access == CPUAccess::Write, "Dynamic buffers can only be written to");
	else if (m_originalDesc.usage == ResourceUsage::Staging)
		PE_ASSERT(m_originalDesc.cpuAccess == access, "Map access doesn't match the one buffer was created with");

	PE_ASSERT(!m_isMapped, "Can't map a buffer that is already mapped, unmap it first");

	m_isMapped = true;

	if (m_originalDesc.usage == ResourceUsage::Dynamic)
	{
		m_dynamicAllocation = D3D12RenderDevice::Get().AllocateDynamicBufferMemory(m_originalDesc.size);

		return m_dynamicAllocation.gpuRingAllocation.cpuAddress;
	}
	else if (m_originalDesc.usage == ResourceUsage::Staging)
	{
		void* address = nullptr;
		PE_ASSERT_HR(m_resource->Map(0, nullptr, &address));

		return address;
	}
	else
	{
		PE_ASSERT_NO_ENTRY();
		return nullptr;
	}
}

void D3D12Buffer::Unmap()
{
	m_isMapped = false;

	if (m_originalDesc.usage == ResourceUsage::Staging)
		m_resource->Unmap(0, nullptr);
}

BufferDesc D3D12Buffer::GetBufferDesc() const
{
	if (auto* d3d12Resource = GetD3D12Resource())
	{
		D3D12_RESOURCE_DESC d3ddesc = d3d12Resource->GetDesc();

		BufferDesc desc = {
			.bufferName = m_originalDesc.bufferName,
			.size = (int32_t)d3ddesc.Width,
			.bindFlags = GetBindFlags(d3ddesc.Flags),
			.usage = m_originalDesc.usage,
			.cpuAccess = m_originalDesc.cpuAccess
		};
		if (m_originalDesc.usage == ResourceUsage::Dynamic)
			desc.size = m_originalDesc.size;

		return desc;
	}

	return m_originalDesc;
}

ID3D12Resource* D3D12Buffer::GetD3D12Resource() const
{
	if (m_originalDesc.usage == ResourceUsage::Dynamic)
		return D3D12RenderDevice::Get().GetD3D12ResourceForDynamicAllocation(m_dynamicAllocation.ringBufferID);
	else
		return m_resource.Get();
}

int64_t D3D12Buffer::GetDefaultOffset() const
{
	if (m_originalDesc.usage == ResourceUsage::Dynamic)
		return m_dynamicAllocation.gpuRingAllocation.offset;
	else
		return 0;
}
}
