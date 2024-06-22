#pragma once

#include <queue>


namespace Prism::Render
{
template<typename T>
class ReleaseQueue
{
public:
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
}
