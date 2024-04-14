#include "pcpch.h"
#include "RenderAPI/D3D12/D3D12RenderContext.h"
#include "D3D12DescriptorHeapManager.h"
#include "Prism-Core/Render/RenderConstants.h"
#include "RenderAPI/D3D12/D3D12RenderDevice.h"


namespace Prism::Render::D3D12
{
//
// DescriptorHeapAllocation
//

template<DescriptorType Type>
DescriptorHeapAllocation<Type>::DescriptorHeapAllocation(DescriptorHeap<Type>* heap,
												   D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
												   D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle,
												   int32_t handlesCount) requires HAS_GPU_HANDLE
	: m_heap(heap), m_numHandles(handlesCount), m_firstCPUHandle(cpuHandle), m_firstGPUHandle(gpuHandle)
{
}

template<DescriptorType Type>
DescriptorHeapAllocation<Type>::DescriptorHeapAllocation(DescriptorHeap<Type>* heap,
												   D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
												   int32_t handlesCount)
	: m_heap(heap), m_numHandles(handlesCount), m_firstCPUHandle(cpuHandle)
{
}

template<DescriptorType Type>
DescriptorHeap<Type>* DescriptorHeapAllocation<Type>::GetOwningHeap() const
{
	PE_ASSERT(m_heap);
	return m_heap;
}

template<DescriptorType Type>
CD3DX12_CPU_DESCRIPTOR_HANDLE DescriptorHeapAllocation<Type>::GetCPUHandle(int32_t index) const
{
	PE_ASSERT(m_heap && m_numHandles > 0);
	PE_ASSERT(index >= 0 && index < m_numHandles, "Index out of allocation bounds");

	return {
		m_firstCPUHandle, index,
		D3D12RenderDevice::Get().GetDescriptorHandleSize(m_heap->GetHeapType())
	};
}

template<DescriptorType Type>
CD3DX12_GPU_DESCRIPTOR_HANDLE DescriptorHeapAllocation<Type>::GetGPUHandle(int32_t index) const requires HAS_GPU_HANDLE
{
	PE_ASSERT(m_heap && m_numHandles > 0);
	PE_ASSERT(m_firstGPUHandle.ptr != 0, "Allocation doesn't hold a GPU handle!");
	PE_ASSERT(index >= 0 && index < m_numHandles, "Index out of allocation bounds");

	return {
		m_firstGPUHandle, index,
		D3D12RenderDevice::Get().GetDescriptorHandleSize(m_heap->GetHeapType())
	};
}


//
// DescriptorHeap
//

template<DescriptorType Type>
DescriptorHeap<Type>::DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, int32_t descriptorsCount)
{
	PE_ASSERT(descriptorsCount > 0);

	D3D12_DESCRIPTOR_HEAP_DESC desc = {
		.Type = type,
		.NumDescriptors = (UINT)descriptorsCount,
		.Flags = IS_GPU_HEAP ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		.NodeMask = 0
	};
	PE_ASSERT_HR(D3D12RenderDevice::Get().GetD3D12Device()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_descriptorHeap)));

	AddNewBlock(0, descriptorsCount);
}

template<DescriptorType Type>
DescriptorHeapAllocation<Type> DescriptorHeap<Type>::Allocate(int32_t count)
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

template<DescriptorType Type>
D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeap<Type>::GetHeapType() const
{
	return m_descriptorHeap->GetDesc().Type;
}

template<DescriptorType Type>
DescriptorHeapAllocation<Type> DescriptorHeap<Type>::AllocateFromFreeBlock(const SizesMapTypeIt& freeBlock, int32_t count)
{
	PE_ASSERT(freeBlock != m_freeBlocksBySize.end());

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

	if constexpr (IS_GPU_HEAP)
	{
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

	return {this, cpuHandle, count};
}

template<DescriptorType Type>
void DescriptorHeap<Type>::AddNewBlock(int32_t offset, int32_t size)
{
	auto [newBlockOffsetIt, success] = m_freeBlocksByOffset.emplace(offset, FreeBlockInfo());
	auto newBlockSizeIt = m_freeBlocksBySize.emplace(size, newBlockOffsetIt);
	newBlockOffsetIt->second.sizeMapIt = newBlockSizeIt;
}


//
// CPUDescriptorHeapManager
//

CPUDescriptorHeapManager::CPUDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type)
	: m_heapType(type)
{
	// Create 1 heap
	m_heaps.emplace_back(m_heapType, Constants::DESCRIPTOR_COUNT_PER_CPU_HEAP);
}

CPUDescriptorHeapAllocation CPUDescriptorHeapManager::Allocate(int32_t count)
{
	// TODO
	return m_heaps[0].Allocate(count);
}


//
// GPUDescriptorHeapManager
//

GPUDescriptorHeapManager::GPUDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type)
	: m_heapType(type), m_heap(type, Constants::DESCRIPTOR_COUNT_PER_CPU_HEAP)
{
	PE_ASSERT(m_heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || m_heapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
}

GPUDescriptorHeapAllocation GPUDescriptorHeapManager::Allocate(int32_t count)
{
	return m_heap.Allocate(count);
}
}
