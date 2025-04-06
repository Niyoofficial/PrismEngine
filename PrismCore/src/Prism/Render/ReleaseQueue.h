#pragma once

#include <queue>

#include "Prism/Utilities/PreservingObjectContainer.h"


namespace Prism::Render
{
template<typename T>
class ReleaseQueue
{
public:
	ReleaseQueue() = default;
	ReleaseQueue(const ReleaseQueue&) = delete;
	ReleaseQueue(ReleaseQueue&& other) noexcept
		: m_releaseQueue(std::move(other))
	{
		other.m_releaseQueue = {};
	}

	ReleaseQueue& operator=(const ReleaseQueue&) = delete;
	ReleaseQueue& operator=(ReleaseQueue&& other) noexcept
	{
		m_releaseQueue = std::move(other.m_releaseQueue);
		other.m_releaseQueue = {};

		return *this;
	}

	~ReleaseQueue() = default;

	// Objects MUST be std::move'd into this function
	void AddResource(T&& resource, uint64_t fenceValue) requires !std::is_lvalue_reference_v<T>
	{
		m_releaseQueue.emplace(fenceValue, std::move(resource));
	}

	void PurgeReleaseQueue(uint64_t completedFenceValue)
	{
		while (!m_releaseQueue.empty())
		{
			if (m_releaseQueue.front().first <= completedFenceValue)
				m_releaseQueue.pop();
			else
				break;
		}
	}

private:
	std::queue<std::pair<uint64_t, T>> m_releaseQueue;
};

class ReleaseQueueAny
{
public:
	ReleaseQueueAny() = default;
	ReleaseQueueAny(const ReleaseQueueAny&) = delete;
	ReleaseQueueAny(ReleaseQueueAny&& other) noexcept;

	ReleaseQueueAny& operator=(const ReleaseQueueAny&) = delete;
	ReleaseQueueAny& operator=(ReleaseQueueAny&& other) noexcept;

	~ReleaseQueueAny() = default;

	// Objects MUST be std::move'd into this function
	template<typename T>
	void AddResource(T&& resource, uint64_t fenceValue) requires !std::is_lvalue_reference_v<T>
	{
		m_releaseQueue.emplace(fenceValue, new PreservedResourceWrapper{std::move(resource)});
	}

	void PurgeReleaseQueue(uint64_t completedFenceValue)
	{
		while (!m_releaseQueue.empty())
		{
			if (m_releaseQueue.front().first <= completedFenceValue)
				m_releaseQueue.pop();
			else
				break;
		}
	}

private:
	std::queue<std::pair<uint64_t, Ref<PreservedResourceWrapperBase>>> m_releaseQueue;
};
}
