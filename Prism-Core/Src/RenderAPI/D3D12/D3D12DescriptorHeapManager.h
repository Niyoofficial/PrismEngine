#pragma once

#include "RenderAPI/D3D12/D3D12Base.h"
#include <map>

namespace Prism::Render::D3D12
{
enum class DescriptorType
{
	CPU,
	GPU
};

template<DescriptorType Type>
class DescriptorHeap;

template<DescriptorType Type>
class DescriptorHeapAllocation
{
	static constexpr bool HAS_GPU_HANDLE = Type == DescriptorType::GPU;

	using DescriptorHandleType = std::conditional<HAS_GPU_HANDLE, CD3DX12_GPU_DESCRIPTOR_HANDLE, CD3DX12_CPU_DESCRIPTOR_HANDLE>;

public:
	DescriptorHeapAllocation() = default;
	DescriptorHeapAllocation(DescriptorHeap<Type>* heap,
							 D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
							 D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle,
							 int32_t handlesCount = 1) requires HAS_GPU_HANDLE;
	DescriptorHeapAllocation(DescriptorHeap<Type>* heap,
							 D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
							 int32_t handlesCount = 1);

	~DescriptorHeapAllocation();

	DescriptorHeapAllocation(DescriptorHeapAllocation&& other) noexcept;
	DescriptorHeapAllocation& operator=(DescriptorHeapAllocation&& other) noexcept;

	// No copies allowed
	DescriptorHeapAllocation(const DescriptorHeapAllocation& other) = delete;
	DescriptorHeapAllocation operator=(const DescriptorHeapAllocation& other) = delete;

	void Reset();

	DescriptorHeap<Type>* GetOwningHeap() const;
	CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(int32_t index = 0) const;
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(int32_t index = 0) const requires HAS_GPU_HANDLE;

	int32_t GetNumHandles() const { return m_numHandles; }

	bool IsNull() const;

private:
	DescriptorHeap<Type>* m_heap = nullptr;
	int32_t m_numHandles = -1;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_firstCPUHandle = {};

	struct Empty {};
	using GPUDescriptorHandle = std::conditional_t<HAS_GPU_HANDLE, CD3DX12_GPU_DESCRIPTOR_HANDLE, Empty>;
	[[no_unique_address]] GPUDescriptorHandle m_firstGPUHandle = {};
};

template DescriptorHeapAllocation<DescriptorType::CPU>;
template DescriptorHeapAllocation<DescriptorType::GPU>;

using CPUDescriptorHeapAllocation = DescriptorHeapAllocation<DescriptorType::CPU>;
using GPUDescriptorHeapAllocation = DescriptorHeapAllocation<DescriptorType::GPU>;

// Wrapper for ID3D12DescriptorHeap
template<DescriptorType Type>
class DescriptorHeap
{
	static constexpr bool IS_GPU_HEAP = Type == DescriptorType::GPU;

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
	DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, int32_t descriptorsCount);
	~DescriptorHeap() = default;

	DescriptorHeap(DescriptorHeap&& other) = default;
	DescriptorHeap& operator=(DescriptorHeap&& other) = default;

	// No copies allowed
	DescriptorHeap(const DescriptorHeap& other) = delete;
	DescriptorHeap operator=(const DescriptorHeap& other) = delete;

	DescriptorHeapAllocation<Type> Allocate(int32_t count);
	void Free(DescriptorHeapAllocation<Type>&& allocation);

	D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() const;
	ID3D12DescriptorHeap* GetD3D12DescriptorHeap() const { return m_descriptorHeap.Get(); }

private:
	DescriptorHeapAllocation<Type> AllocateFromFreeBlock(const SizesMapTypeIt& freeBlock, int32_t count);
	void AddNewBlock(int32_t offset, int32_t size);

private:
	ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;

	OffsetsMapType m_freeBlocksByOffset;
	SizesMapType m_freeBlocksBySize;
};

template DescriptorHeap<DescriptorType::CPU>;
template DescriptorHeap<DescriptorType::GPU>;

using CPUDescriptorHeap = DescriptorHeap<DescriptorType::CPU>;
using GPUDescriptorHeap = DescriptorHeap<DescriptorType::GPU>;


class CPUDescriptorHeapManager
{
public:
	explicit CPUDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type);

	CPUDescriptorHeapAllocation Allocate(int32_t count = 1);

	const std::vector<CPUDescriptorHeap>& GetDescriptorHeaps() const { return m_heaps; }

private:
	D3D12_DESCRIPTOR_HEAP_TYPE m_heapType;

	std::vector<CPUDescriptorHeap> m_heaps;
};


class GPUDescriptorHeapManager
{
public:
	explicit GPUDescriptorHeapManager(D3D12_DESCRIPTOR_HEAP_TYPE type);

	GPUDescriptorHeapAllocation Allocate(int32_t count = 1);

	const GPUDescriptorHeap& GetDescriptorHeap() const { return m_heap; }

private:
	D3D12_DESCRIPTOR_HEAP_TYPE m_heapType;

	GPUDescriptorHeap m_heap;
};
}
