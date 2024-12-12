#pragma once
#include "Prism-Core/Render/ReleaseQueue.h"

namespace Prism::Render
{
/**
 * Will create render worker threads that process and execute command lists
 */
class RenderCommandQueue : public RefCounted
{
public:
	uint64_t Submit(class RenderContext* context);

	void Flush();
	virtual void WaitForCmdListToComplete(uint64_t fenceValue) = 0;

	virtual uint64_t GetLastSubmittedCmdListFenceValue() const;
	virtual uint64_t GetLastQueuedCmdListFenceValue() const;
	virtual uint64_t GetLastCompletedCmdListFenceValue() const = 0;

	virtual void ReleaseStaleResources();

private:
	virtual void Execute(class RenderCommandList* cmdList) = 0;

private:
	uint64_t m_lastSubmittedCmdListFenceValue = 0;
	uint64_t m_lastQueuedCmdListFenceValue = 0;

	std::queue<RenderCommandList*> m_cmdListsQueue;
	std::mutex m_cmdListsExecutingMutex;
	bool m_isCurrentlyExecuting = false;

	ReleaseQueue<Ref<RenderContext>> m_contextReleaseQueue;
};
}
