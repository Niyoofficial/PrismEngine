#pragma once
#include "RenderAPI/D3D12/D3D12DynamicGPURingBuffer.h"

namespace Prism::Render::D3D12
{
struct DynamicBufferAllocator
{
public:
	struct Allocation
	{
		bool IsValid() const;

		bool operator==(const Allocation&) const = default;

		int64_t ringBufferID = -1;
		DynamicGPURingBuffer::Allocation gpuRingAllocation = {};
	};
public:
	DynamicBufferAllocator() = default;

	// Copies nor moves allowed
	DynamicBufferAllocator(const DynamicBufferAllocator&) = delete;
	DynamicBufferAllocator& operator=(const DynamicBufferAllocator&) = delete;
	DynamicBufferAllocator(DynamicBufferAllocator&&) = delete;
	DynamicBufferAllocator& operator=(DynamicBufferAllocator&&) = delete;

	~DynamicBufferAllocator() = default;

	void Init(int64_t initSize);

	Allocation Allocate(int64_t size);

	void CloseCmdListAllocations(uint64_t fenceValue);
	void ReleaseStaleAllocations(uint64_t lastCompletedFenceValue);

	ID3D12Resource* GetD3D12Resource(int64_t ringBufferID) const;

private:
	int64_t m_nextRingBufferID = 0;

	struct TrackedRingBuffer
	{
		int64_t ID = -1;
		DynamicGPURingBuffer ringBuffer;
	};
	std::vector<TrackedRingBuffer> m_ringBuffers;
};
}
