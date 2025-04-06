#include "pcpch.h"
#include "D3D12RenderCommandQueue.h"

#include "RenderAPI/D3D12/D3D12RenderCommandList.h"
#include "RenderAPI/D3D12/D3D12RenderDevice.h"

namespace Prism::Render::D3D12
{
D3D12RenderCommandQueue::D3D12RenderCommandQueue()
{
	auto* d3d12Device = D3D12RenderDevice::Get().GetD3D12Device();
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	PE_ASSERT_HR(d3d12Device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_d3d12CommandQueue)));
	PE_ASSERT_HR(m_d3d12CommandQueue->SetName(L"Main Cmd Queue"));

	PE_ASSERT_HR(d3d12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_cmdListFence)));
	PE_ASSERT_HR(m_cmdListFence->SetName(L"Cmd List Fence"));
}

uint64_t D3D12RenderCommandQueue::GetFenceValue()
{
	return m_fenceValue;
}

void D3D12RenderCommandQueue::IncreaseFenceValue()
{
	++m_fenceValue;
}

void D3D12RenderCommandQueue::SignalFence(uint64_t fenceValue)
{
	PE_ASSERT_HR(m_d3d12CommandQueue->Signal(m_cmdListFence.Get(), fenceValue));
}

uint64_t D3D12RenderCommandQueue::IncreaseAndSignalFence()
{
	IncreaseFenceValue();
	SignalFence(m_fenceValue);
	return GetFenceValue();
}

uint64_t D3D12RenderCommandQueue::GetCompletedFenceValue() const
{
	return m_cmdListFence->GetCompletedValue();
}

void D3D12RenderCommandQueue::WaitForFenceToComplete(uint64_t fenceValue)
{
	if (m_cmdListFence->GetCompletedValue() < fenceValue)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);

		PE_ASSERT_HR(m_cmdListFence->SetEventOnCompletion(fenceValue, eventHandle));

		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	ExecuteGPUCompletionEvents();
}

ID3D12CommandQueue* D3D12RenderCommandQueue::GetD3D12CommandQueue() const
{
	return m_d3d12CommandQueue.Get();
}

void D3D12RenderCommandQueue::Execute(RenderCommandList* cmdList)
{
	std::array<ID3D12CommandList*, 1> cmdListArray = {
		static_cast<D3D12RenderCommandList*>(cmdList)->GetD3D12CommandList()
	};
	m_d3d12CommandQueue->ExecuteCommandLists((UINT)cmdListArray.size(), cmdListArray.data());
}
}
