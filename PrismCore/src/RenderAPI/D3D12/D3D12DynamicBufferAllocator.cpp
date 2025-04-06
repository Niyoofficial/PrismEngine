#include "pcpch.h"
#include "D3D12DynamicBufferAllocator.h"

namespace Prism::Render::D3D12
{
void DynamicBufferAllocator::Init(int64_t initSize)
{
	m_ringBuffers.emplace_back(initSize);
}

DynamicGPURingBuffer::DynamicAllocation DynamicBufferAllocator::Allocate(int64_t size)
{
	auto allocation = m_ringBuffers.back().Allocate(size);
	if (!allocation.IsValid())
	{
		int64_t newMaxSize = m_ringBuffers.back().GetMaxSize() * 2;
		while (newMaxSize < size)
			newMaxSize *= 2;

		m_ringBuffers.emplace_back(newMaxSize);
		allocation = m_ringBuffers.back().Allocate(size);
	}

	return allocation;
}

void DynamicBufferAllocator::CloseCmdListAllocations(uint64_t fenceValue)
{
    for (auto& ringBuffer : m_ringBuffers)
		ringBuffer.CloseCmdListAllocations(fenceValue);
}

void DynamicBufferAllocator::ReleaseStaleAllocations(uint64_t lastCompletedFenceValue)
{
    size_t numBuffsToDelete = 0;
    for (size_t i = 0; i < m_ringBuffers.size(); ++i)
    {
        auto& ringBuffer = m_ringBuffers[i];
        ringBuffer.ReleaseCompletedResources(lastCompletedFenceValue);
        if (numBuffsToDelete == i && i < m_ringBuffers.size() - 1 && ringBuffer.IsEmpty())
            ++numBuffsToDelete;
    }

    if (numBuffsToDelete)
        m_ringBuffers.erase(m_ringBuffers.begin(), m_ringBuffers.begin() + (int32_t)numBuffsToDelete);
}
}
