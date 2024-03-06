#pragma once
#include <any>

#include "Prism-Core/Render/RenderConstants.h"
#include "Prism-Core/Render/RenderThread.h"

namespace Prism::Render
{
class RenderThreadCommandList
{
private:
	using RenderCmdFuncPtr = void(*)(void*);

	struct CommandMetaData
	{
		RenderCmdFuncPtr commandFunc = nullptr;
		uint32_t commandBufferSize = 0;
	};

public:
	RenderThreadCommandList();
	~RenderThreadCommandList();

	template<typename Func>
	void RecordCommand(Func&& function)
	{
		AllocateCommand(&RenderCmd<Func>, sizeof(Func), &function);
	}

private:
	void AllocateCommand(RenderCmdFuncPtr renderCmdFunc, uint32_t cmdBufferSize, void* cmdBufferPtr);

	template<typename Func>
	static void RenderCmd(void* funcPtr)
	{
		auto* func = static_cast<Func*>(funcPtr);
		(*func)();

		func->~Func();
	}

	template<typename T>
	void AppendToCmdBuffer(const T& data);
	void AppendToCmdBuffer(const void* data, uint32_t size);

private:
	std::byte* m_cmdBufferData = nullptr;
	std::byte* m_cmdBufferDataEndPtr = nullptr;
};

class RenderThreadCommandQueue
{
public:
	RenderThreadCommandQueue();

	template<typename Func>
	void RecordCommandToCurrentCmdList(Func&& function)
	{
		//m_commandLists[0].RecordCommand(std::forward<Func>(function));
	}

	void ExecuteCurrentCmdList();

private:
	//std::array<RenderThread, Constants::RENDER_THREADS_COUNT> m_renderThreads;
	//std::array<RenderThreadCommandList, Constants::RENDER_THREADS_COUNT> m_commandLists;
};
}
