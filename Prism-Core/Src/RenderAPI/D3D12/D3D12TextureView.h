#pragma once
#include "Prism-Core/Render/TextureView.h"
#include "RenderAPI/D3D12/D3D12DescriptorHeapManager.h"

namespace Prism::Render
{
class Texture;
}

namespace Prism::Render::D3D12
{
class D3D12TextureView : public TextureView
{
public:
	D3D12TextureView(TextureViewDesc desc, Texture* texture);

	void BuildView();

	const CPUDescriptorHeapAllocation& GetDescriptor() const { return m_descriptor; }

private:
	TextureViewDesc m_viewDesc;
	CPUDescriptorHeapAllocation m_descriptor;
};
}
