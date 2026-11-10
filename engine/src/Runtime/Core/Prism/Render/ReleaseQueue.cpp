#include "pcpch.h"
#include "ReleaseQueue.h"

namespace Prism::Render
{
ReleaseQueueAny::ReleaseQueueAny(ReleaseQueueAny&& other) noexcept
	: m_releaseQueue(std::move(other.m_releaseQueue))
{
	other.m_releaseQueue = {};
}

ReleaseQueueAny& ReleaseQueueAny::operator=(ReleaseQueueAny&& other) noexcept
{
	m_releaseQueue = std::move(other.m_releaseQueue);
	other.m_releaseQueue = {};

	return *this;
}
}
