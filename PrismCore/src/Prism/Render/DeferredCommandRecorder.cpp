#include "pcpch.h"
#include "Prism/Render/DeferredCommandRecorder.h"

#include "Prism/Render/RenderDevice.h"


namespace Prism::Render
{
DeferredCommandRecorder::DeferredCommandRecorder()
	: m_commandLink(&m_root)
{
	if (RenderDevice::Get().GetBypassCommandRecording())
		m_commandListForBypass = RenderCommandList::Create();
}

void DeferredCommandRecorder::Close()
{
	m_closed = true;
}

RenderCommandList* DeferredCommandRecorder::GetCommandListForBypass()
{
	return m_commandListForBypass;
}

void DeferredCommandRecorder::RecordCommands(RenderCommandList* commandList)
{
	PE_ASSERT(m_root);
	PE_ASSERT(commandList);
	PE_ASSERT(m_closed);

	auto* current = m_root;
	while (current)
	{
		auto* next = current->next;
		current->ExecuteAndDestruct(commandList);
		current = next;
	}
}
}
