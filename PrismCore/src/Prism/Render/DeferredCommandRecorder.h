#pragma once
#include "Prism/Render/RenderCommands.h"
#include "Prism/Render/RenderConstants.h"
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
			//PE_RENDER_LOG(Info, "Recording: {}", T::GetCommandString());

			size_t alignedOffset = Align(m_newCommandOffset, alignof(T));
			m_newCommandOffset = alignedOffset + sizeof(T);
			PE_ASSERT(alignedOffset <= m_commands.size() && m_commands.size() - alignedOffset >= sizeof(T));
			auto* command = (Commands::RenderCommandBase*)new (m_commands.data() + alignedOffset) T(std::forward<Args>(args)...);
			*m_commandLink = command;
			m_commandLink = &command->next;
		}
	}

	void Close();

	RenderCommandList* GetCommandListForBypass();

private:
	void RecordCommands(RenderCommandList* commandList);

private:
	// TODO: Replace with a chunk allocator
	std::array<uint8_t, Constants::CMD_LIST_BYTE_SIZE> m_commands = {};
	size_t m_newCommandOffset = 0;
	Commands::RenderCommandBase* m_root = nullptr;
	Commands::RenderCommandBase** m_commandLink = nullptr;

	bool m_closed = false;
	std::atomic<bool> m_processed = false;

	Ref<RenderCommandList> m_commandListForBypass;
};
}
