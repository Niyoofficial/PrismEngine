#pragma once

#include "RenderAPI/D3D12/D3D12Base.h"

namespace Prism::D3D12
{
class DescriptorHeapAllocation
{
public:
	DescriptorHeapAllocation(class DescriptorHeap* heap,
							 D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
							 D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle,
							 int32_t handlesCount = 1);
	DescriptorHeapAllocation(DescriptorHeap* heap,
							 D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
							 int32_t handlesCount = 1);

	~DescriptorHeapAllocation() = default;

	DescriptorHeapAllocation(DescriptorHeapAllocation&& other) = default;
	DescriptorHeapAllocation& operator=(DescriptorHeapAllocation&& other) = default;

	// No copies allowed
	DescriptorHeapAllocation(const DescriptorHeapAllocation& other) = delete;
	DescriptorHeapAllocation operator=(const DescriptorHeapAllocation& other) = delete;

	DescriptorHeap* GetOwningHeap() const;
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(int32_t index = 0) const;
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(int32_t index = 0) const;

private:
	DescriptorHeap* m_heap = nullptr;
	int32_t m_handlesCount = -1;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_firstCPUHandle = {};
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_firstGPUHandle = {};
};


// Wrapper for ID3D12DescriptorHeap
class DescriptorHeap
{
	struct FreeBlockInfo;

	using OffsetsMapType = std::unordered_map<int32_t, FreeBlockInfo>;
	using SizesMapType = std::unordered_multimap<int32_t, OffsetsMapType::iterator>;

	struct FreeBlockInfo
	{
		SizesMapType::iterator sizeMapIt;
	};

public:
	DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, int32_t descriptorsCount, bool shaderVisible);
	~DescriptorHeap() = default;

	DescriptorHeap(DescriptorHeap&& other) = default;
	DescriptorHeap& operator=(DescriptorHeap&& other) = default;

	// No copies allowed
	DescriptorHeap(const DescriptorHeap& other) = delete;
	DescriptorHeap operator=(const DescriptorHeap& other) = delete;

	DescriptorHeapAllocation Allocate(int32_t count);

	D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() const;

private:
	DescriptorHeapAllocation AllocateFromFreeBlock(const SizesMapType::iterator& freeBlock, int32_t count);

private:
	ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;

	OffsetsMapType m_freeBlocksByOffset;
	SizesMapType m_freeBlocksBySize;
};


class CPUDescriptorHeapManager
{
public:
	explicit CPUDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type);

	DescriptorHeapAllocation Allocate(int32_t count);

private:
	D3D12_DESCRIPTOR_HEAP_TYPE m_heapType;

	std::vector<DescriptorHeap> m_heaps;
};


class GPUDescriptorHeapManager
{
public:
	explicit GPUDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type);

	DescriptorHeapAllocation Allocate(int32_t count);

private:
	D3D12_DESCRIPTOR_HEAP_TYPE m_heapType;

	std::vector<DescriptorHeap> m_heaps;
};
}
