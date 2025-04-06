#include "pcpch.h"
#include "D3D12DescriptorHeapManager.h"
#include "RenderAPI/D3D12/D3D12RenderDevice.h"


namespace Prism::Render::D3D12
{
//
// DescriptorHeapAllocation
//

DescriptorHeapAllocation::DescriptorHeapAllocation(DescriptorHeap* heap,
												   D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
												   D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle,
												   int32_t handlesCount)
	: m_heap(heap), m_numHandles(handlesCount), m_firstCPUHandle(cpuHandle), m_firstGPUHandle(gpuHandle)
{
}

DescriptorHeapAllocation::~DescriptorHeapAllocation()
{
	if (!IsNull())
		m_heap->Free(std::move(*this));

	PE_ASSERT(IsNull());
}


DescriptorHeapAllocation::DescriptorHeapAllocation(DescriptorHeapAllocation&& other) noexcept
	: m_heap(std::move(other.m_heap)),
	  m_numHandles(std::move(other.m_numHandles)),
	  m_firstCPUHandle(std::move(other.m_firstCPUHandle)),
	  m_firstGPUHandle(std::move(other.m_firstGPUHandle))
{
	other.Reset();
}

DescriptorHeapAllocation& DescriptorHeapAllocation::operator=(DescriptorHeapAllocation&& other) noexcept
{
	m_heap = std::move(other.m_heap);
	m_numHandles = std::move(other.m_numHandles);
	m_firstCPUHandle = std::move(other.m_firstCPUHandle);
	m_firstGPUHandle = std::move(other.m_firstGPUHandle);

	other.Reset();

	return *this;
}

void DescriptorHeapAllocation::Reset()
{
	m_heap = nullptr;
	m_numHandles = -1;
	m_firstCPUHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE{};
	m_firstGPUHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE{};
}

DescriptorHeap* DescriptorHeapAllocation::GetOwningHeap() const
{
	PE_ASSERT(m_heap);
	return m_heap;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE DescriptorHeapAllocation::GetCPUHandle(int32_t index) const
{
	PE_ASSERT(m_heap && m_numHandles > 0);
	PE_ASSERT(index >= 0 && index < m_numHandles, "Index out of allocation bounds");

	return {
		m_firstCPUHandle, index,
		D3D12RenderDevice::Get().GetDescriptorHandleSize(m_heap->GetHeapType())
	};
}

CD3DX12_GPU_DESCRIPTOR_HANDLE DescriptorHeapAllocation::GetGPUHandle(int32_t index) const
{
	PE_ASSERT(m_heap && m_numHandles > 0);
	PE_ASSERT(m_firstGPUHandle.ptr != 0, "Allocation doesn't hold a GPU handle!");
	PE_ASSERT(index >= 0 && index < m_numHandles, "Index out of allocation bounds");

	return {
		m_firstGPUHandle, index,
		D3D12RenderDevice::Get().GetDescriptorHandleSize(m_heap->GetHeapType())
	};
}

int32_t DescriptorHeapAllocation::GetHandleIndexInHeap(int32_t index) const
{
	PE_ASSERT(m_heap && m_numHandles > 0);
	PE_ASSERT(index >= 0 && index < m_numHandles, "Index out of allocation bounds");

	auto firstHandleInHeap = m_heap->GetD3D12DescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	uint32_t handleSize = D3D12RenderDevice::Get().GetDescriptorHandleSize(m_heap->GetHeapType());
	return (int32_t)((m_firstCPUHandle.ptr - firstHandleInHeap.ptr) / (SIZE_T)handleSize);
}

bool DescriptorHeapAllocation::IsNull() const
{
	bool heapIsNull = m_heap == nullptr;
	PE_ASSERT(!heapIsNull || m_firstCPUHandle.ptr == 0);

	return heapIsNull;
}


//
// DescriptorHeap
//

DescriptorHeap::DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, int32_t descriptorsCount, bool shouldBeGPUVisible)
{
	PE_ASSERT(descriptorsCount > 0);

	D3D12_DESCRIPTOR_HEAP_DESC desc = {
		.Type = type,
		.NumDescriptors = (UINT)descriptorsCount,
		.Flags = shouldBeGPUVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		.NodeMask = 0
	};
	PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_descriptorHeap)));

	AddNewBlock(0, descriptorsCount);
}

DescriptorHeapAllocation DescriptorHeap::Allocate(int32_t count)
{
	auto sizeIt = std::ranges::min_element(m_freeBlocksBySize,
										   [count](auto e1, auto e2) -> bool
										   {
											   if (e1.first <= count) return false;
											   if (e2.first <= count) return true;
											   return (e1.first < e2.first);
										   });

	return AllocateFromFreeBlock(sizeIt, count);
}

