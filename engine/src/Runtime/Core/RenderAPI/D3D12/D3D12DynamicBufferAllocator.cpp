#include "pcpch.h"
#include "D3D12DynamicBufferAllocator.h"

namespace Prism::Render::D3D12
{
bool DynamicBufferAllocator::Allocation::IsValid() const
{
	return ringBufferID != -1 && gpuRingAllocation.IsValid();
}

void DynamicBufferAllocator::Init(int64_t initSize)
{
	m_ringBuffers.emplace_back(m_nextRingBufferID++, DynamicGPURingBuffer(initSize));
}

DynamicBufferAllocator::Allocation DynamicBufferAllocator::Allocate(int64_t size)
{
	auto allocation = m_ringBuffers.back().ringBuffer.Allocate(size);
	if (!allocation.IsValid())
	{
		int64_t newMaxSize = m_ringBuffers.back().ringBuffer.GetMaxSize() * 2;
		while (newMaxSize < size)
			newMaxSize *= 2;

		m_ringBuffers.emplace_back(m_nextRingBufferID++, DynamicGPURingBuffer(newMaxSize));
		allocation = m_ringBuffers.back().ringBuffer.Allocate(size);
	}

	return {m_ringBuffers.back().ID, allocation};
}

void DynamicBufferAllocator::CloseCmdListAllocations(uint64_t fenceValue)
{
    for (auto& ringBuffer : m_ringBuffers)
		ringBuffer.ringBuffer.CloseCmdListAllocations(fenceValue);
}

void DynamicBufferAllocator::ReleaseStaleAllocations(uint64_t lastCompletedFenceValue)
{
    size_t numBuffsToDelete = 0;
    for (size_t i = 0; i < m_ringBuffers.size(); ++i)
    {
        auto& ringBuffer = m_ringBuffers[i].ringBuffer;
        ringBuffer.ReleaseCompletedResources(lastCompletedFenceValue);
        if (numBuffsToDelete == i && i < m_ringBuffers.size() - 1 && ringBuffer.IsEmpty())
            ++numBuffsToDelete;
    }

    if (numBuffsToDelete)
        m_ringBuffers.erase(m_ringBuffers.begin(), m_ringBuffers.begin() + (int32_t)numBuffsToDelete);
}

ID3D12Resource* DynamicBufferAllocator::GetD3D12Resource(int64_t ringBufferID) const
{
	auto it = std::ranges::find_if(m_ringBuffers,
		[ringBufferID](auto& element)
		{
			return element.ID == ringBufferID;
		});

	if (it != m_ringBuffers.end())
		return it->ringBuffer.GetD3D12Resource();

	return nullptr;
}
}
