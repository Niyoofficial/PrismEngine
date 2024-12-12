#pragma once
#include "Prism-Core/Render/RenderCommandQueue.h"
#include "RenderAPI/D3D12/D3D12RenderCommandList.h"

namespace Prism::Render::D3D12
{
class D3D12RenderCommandQueue : public RenderCommandQueue
{
public:
	D3D12RenderCommandQueue();

	virtual void WaitForCmdListToComplete(uint64_t fenceValue) override;

	virtual uint64_t GetLastCompletedCmdListFenceValue() const override;

	ID3D12CommandQueue* GetD3D12CommandQueue() const;

private:
	virtual void Execute(RenderCommandList* cmdList) override;

private:
	ComPtr<ID3D12CommandQueue> m_d3d12CommandQueue;

	uint64_t m_lastExecutedCmdListFenceValue = 0;
	ComPtr<ID3D12Fence> m_cmdListFence;
};
}
