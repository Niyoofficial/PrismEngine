#pragma once
#include "RenderAPI/D3D12/D3D12Base.h"

namespace Prism::Render::D3D12
{
struct DynamicGPURingBuffer
{
public:
	static constexpr int64_t INVALID_OFFSET = -1;

	struct CompletedTailAttributes
	{
		uint64_t fenceValue = 0;
		int64_t offset = INVALID_OFFSET;
		int64_t size = INVALID_OFFSET;
	};

	struct Allocation
	{
		bool IsValid() const;

		bool operator==(const Allocation&) const = default;

		int64_t offset = INVALID_OFFSET;
		int64_t size = INVALID_OFFSET;
		void* cpuAddress = nullptr;
		D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = {};
	};

public:
	explicit DynamicGPURingBuffer(int64_t maxSize);

	DynamicGPURingBuffer(DynamicGPURingBuffer&& rhs) noexcept;
	DynamicGPURingBuffer& operator=(DynamicGPURingBuffer&& rhs) noexcept;

	// Copies are not allowed
	DynamicGPURingBuffer(const DynamicGPURingBuffer&) = delete;
	DynamicGPURingBuffer& operator=(const DynamicGPURingBuffer&) = delete;

	~DynamicGPURingBuffer();

	Allocation Allocate(int64_t size);

	void CloseCmdListAllocations(uint64_t fenceValue);
	void ReleaseCompletedResources(uint64_t completedFenceValue);

	int64_t GetMaxSize() const { return m_maxSize; }
	int64_t GetUsedSize() const { return m_usedSize; }
	bool IsFull() const { return m_usedSize == m_maxSize; }
	bool IsEmpty() const { return m_usedSize == 0; }
	ID3D12Resource* GetD3D12Resource() const { return m_resource.Get(); }

private:
	std::deque<CompletedTailAttributes> m_completedTails;
	int64_t m_head = 0;
	int64_t m_tail = 0;
	int64_t m_maxSize = 0;
	int64_t m_usedSize = 0;
	int64_t m_currentCmdListSize = 0;
	ComPtr<ID3D12Resource> m_resource;
	D3D12_GPU_VIRTUAL_ADDRESS m_gpuVirtualAddress = {};
	void* m_cpuVirtualAddress = nullptr;
};
}
