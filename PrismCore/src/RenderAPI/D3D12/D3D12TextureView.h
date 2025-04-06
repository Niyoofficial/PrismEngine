#pragma once
#include "Prism/Render/TextureView.h"
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

	void BuildDynamicDescriptor(CD3DX12_CPU_DESCRIPTOR_HANDLE d3d12DescriptorHandle);

	const DescriptorHeapAllocation& GetDescriptor() const { return m_descriptor; }

	D3D12_DESCRIPTOR_HEAP_TYPE GetDescriptorHeapType() const;

private:
	TextureViewDesc m_viewDesc;
	DescriptorHeapAllocation m_descriptor;
};
}
