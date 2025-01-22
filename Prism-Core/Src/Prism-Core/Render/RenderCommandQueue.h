#pragma once
#include "Prism-Core/Render/ReleaseQueue.h"

namespace Prism::Render
{
struct SubmittedRenderContext
{
	uint64_t fenceValue = 0;
	bool readyToBeReleased = false;
	Ref<class RenderContext> renderContext;
};

/**
 * Will create render worker threads that process and execute command lists
 */
class RenderCommandQueue : public RefCounted
{
private:
	struct CmdListWithFenceValue
	{
		class RenderCommandList* cmdList = nullptr;
		uint64_t fenceValue = 0;
	};

public:
	uint64_t Submit(RenderContext* context);
	uint64_t SubmitDirectly(RenderCommandList* cmdList);

	void Flush();
	virtual void WaitForCmdListToComplete(uint64_t fenceValue) = 0;

	uint64_t GetLastSubmittedCmdListFenceValue() const;
	uint64_t GetLastQueuedCmdListFenceValue() const;
	virtual uint64_t GetLastCompletedCmdListFenceValue() const = 0;

	void ExecuteGPUCompletionEvents();

	virtual void ReleaseStaleResources();

private:
	virtual void Execute(RenderCommandList* cmdList, uint64_t fenceValue) = 0;

	void TryExecuteQueuedCmdListsAsync();

private:
	uint64_t m_lastSubmittedCmdListFenceValue = 0;
	uint64_t m_lastQueuedCmdListFenceValue = 0;

	std::queue<CmdListWithFenceValue> m_cmdListsQueue;
	std::mutex m_cmdListsExecutingMutex;
	bool m_isCurrentlyExecuting = false;

	std::deque<SubmittedRenderContext> m_submittedContexts;
};
}
