#pragma once
#include "Prism/Render/RenderCommands.h"
#include "Prism/Utilities/MemoryUtils.h"

namespace Prism::Render
{
class DeferredCommandRecorder
{
	friend class RenderCommandQueue;

public:
	DeferredCommandRecorder();

	template<typename T, typename... Args>
	void AllocateCommand(Args&&... args) requires std::is_base_of_v<Commands::RenderCommandBase, T>
	{
		PE_ASSERT(!m_closed);

		if (m_commandListForBypass)
		{
			PE_ASSERT(m_commandListForBypass);
			T command(std::forward<Args>(args)...);
			command.Execute(m_commandListForBypass);
		}
		else
		{
			size_t alignedOffset = Align(m_newCommandOffset, alignof(T));
			m_newCommandOffset = alignedOffset + sizeof(T);
			m_commands.insert(m_commands.end(), m_newCommandOffset - m_commands.size(), 0);
			PE_ASSERT(alignedOffset <= m_commands.size() && m_commands.size() - alignedOffset >= sizeof(T));
			auto* command = (Commands::RenderCommandBase*)new (m_commands.data() + alignedOffset) T(std::forward<Args>(args)...);
			*m_commandLink = command;
			m_commandLink = &command->next;

			/*if (command->GetCommandString().empty())
				PE_RENDER_LOG(Info, "Recording: {}", T::GetCommandStringStatic());
			else
				PE_RENDER_LOG(Info, "Recording: {}_{}", T::GetCommandStringStatic(), command->GetCommandString());*/
		}
	}

	void Close();

	RenderCommandList* GetCommandListForBypass();

private:
	void RecordCommands(RenderCommandList* commandList);

private:
	std::vector<uint8_t> m_commands = {};
	size_t m_newCommandOffset = 0;
	Commands::RenderCommandBase* m_root = nullptr;
	Commands::RenderCommandBase** m_commandLink = nullptr;

	bool m_closed = false;
	std::atomic<bool> m_processed = false;

	Ref<RenderCommandList> m_commandListForBypass;
};
}
