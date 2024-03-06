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
	D3D12TextureView(const TextureViewDesc& desc, Texture* texture);

	virtual Texture* GetTexture() const override;

	const DescriptorHeapAllocation& GetDescriptor() const { return m_descriptor; }

private:
	Texture* m_owningTexture = nullptr;
	DescriptorHeapAllocation m_descriptor;
};
}
