#pragma once
#include "Prism/Render/RenderCommandQueue.h"
#include "RenderAPI/D3D12/D3D12RenderCommandList.h"

namespace Prism::Render::D3D12
{
class D3D12RenderCommandQueue : public RenderCommandQueue
{
public:
	D3D12RenderCommandQueue();

	void SetMarker(glm::float3 color, std::wstring string) override;
	void BeginEvent(glm::float3 color, std::wstring string) override;
	void EndEvent() override;

	uint64_t GetFenceValue() override;
	void IncreaseFenceValue() override;
	void SignalFence(uint64_t fenceValue) override;
	virtual uint64_t IncreaseAndSignalFence() override;
	virtual uint64_t GetCompletedFenceValue() const override;

	virtual void WaitForFenceToComplete(uint64_t fenceValue) override;

	ID3D12CommandQueue* GetD3D12CommandQueue() const;

private:
	virtual void Execute(RenderCommandList* cmdList) override;

private:
	ComPtr<ID3D12CommandQueue> m_d3d12CommandQueue;

	uint64_t m_fenceValue = 0;
	ComPtr<ID3D12Fence> m_cmdListFence;
};
}
