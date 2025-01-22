#include "pcpch.h"
#include "RenderCommandQueue.h"

#include <future>

#include "Prism-Core/Render/RenderContext.h"
#include "Prism-Core/Render/RenderResourceCreation.h"


namespace Prism::Render
{
uint64_t RenderCommandQueue::Submit(RenderContext* context)
{
	PE_ASSERT(context);

	context->CloseContext();

	++m_lastSubmittedCmdListFenceValue;

	auto cmdList = RenderCommandList::Create();
	Ref<RenderCommandList> cmdListCopy = cmdList;
	context->SafeReleaseResource(cmdListCopy);

	m_submittedContexts.emplace_back(SubmittedRenderContext{
		.fenceValue = m_lastSubmittedCmdListFenceValue,
		.readyToBeReleased = false,
		.renderContext = context
	});

	m_cmdListsQueue.push({cmdList, m_lastSubmittedCmdListFenceValue});

	auto func = [this, cmdRecorder = &context->m_commandRecorder, cmdList = cmdList.Raw()]()
		{
			cmdRecorder->RecordCommands(cmdList);
			cmdList->Close();

			TryExecuteQueuedCmdListsAsync();
		};

	auto future = std::async(std::launch::async, func);

	return GetLastSubmittedCmdListFenceValue();
}

uint64_t RenderCommandQueue::SubmitDirectly(RenderCommandList* cmdList)
{
	cmdList->Close();

	++m_lastSubmittedCmdListFenceValue;

	Ref<RenderCommandList> cmdListRef = cmdList;
	RenderDevice::Get().AddResourceToReleaseQueue(cmdListRef, GetLastSubmittedCmdListFenceValue());

	m_cmdListsQueue.push({ cmdList, m_lastSubmittedCmdListFenceValue });
	auto future = std::async(std::launch::async, [this]() { TryExecuteQueuedCmdListsAsync(); });

	return GetLastSubmittedCmdListFenceValue();
}

void RenderCommandQueue::Flush()
{
	WaitForCmdListToComplete(GetLastSubmittedCmdListFenceValue());
	ExecuteGPUCompletionEvents();
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
	uint64_t completedFenceValue = GetLastCompletedCmdListFenceValue();
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
	{
		std::lock_guard lock(m_cmdListsExecutingMutex);

		if (m_isCurrentlyExecuting)
			return;

		m_isCurrentlyExecuting = true;
	}

	while (!m_cmdListsQueue.empty() && m_cmdListsQueue.front().cmdList->IsClosed())
	{
		auto cmdListToExecute = m_cmdListsQueue.front();

		// Check that makes sure that cmd lists are executed in order and that none are missing
		PE_ASSERT(m_lastQueuedCmdListFenceValue + 1 == cmdListToExecute.fenceValue);

		m_cmdListsQueue.pop();
		Execute(cmdListToExecute.cmdList, cmdListToExecute.fenceValue);
		++m_lastQueuedCmdListFenceValue;
	}

	{
		std::lock_guard lock(m_cmdListsExecutingMutex);

		m_isCurrentlyExecuting = false;
	}
}
}
