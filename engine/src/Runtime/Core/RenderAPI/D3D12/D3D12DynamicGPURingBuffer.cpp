#include "pcpch.h"
#include "D3D12DynamicGPURingBuffer.h"

#include "RenderAPI/D3D12/D3D12RenderDevice.h"
#include "directx/d3dx12_core.h"

namespace Prism::Render::D3D12
{
bool DynamicGPURingBuffer::Allocation::IsValid() const
{
	return
		offset != INVALID_OFFSET &&
		size != INVALID_OFFSET &&
		cpuAddress != nullptr &&
		gpuAddress != D3D12_GPU_VIRTUAL_ADDRESS{};
}

DynamicGPURingBuffer::DynamicGPURingBuffer(int64_t maxSize)
	: m_maxSize(maxSize)
{
	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(m_maxSize);
	PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateCommittedResource(
		&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&m_resource)));
	PE_ASSERT_HR(m_resource->SetName(L"DynamicGPURingBufferResource"));

	m_gpuVirtualAddress = m_resource->GetGPUVirtualAddress();
	PE_ASSERT_HR(m_resource->Map(0, nullptr, &m_cpuVirtualAddress));
}

DynamicGPURingBuffer::DynamicGPURingBuffer(DynamicGPURingBuffer&& rhs) noexcept
	: m_completedTails(std::move(rhs.m_completedTails)),
	  m_head(rhs.m_head),
	  m_tail(rhs.m_tail),
	  m_maxSize(rhs.m_maxSize),
	  m_usedSize(rhs.m_usedSize),
	  m_currentCmdListSize(rhs.m_currentCmdListSize),
	  m_resource(std::move(rhs.m_resource)),
	  m_gpuVirtualAddress(rhs.m_gpuVirtualAddress),
	  m_cpuVirtualAddress(rhs.m_cpuVirtualAddress)
{
	rhs.m_head = 0;
	rhs.m_tail = 0;
	rhs.m_maxSize = 0;
	rhs.m_usedSize = 0;
	rhs.m_currentCmdListSize = 0;
	rhs.m_gpuVirtualAddress = 0;
	rhs.m_cpuVirtualAddress = nullptr;
}

DynamicGPURingBuffer& DynamicGPURingBuffer::operator=(DynamicGPURingBuffer&& rhs) noexcept
{
	m_completedTails = std::move(rhs.m_completedTails);
	m_head = rhs.m_head;
	m_tail = rhs.m_tail;
	m_maxSize = rhs.m_maxSize;
	m_usedSize = rhs.m_usedSize;
	m_currentCmdListSize = rhs.m_currentCmdListSize;
	m_resource = std::move(rhs.m_resource);
	m_gpuVirtualAddress = rhs.m_gpuVirtualAddress;
	m_cpuVirtualAddress = rhs.m_cpuVirtualAddress;

	rhs.m_head = 0;
	rhs.m_tail = 0;
	rhs.m_maxSize = 0;
	rhs.m_usedSize = 0;
	rhs.m_currentCmdListSize = 0;
	rhs.m_gpuVirtualAddress = 0;
	rhs.m_cpuVirtualAddress = nullptr;

	return *this;
}

DynamicGPURingBuffer::~DynamicGPURingBuffer()
{
	PE_ASSERT(m_usedSize == 0);

	if (m_resource)
		m_resource->Unmap(0, nullptr);
}

DynamicGPURingBuffer::Allocation DynamicGPURingBuffer::Allocate(int64_t size)
{
	if (IsFull())
		return {};

	static constexpr int64_t ALIGNMENT = 256;
	static constexpr int64_t ALIGNMENT_MASK = ALIGNMENT - 1;

	size = (size + ALIGNMENT_MASK) & ~ALIGNMENT_MASK;

	if (m_maxSize - m_usedSize < size)
		return {};

	int64_t currentOffset = INVALID_OFFSET;
	if (m_tail >= m_head)
	{
		//                     Head             Tail     MaxSize
		//                     |                |        |
		//  [                  xxxxxxxxxxxxxxxxx         ]
		//                                         
		//
		if (m_tail + size <= m_maxSize)
		{
			int64_t offset = m_tail;
			m_tail += size;
			m_usedSize += size;
			m_currentCmdListSize += size;
			currentOffset = offset;
		}
		else if (size <= m_head)
		{
			int64_t addSize = (m_maxSize - m_tail) + size;
			m_usedSize += addSize;
			m_currentCmdListSize += addSize;
			m_tail = size;
			currentOffset = 0;
		}
		else
		{
			// The memory has to be continuous so if we can't fit it as a whole after tail or
			// in front of the head, then return invalid offset and let a new ring buffer be allocated
			currentOffset = INVALID_OFFSET;
		}
	}
	else if (m_tail + size <= m_head)
	{
		//
		//       Tail          Head             
		//       |             |             
		//  [xxxx              xxxxxxxxxxxxxxxxxxxxxxxxxx]
		//
		int64_t offset = m_tail;
		m_tail += size;
		m_usedSize += size;
		m_currentCmdListSize += size;
		currentOffset = offset;
	}

	return {
		.offset = currentOffset,
		.size = size,
		.cpuAddress = (uint8_t*)m_cpuVirtualAddress + currentOffset,
		.gpuAddress = m_gpuVirtualAddress + currentOffset
	};
}

void DynamicGPURingBuffer::CloseCmdListAllocations(uint64_t fenceValue)
{
	m_completedTails.emplace_back(fenceValue, m_tail, m_currentCmdListSize);
	m_currentCmdListSize = 0;
}

void DynamicGPURingBuffer::ReleaseCompletedResources(uint64_t completedFenceValue)
{
	while (!m_completedTails.empty() && m_completedTails.front().fenceValue <= completedFenceValue)
	{
		auto& oldestTail = m_completedTails.front();
		PE_ASSERT(oldestTail.size <= m_usedSize);
		m_usedSize -= oldestTail.size;
		m_head = oldestTail.offset;
		m_completedTails.pop_front();
	}
}
}