void DescriptorHeap::Free(DescriptorHeapAllocation&& allocation)
{
	if (!D3D12RenderDevice::TryGet())
		return;

	uint32_t handleSize = D3D12RenderDevice::Get().GetDescriptorHandleSize(GetHeapType());
	int32_t offset = (int32_t)((allocation.GetCPUHandle().ptr - m_descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr) / handleSize);
	int32_t size = allocation.GetNumHandles();

	auto nextBlockIt = m_freeBlocksByOffset.upper_bound(offset);
	PE_ASSERT(nextBlockIt == m_freeBlocksByOffset.end() || offset + size <= nextBlockIt->first);

	auto prevBlockIt = nextBlockIt;
	if (prevBlockIt != m_freeBlocksByOffset.begin())
	{
		--prevBlockIt;
		PE_ASSERT(offset >= prevBlockIt->first + prevBlockIt->second.GetBlockSize());
	}
	else
	{
		prevBlockIt = m_freeBlocksByOffset.end();
	}

	int32_t newOffset = -1;
	int32_t newSize = -1;
	if (prevBlockIt != m_freeBlocksByOffset.end() && offset == prevBlockIt->first + prevBlockIt->second.GetBlockSize())
	{
		//  PrevBlock.Offset             Offset
		//       |                          |
		//       |<-----PrevBlock.Size----->|<------Size-------->|
		//
		newOffset = prevBlockIt->first;
		newSize = prevBlockIt->second.GetBlockSize() + size;

		if (nextBlockIt != m_freeBlocksByOffset.end() && offset + size == nextBlockIt->first)
		{
			//   PrevBlock.Offset           Offset            NextBlock.Offset
			//     |                          |                    |
			//     |<-----PrevBlock.Size----->|<------Size-------->|<-----NextBlock.Size----->|
			//
			newSize += nextBlockIt->second.GetBlockSize();
			m_freeBlocksBySize.erase(prevBlockIt->second.sizeMapIt);
			m_freeBlocksBySize.erase(nextBlockIt->second.sizeMapIt);
			// Delete the range of two blocks
			++nextBlockIt;
			m_freeBlocksByOffset.erase(prevBlockIt, nextBlockIt);
		}
		else
		{
			//   PrevBlock.Offset           Offset                     NextBlock.Offset
			//     |                          |                             |
			//     |<-----PrevBlock.Size----->|<------Size-------->| ~ ~ ~  |<-----NextBlock.Size----->|
			//
			m_freeBlocksBySize.erase(prevBlockIt->second.sizeMapIt);
			m_freeBlocksByOffset.erase(prevBlockIt);
		}
	}
	else if (nextBlockIt != m_freeBlocksByOffset.end() && offset + size == nextBlockIt->first)
	{
		//   PrevBlock.Offset                   Offset            NextBlock.Offset
		//     |                                  |                    |
		//     |<-----PrevBlock.Size----->| ~ ~ ~ |<------Size-------->|<-----NextBlock.Size----->|
		//
		newSize = size + nextBlockIt->second.GetBlockSize();
		newOffset = offset;
		m_freeBlocksBySize.erase(nextBlockIt->second.sizeMapIt);
		m_freeBlocksByOffset.erase(nextBlockIt);
	}
	else
	{
		//   PrevBlock.Offset                   Offset                     NextBlock.Offset
		//     |                                  |                            |
		//     |<-----PrevBlock.Size----->| ~ ~ ~ |<------Size-------->| ~ ~ ~ |<-----NextBlock.Size----->|
		//
		newSize = size;
		newOffset = offset;
	}

	AddNewBlock(newOffset, newSize);

	allocation.Reset();
}

D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeap::GetHeapType() const
{
	return m_descriptorHeap->GetDesc().Type;
}

DescriptorHeapAllocation DescriptorHeap::AllocateFromFreeBlock(const SizesMapTypeIt& freeBlock, int32_t count)
{
	PE_ASSERT(freeBlock != m_freeBlocksBySize.end(), "Failed to allocate descriptors!");

	auto offsetIt = freeBlock->second;
	int32_t allocationOffset = offsetIt->first;

	int32_t freeBlockNewOffset = allocationOffset + count;
	int32_t freeBlockNewSize = freeBlock->first - count;

	m_freeBlocksBySize.erase(freeBlock);
	m_freeBlocksByOffset.erase(offsetIt);
	if (freeBlockNewSize > 0)
		AddNewBlock(freeBlockNewOffset, freeBlockNewSize);

	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(
		m_descriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		allocationOffset,
		D3D12RenderDevice::Get().GetDescriptorHandleSize(GetHeapType()));

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle = {};
	if (m_descriptorHeap->GetDesc().Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
	{
		gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
			m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(),
			allocationOffset,
			D3D12RenderDevice::Get().GetDescriptorHandleSize(m_descriptorHeap->GetDesc().Type)
		);
	}
	return {this, cpuHandle, gpuHandle, count};
}

void DescriptorHeap::AddNewBlock(int32_t offset, int32_t size)
{
	auto [newBlockOffsetIt, success] = m_freeBlocksByOffset.emplace(offset, FreeBlockInfo());
	auto newBlockSizeIt = m_freeBlocksBySize.emplace(size, newBlockOffsetIt);
	newBlockOffsetIt->second.sizeMapIt = newBlockSizeIt;
}


//
// CPUDescriptorHeapManager
//

CPUDescriptorHeapManager::CPUDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type, int32_t descriptorsPerHeap)
	: m_heapType(type)
{
	// Create 1 heap
	m_heaps.emplace_back(m_heapType, descriptorsPerHeap, false);
}

DescriptorHeapAllocation CPUDescriptorHeapManager::Allocate(int32_t count)
{
	// TODO
	return m_heaps[0].Allocate(count);
}


//
// GPUDescriptorHeapManager
//

GPUDescriptorHeapManager::GPUDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type, int32_t descriptorsPerHeap)
	: m_heapType(type), m_heap(type, descriptorsPerHeap, true)
{
	PE_ASSERT(m_heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || m_heapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

DescriptorHeapAllocation GPUDescriptorHeapManager::Allocate(int32_t count)
{
	// TODO: Should be thread safe!
	return m_heap.Allocate(count);
}
}
