#include "pcpch.h"
#include "RenderThreadCommands.h"

namespace Prism::Render
{
RenderThreadCommandList::RenderThreadCommandList()
{
	m_cmdBufferData = new std::byte[Constants::RENDER_THREAD_CMD_LIST_BYTE_SIZE];
	m_cmdBufferDataEndPtr = m_cmdBufferData;
}

RenderThreadCommandList::~RenderThreadCommandList()
{
	delete[] m_cmdBufferData;
}

void RenderThreadCommandList::AllocateCommand(RenderCmdFuncPtr renderCmdFunc, uint32_t cmdBufferSize, void* cmdBufferPtr)
{
	CommandMetaData meta = {
		.commandFunc = renderCmdFunc,
		.commandBufferSize = cmdBufferSize
	};

	AppendToCmdBuffer(meta);
	AppendToCmdBuffer(cmdBufferPtr, cmdBufferSize);
}

template<typename T>
void RenderThreadCommandList::AppendToCmdBuffer(const T& data)
{
	AppendToCmdBuffer(&data, sizeof(T));
}

void RenderThreadCommandList::AppendToCmdBuffer(const void* data, uint32_t size)
{
	// TODO: Check if there is enough space left
	memcpy(m_cmdBufferDataEndPtr, data, size);
	m_cmdBufferDataEndPtr += size;
}

RenderThreadCommandQueue::RenderThreadCommandQueue()
{
	for (RenderThread thread : m_renderThreads)
	{

	}
}

void RenderThreadCommandQueue::ExecuteCurrentCmdList()
{
	
}
}
