#pragma once
#include "Prism/Render/DeferredCommandRecorder.h"
#include "Prism/Render/ReleaseQueue.h"

namespace Prism::Render
{
class Swapchain;

struct SubmittedRenderContext
{
	uint64_t fenceValue = 0;
	bool readyToBeReleased = false;
	Ref<class RenderContext> renderContext;
};

enum class CommandQueueFlushType
{
	// Wait for the commands to be submitted to the GPU
	WaitForSubmission,
	// Wait for the commands to be completed by the GPU
	WaitForCompletion
};

/**
 * Is responsible for recording and submitting rendering commands to the GPU
 */
class RenderCommandQueue
{
public:
	RenderCommandQueue();
	virtual ~RenderCommandQueue() = default;

	uint64_t Submit(const Ref<RenderContext>& context);
	void EnqueuePresent(const Ref<Swapchain>& swapchain);

	virtual void SetMarker(glm::float3 color, std::wstring string) = 0;
	virtual void BeginEvent(glm::float3 color, std::wstring string) = 0;
	virtual void EndEvent() = 0;

	// Not thread safe
	virtual uint64_t GetFenceValue() = 0;
	virtual void IncreaseFenceValue() = 0;
	// TODO: Rework this, currently signaling fence without flushing the recording queue
	// will break the commands submission
	virtual void SignalFence(uint64_t fenceValue) = 0;
	virtual uint64_t IncreaseAndSignalFence() = 0;
	virtual uint64_t GetCompletedFenceValue() const = 0;

	void Flush(CommandQueueFlushType flushType);
	virtual void WaitForFenceToComplete(uint64_t fenceValue) = 0;

	uint64_t GetLastSubmittedCmdListFenceValue() const;
	uint64_t GetLastQueuedCmdListFenceValue() const;

	void ExecuteGPUCompletionEvents();

	virtual void ReleaseStaleResources();

private:
	virtual void Execute(class RenderCommandList* cmdList) = 0;

	void TryExecuteQueuedCmdListsAsync();

private:
	std::jthread m_commandRecordingThread;

	uint64_t m_lastSubmittedCmdListFenceValue = 0;
	uint64_t m_lastQueuedCmdListFenceValue = 0;

	struct ContextRecordingInfo
	{
		RenderContext* renderContext = nullptr;
		RenderCommandList* cmdList = nullptr;
		uint64_t fenceValue = 0;
	};
	struct PresentRecordingInfo
	{
		Swapchain* swapchain = nullptr;
	};
	using RecordingInfo = std::variant<ContextRecordingInfo, PresentRecordingInfo>;
	std::queue<RecordingInfo> m_recordingQueue;
	std::mutex m_recordingMutex;
	std::condition_variable m_recordingCV;

	std::deque<SubmittedRenderContext> m_submittedContexts;
};
}
