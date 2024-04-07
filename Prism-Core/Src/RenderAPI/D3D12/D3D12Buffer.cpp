#include "pcpch.h"
#include "D3D12Buffer.h"

#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "RenderAPI/D3D12/D3D12TypeConversions.h"


namespace Prism::Render::D3D12
{
D3D12Buffer::D3D12Buffer(const BufferDesc& desc, const std::vector<BufferInitData>& initData)
	: m_originalDesc(desc)
{
	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(m_originalDesc.size, GetD3D12ResourceFlags(m_originalDesc.bindFlags));
	PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr, IID_PPV_ARGS(&m_resource)));

	PE_ASSERT_HR(m_resource->SetName(desc.bufferName.c_str()));

	if (!initData.empty())
	{
		ID3D12Resource* uploadBuffer = nullptr;

		auto uploadHeapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateCommittedResource(
			&uploadHeapProps,
			D3D12_HEAP_FLAG_NONE,
			&bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(&uploadBuffer)));

		D3D12Buffer wrappedUploadBuffer(uploadBuffer, L"UploadBuffer", ResourceUsage::Dynamic);

		void* mappedData = nullptr;
		PE_ASSERT_HR(uploadBuffer->Map(0, nullptr, &mappedData));

		for (const BufferInitData& data : initData)
		{
			PE_ASSERT(desc.size >= data.sizeInBytes + data.byteOffset);

			std::byte* offsetMappedData = (std::byte*)mappedData + data.byteOffset;
			memcpy_s(offsetMappedData, uploadBuffer->GetDesc().Width - data.byteOffset, data.data, data.sizeInBytes);
		}

		uploadBuffer->Unmap(0, nullptr);

		std::unique_ptr<RenderContext> context;
		context.reset(RenderDevice::Get().AllocateContext());

		context->CopyBufferRegion(this, 0, &wrappedUploadBuffer, 0, (int32_t)uploadBuffer->GetDesc().Width);

		context->Transition({
			.resource = this,
			.oldState = ResourceStateFlags::CopyDest,
			.newState = ResourceStateFlags::Common
		});

		RenderDevice::Get().SubmitContext(context.get());

		RenderDevice::Get().FlushCommandQueue();
	}
}

D3D12Buffer::D3D12Buffer(ID3D12Resource* resource, const std::wstring& name, ResourceUsage usage)
	: m_originalDesc(D3D12::GetBufferDesc(resource->GetDesc(), name, usage)), m_resource(resource)
{
}

BufferDesc D3D12Buffer::GetBufferDesc() const
{
	return D3D12::GetBufferDesc(m_resource->GetDesc(), m_originalDesc.bufferName, m_originalDesc.usage);
}
}
