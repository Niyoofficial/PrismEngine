#pragma once

#include "RenderAPI/D3D12/D3D12Base.h"
#include <map>

namespace Prism::Render::D3D12
{
class DescriptorHeap;

enum class HeapDeviceType
{
	GPU,
	CPU
};

class DescriptorHeapAllocation
{
public:
	DescriptorHeapAllocation() = default;
	DescriptorHeapAllocation(DescriptorHeap* heap,
							 D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
							 D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle,
							 int32_t handlesCount = 1);

	~DescriptorHeapAllocation();

	DescriptorHeapAllocation(DescriptorHeapAllocation&& other) noexcept;
	DescriptorHeapAllocation& operator=(DescriptorHeapAllocation&& other) noexcept;

	// No copies allowed
	DescriptorHeapAllocation(const DescriptorHeapAllocation& other) = delete;
	DescriptorHeapAllocation operator=(const DescriptorHeapAllocation& other) = delete;

	void Reset();

	DescriptorHeap* GetOwningHeap() const;
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(int32_t index = 0) const;
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(int32_t index = 0) const;
	int32_t GetHandleIndexInHeap(int32_t index = 0) const;

	int32_t GetNumHandles() const { return m_numHandles; }

	bool IsNull() const;

private:
	DescriptorHeap* m_heap = nullptr;
	int32_t m_numHandles = -1;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_firstCPUHandle = {};
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_firstGPUHandle = {};
};

// Wrapper for ID3D12DescriptorHeap
class DescriptorHeap
{
	struct FreeBlockInfo;

	using OffsetsMapType = std::map<int32_t, FreeBlockInfo>;
	using OffsetsMapTypeIt = typename OffsetsMapType::iterator;

	using SizesMapType = std::multimap<int32_t, OffsetsMapTypeIt>;
	using SizesMapTypeIt = typename SizesMapType::iterator;

	struct FreeBlockInfo
	{
		int32_t GetBlockSize()
		{
			return sizeMapIt->first;
		}

		SizesMapTypeIt sizeMapIt;
	};

public:
	DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, int32_t descriptorsCount, bool shouldBeGPUVisible);
	~DescriptorHeap() = default;

	DescriptorHeap(DescriptorHeap&& other) = default;
	DescriptorHeap& operator=(DescriptorHeap&& other) = default;

	// No copies allowed
	DescriptorHeap(const DescriptorHeap& other) = delete;
	DescriptorHeap operator=(const DescriptorHeap& other) = delete;

	DescriptorHeapAllocation Allocate(int32_t count);
	void Free(DescriptorHeapAllocation&& allocation);

	D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() const;
	ID3D12DescriptorHeap* GetD3D12DescriptorHeap() const { return m_descriptorHeap.Get(); }

private:
	DescriptorHeapAllocation AllocateFromFreeBlock(const SizesMapTypeIt& freeBlock, int32_t count);
	void AddNewBlock(int32_t offset, int32_t size);

private:
	ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;

	OffsetsMapType m_freeBlocksByOffset;
	SizesMapType m_freeBlocksBySize;
};

class CPUDescriptorHeapManager
{
public:
	explicit CPUDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type, int32_t descriptorsPerHeap);

	DescriptorHeapAllocation Allocate(int32_t count = 1);

	const std::vector<DescriptorHeap>& GetDescriptorHeaps() const { return m_heaps; }

private:
	D3D12_DESCRIPTOR_HEAP_TYPE m_heapType;

	std::vector<DescriptorHeap> m_heaps;
};


class GPUDescriptorHeapManager
{
public:
	explicit GPUDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type, int32_t descriptorsPerHeap);

	DescriptorHeapAllocation Allocate(int32_t count = 1);

	const DescriptorHeap& GetDescriptorHeap() const { return m_heap; }

private:
	D3D12_DESCRIPTOR_HEAP_TYPE m_heapType;

	DescriptorHeap m_heap;
};
}
