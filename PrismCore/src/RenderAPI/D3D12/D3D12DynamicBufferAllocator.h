#pragma once
#include "RenderAPI/D3D12/D3D12DynamicGPURingBuffer.h"

namespace Prism::Render::D3D12
{
struct DynamicBufferAllocator
{
public:
	DynamicBufferAllocator() = default;

	// Copies nor moves allowed
	DynamicBufferAllocator(const DynamicBufferAllocator&) = delete;
	DynamicBufferAllocator& operator=(const DynamicBufferAllocator&) = delete;
	DynamicBufferAllocator(DynamicBufferAllocator&&) = delete;
	DynamicBufferAllocator& operator=(DynamicBufferAllocator&&) = delete;

	~DynamicBufferAllocator() = default;

	void Init(int64_t initSize);

	DynamicGPURingBuffer::DynamicAllocation Allocate(int64_t size);

	void CloseCmdListAllocations(uint64_t fenceValue);
	void ReleaseStaleAllocations(uint64_t lastCompletedFenceValue);

private:
	std::vector<DynamicGPURingBuffer> m_ringBuffers;
};
}
