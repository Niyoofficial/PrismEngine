﻿#pragma once
#include "Prism-Core/Render/RenderContext.h"
#include "RenderAPI/D3D12/D3D12GraphicsPipelineState.h"


namespace Prism::Render::D3D12
{
class D3D12RenderContext : public RenderContext
{
public:
	D3D12RenderContext();

	ID3D12CommandAllocator* GetCommandAllocator() const { return m_commandAllocator.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() const { return m_commandList.Get(); }

	virtual void Draw(DrawCommandDesc desc) override;
	virtual void DrawIndexed(DrawIndexedCommandDesc desc) override;

	virtual void SetPSO(GraphicsPipelineState* pso) override;
	virtual void SetRenderTargets(std::vector<TextureView*> rtvs, TextureView* dsv) override;
	virtual void SetViewports(std::vector<Viewport> viewports) override;
	virtual void SetScissors(std::vector<Scissor> scissors) override;

	virtual void ClearRenderTargetView(TextureView* rtv, glm::float4* clearColor = nullptr) override;
	virtual void ClearDepthStencilView(TextureView* dsv, Flags<ClearFlags> flags, DepthStencilValue* clearValue = nullptr) override;

	virtual void Transition(StateTransitionDesc desc) override;

private:
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
};
}