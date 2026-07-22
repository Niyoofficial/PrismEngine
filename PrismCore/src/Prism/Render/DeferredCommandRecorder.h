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
	~DeferredCommandRecorder();

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
			if (alignedOffset + sizeof(T) >= CMD_PAGE_SIZE)
			{
				m_commandPages.push_back(new uint8_t[CMD_PAGE_SIZE]);
				m_pageIndex++;
				alignedOffset = 0;
			}
			m_newCommandOffset = alignedOffset + sizeof(T);
			auto* command = (Commands::RenderCommandBase*)new (m_commandPages[m_pageIndex] + alignedOffset) T(std::forward<Args>(args)...);
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
	constexpr static size_t CMD_PAGE_SIZE = 1024ull * 100;
	std::vector<uint8_t*> m_commandPages;
	size_t m_newCommandOffset = 0;
	int32_t m_pageIndex = 0;
	Commands::RenderCommandBase* m_root = nullptr;
	Commands::RenderCommandBase** m_commandLink = nullptr;

	bool m_closed = false;
	std::atomic<bool> m_processed = false;

	Ref<RenderCommandList> m_commandListForBypass;
};
}
