#include "pcpch.h"
#include "D3D12DescriptorHeapManager.h"

#include "Prism-Core/Render/RenderConstants.h"
#include "RenderAPI/D3D12/D3D12RenderAPI.h"

namespace Prism::D3D12
{
//
// DescriptorHeapAllocation
//

DescriptorHeapAllocation::DescriptorHeapAllocation(DescriptorHeap* heap,
												   D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
												   D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle,
												   int32_t handlesCount)
	: m_heap(heap), m_handlesCount(handlesCount), m_firstCPUHandle(cpuHandle), m_firstGPUHandle(gpuHandle)
{
}

DescriptorHeapAllocation::DescriptorHeapAllocation(DescriptorHeap* heap,
												   D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
												   int32_t handlesCount)
	: DescriptorHeapAllocation(heap, cpuHandle, {}, handlesCount)
{
}

DescriptorHeap* DescriptorHeapAllocation::GetOwningHeap() const
{
	PE_ASSERT(m_heap);
	return m_heap;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE DescriptorHeapAllocation::GetCPUHandle(int32_t index) const
{
	PE_ASSERT(m_heap && m_handlesCount > 0);
	PE_ASSERT(index >= 0 && index < m_handlesCount, "Index out of allocation bounds");

	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_firstCPUHandle, index,
		D3D12RenderAPI::Get()->GetDescriptorHandleSize(m_heap->GetHeapType()));
}

CD3DX12_GPU_DESCRIPTOR_HANDLE DescriptorHeapAllocation::GetGPUHandle(int32_t index) const
{
	PE_ASSERT(m_heap && m_handlesCount > 0);
	PE_ASSERT(m_firstGPUHandle.ptr != 0, "Allocation doesn't hold a GPU handle!");
	PE_ASSERT(index >= 0 && index < m_handlesCount, "Index out of allocation bounds");

	return CD3DX12_GPU_DESCRIPTOR_HANDLE(
		m_firstGPUHandle, index,
		D3D12RenderAPI::Get()->GetDescriptorHandleSize(m_heap->GetHeapType()));
}


//
// DescriptorHeap
//

DescriptorHeap::DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, int32_t descriptorsCount, bool shaderVisible)
{
	PE_ASSERT(descriptorsCount > 0);

	D3D12_DESCRIPTOR_HEAP_DESC desc = {
		.Type = type,
		.NumDescriptors = (UINT)descriptorsCount,
		.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		.NodeMask = 0
	};
	PE_ASSERT_HR(D3D12RenderAPI::Get()->GetD3DDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_descriptorHeap)));
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

D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeap::GetHeapType() const
{
	return m_descriptorHeap->GetDesc().Type;
}

DescriptorHeapAllocation DescriptorHeap::AllocateFromFreeBlock(const SizesMapType::iterator& freeBlock, int32_t count)
{
	PE_ASSERT(freeBlock != m_freeBlocksBySize.end());

	auto offsetIt = freeBlock->second;
	int32_t allocationOffset = offsetIt->first;

	int32_t freeBlockNewOffset = allocationOffset + count;
	int32_t freeBlockNewSize = freeBlock->first - count;

	m_freeBlocksBySize.erase(freeBlock);
	m_freeBlocksByOffset.erase(offsetIt);
	if (freeBlockNewSize > 0)
	{
		auto [newBlockOffsetIt, success] = m_freeBlocksByOffset.emplace(freeBlockNewOffset, FreeBlockInfo());
		auto newBlockSizeIt = m_freeBlocksBySize.emplace(freeBlockNewSize, newBlockOffsetIt);
		newBlockOffsetIt->second.sizeMapIt = newBlockSizeIt;
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE cpuHandle(
		m_descriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		allocationOffset,
		D3D12RenderAPI::Get()->GetDescriptorHandleSize(GetHeapType()));

	CD3DX12_GPU_DESCRIPTOR_HANDLE gpuHandle = {};
	if (m_descriptorHeap->GetDesc().Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
	{
		gpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
			m_descriptorHeap->GetGPUDescriptorHandleForHeapStart(),
			allocationOffset,
			D3D12RenderAPI::Get()->GetDescriptorHandleSize(m_descriptorHeap->GetDesc().Type)
		);
	}
	return {this, cpuHandle, gpuHandle, count};
}


//
// CPUDescriptorHeapManager
//

CPUDescriptorHeapManager::CPUDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type)
	: m_heapType(type)
{
	// Create 1 heap
	m_heaps.emplace_back(m_heapType, Render::Constants::DESCRIPTOR_COUNT_PER_CPU_HEAP, false);
}

DescriptorHeapAllocation CPUDescriptorHeapManager::Allocate(int32_t count)
{
	// TODO
	return m_heaps[0].Allocate(count);
}

GPUDescriptorHeapManager::GPUDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type)
	: m_heapType(type)
{
	PE_ASSERT(type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

	m_heaps.emplace_back(m_heapType, Render::Constants::DESCRIPTOR_COUNT_PER_CPU_HEAP, true);
}

DescriptorHeapAllocation GPUDescriptorHeapManager::Allocate(int32_t count)
{
	// TODO
	return {nullptr, {}, {}};
}
}
