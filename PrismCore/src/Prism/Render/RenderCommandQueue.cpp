#include "pcpch.h"
#include "RenderCommandQueue.h"

#include <future>

#include "Prism/Render/RenderContext.h"
#include "Prism/Render/RenderResourceCreation.h"


namespace Prism::Render
{
template<class... Ts>
struct Overloaded : Ts... { using Ts::operator()...; };

RenderCommandQueue::RenderCommandQueue()
	: m_commandRecordingThread(
		[commandQueue = this](std::stop_token st)
		{
			//auto test = commandQueue;
			while (!st.stop_requested())
			{
				RecordingInfo recordingInfo;
				{
					std::unique_lock lock(commandQueue->m_recordingMutex);

					if (commandQueue->m_recordingQueue.empty())
					{
						commandQueue->m_recordingCV.notify_one();
						continue;
					}

					recordingInfo = commandQueue->m_recordingQueue.front();
					commandQueue->m_recordingQueue.pop();
				}

				std::visit(Overloaded{
					[&commandQueue](const ContextRecordingInfo& context)
					{
						if (context.renderContext)
						{
							context.renderContext->m_commandRecorder.RecordCommands(context.cmdList);
							context.cmdList->Close();

							//PE_RENDER_LOG(Info, "Executing \"{}\"", WStringToString(context.renderContext->m_debugName));
							commandQueue->Execute(context.cmdList);

							commandQueue->SignalFence(context.fenceValue);
						}
					},
					[](const PresentRecordingInfo& present)
					{
						//PE_RENDER_LOG(Info, "Presenting \"{}\"", present.swapchain->GetCurrentBackBufferIndex());
						present.swapchain->Present();
					}}, recordingInfo);
			}
		})
{
}

uint64_t RenderCommandQueue::Submit(const Ref<RenderContext>& context)
{
	PE_ASSERT(context);

	context->CloseContext();

	IncreaseFenceValue();
	m_lastSubmittedCmdListFenceValue = GetFenceValue();

	Ref<RenderCommandList> cmdList;
	if (RenderDevice::Get().GetBypassCommandRecording())
	{
		cmdList = context->m_commandRecorder.GetCommandListForBypass();
	}
	else
	{
		cmdList = RenderCommandList::Create();
		context->SafeReleaseResource(cmdList);
	}

	m_submittedContexts.emplace_back(SubmittedRenderContext{
		.fenceValue = m_lastSubmittedCmdListFenceValue,
		.readyToBeReleased = false,
		.renderContext = context
	});

	{
		std::unique_lock lock(m_recordingMutex);
		m_recordingQueue.emplace(ContextRecordingInfo{context, cmdList, m_lastSubmittedCmdListFenceValue});
	}

	return m_lastSubmittedCmdListFenceValue;
}

void RenderCommandQueue::EnqueuePresent(const Ref<Swapchain>& swapchain)
{
	RenderDevice::Get().AddResourceToReleaseQueueWhenFrameEnds(swapchain);

	swapchain->AdvanceBackBufferIndex();

	{
		std::unique_lock lock(m_recordingMutex);
		m_recordingQueue.emplace(PresentRecordingInfo{swapchain});
	}
}

void RenderCommandQueue::Flush(CommandQueueFlushType flushType)
{
	// This part happens no matter what flush type is chosen
	{
		std::unique_lock lock(m_recordingMutex);

		m_recordingQueue.push({});

		m_recordingCV.wait(lock, [this]() { return m_recordingQueue.empty(); });
	}

	if (flushType == CommandQueueFlushType::WaitForCompletion)
	{
		uint64_t fenceValue = IncreaseAndSignalFence();
		SetMarker({ 1.f, 0.f, 0.f }, L"Flush");
		WaitForFenceToComplete(fenceValue);
		ReleaseStaleResources();
	}
}

uint64_t RenderCommandQueue::GetLastSubmittedCmdListFenceValue() const
{
	return m_lastSubmittedCmdListFenceValue;
}

uint64_t RenderCommandQueue::GetLastQueuedCmdListFenceValue() const
{
	return m_lastQueuedCmdListFenceValue;
}

void RenderCommandQueue::ExecuteGPUCompletionEvents()
{
	uint64_t completedFenceValue = GetCompletedFenceValue();
	for (auto& submittedContext : m_submittedContexts)
	{
		if (submittedContext.fenceValue <= completedFenceValue)
		{
			if (!submittedContext.readyToBeReleased)
			{
				submittedContext.readyToBeReleased = true;
				submittedContext.renderContext->ExecuteGPUCompletionCallbacks();
			}
		}
		else
		{
			break;
		}
	}
}

void RenderCommandQueue::ReleaseStaleResources()
{
	// Special treatment for RenderContexts
	// This could also be done by transferring the context to RenderDevice release queue
	// but since we know they can already be cleared we just do that
	while (!m_submittedContexts.empty())
	{
		if (m_submittedContexts.front().readyToBeReleased)
			m_submittedContexts.pop_front();
		else
			break;
	}
}

void RenderCommandQueue::TryExecuteQueuedCmdListsAsync()
{
	/*{
		std::lock_guard lock(m_cmdListsExecutingMutex);

		if (m_isCurrentlyExecuting)
			return;

		m_isCurrentlyExecuting = true;
	}

	while (!m_recordingQueue.empty() && m_recordingQueue.front()->IsClosed())
	{
		auto cmdListToExecute = m_recordingQueue.front();

		m_recordingQueue.pop();
		Execute(cmdListToExecute);
		m_lastQueuedCmdListFenceValue = IncreaseAndSignalFence();
	}

	{
		std::lock_guard lock(m_cmdListsExecutingMutex);

		m_isCurrentlyExecuting = false;
	}*/
}
}
