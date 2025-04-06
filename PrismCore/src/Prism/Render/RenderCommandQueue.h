#pragma once
#include "Prism/Render/ReleaseQueue.h"

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

	// Not thread safe
	virtual uint64_t GetFenceValue() = 0;
	virtual void IncreaseFenceValue() = 0;
	virtual void SignalFence(uint64_t fenceValue) = 0;
	virtual uint64_t IncreaseAndSignalFence() = 0;
	virtual uint64_t GetCompletedFenceValue() const = 0;

	void Flush();
	virtual void WaitForFenceToComplete(uint64_t fenceValue) = 0;

	uint64_t GetLastSubmittedCmdListFenceValue() const;
	uint64_t GetLastQueuedCmdListFenceValue() const;

	void ExecuteGPUCompletionEvents();

	virtual void ReleaseStaleResources();

private:
	virtual void Execute(RenderCommandList* cmdList) = 0;

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
