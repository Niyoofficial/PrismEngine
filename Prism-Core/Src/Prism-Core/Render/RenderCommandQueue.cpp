﻿#include "pcpch.h"
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
	auto cmdList = RenderCommandList::Create(m_lastSubmittedCmdListFenceValue);
	context->m_preservedResources.MoveObjects(cmdList->m_preservedResources);
	Ref<RenderCommandList> cmdListCopy = cmdList;
	context->SafeReleaseResource(cmdListCopy);

	m_contextReleaseQueue.AddResource(context, m_lastSubmittedCmdListFenceValue);
	
	m_cmdListsQueue.push(cmdList);

	auto func = [this, cmdRecorder = &context->m_commandRecorder, cmdList = cmdList.Raw()]()
		{
			cmdRecorder->RecordCommands(cmdList);
			cmdList->Close();

			{
				std::lock_guard lock(m_cmdListsExecutingMutex);

				if (m_isCurrentlyExecuting)
					return;

				m_isCurrentlyExecuting = true;
			}

			while (!m_cmdListsQueue.empty() && m_cmdListsQueue.front()->IsClosed())
			{
				auto* cmdListToExecute = m_cmdListsQueue.front();

				// Check that makes sure that cmd lists are executed in order and that none are missing
				PE_ASSERT(m_lastQueuedCmdListFenceValue + 1 == cmdListToExecute->GetFenceValue());

				m_cmdListsQueue.pop();
				Execute(cmdListToExecute);
				++m_lastQueuedCmdListFenceValue;
			}

			{
				std::lock_guard lock(m_cmdListsExecutingMutex);

				m_isCurrentlyExecuting = false;
			}
		};

	auto future = std::async(std::launch::async, func);

	return GetLastSubmittedCmdListFenceValue();
}

void RenderCommandQueue::Flush()
{
	WaitForCmdListToComplete(GetLastSubmittedCmdListFenceValue());
}

uint64_t RenderCommandQueue::GetLastSubmittedCmdListFenceValue() const
{
	return m_lastSubmittedCmdListFenceValue;
}

uint64_t RenderCommandQueue::GetLastQueuedCmdListFenceValue() const
{
	return m_lastQueuedCmdListFenceValue;
}

void RenderCommandQueue::ReleaseStaleResources()
{
	m_contextReleaseQueue.PurgeReleaseQueue(GetLastCompletedCmdListFenceValue());
}
}
